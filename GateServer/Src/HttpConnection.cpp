#include "HttpConnection.h"
#include "LogicSystem.h"
#include <iostream>
#include "PubFuncLib.h"
CHttpConnection::CHttpConnection(boost::asio::io_context& ioc):
	Socket(ioc)
{
}

void CHttpConnection::Start()
{
	auto self = shared_from_this();

	/* 
	 * async_read为Boost库的异步读取数据函数，非阻塞方式从AsyncReadStream中数据，并在操作完成后触发 handler
	 * async_read(AsyncReadStream& stream,DynamicBuffer& buffer,basic_parser<isRequest>& parser, ReadHandler&& handler)
	 * AsyncReadStream 可以理解为socket
	 * parser 是请求参数
	 * ReadHandler是回调函数
	 */
	http::async_read(Socket, Buffer, Request, [self](beast::error_code ec, std::size_t bytes_transferred)
	                 {
		                 try
		                 {
			                 if (ec)
			                 {
				                 std::cout << "http read err is " << ec.what() << std::endl;
				                 return;
			                 }

			                 //处理读到的数据
			                 boost::ignore_unused(bytes_transferred);//bytes_transferred 表示本次读取到的大小，但是这用不到，用这个忽略编译警告
			                 self->HandleReq();
			                 self->CheckDeadline();
		                 }
		                 catch (std::exception& exp)
		                 {
			                 std::cout << "exception is " << exp.what() << std::endl;
		                 }
	                 }
	);
}

void CHttpConnection::HandleReq()
{
	//设置版本
	Response.version(Request.version());
	//设置为短链接
	Response.keep_alive(false);

	//没有找到对应方法
	auto UrlNotFoundFunc = [self=shared_from_this()]()
	{
		self->Response.result(http::status::not_found);
		self->Response.set(http::field::content_type, "text/plain");
		beast::ostream(self->Response.body()) << "请求连接不存在\r\n";
		self->WriteResponse();
	};

	//相应请求已经执行完成，回应客户端执行结果
	auto ResponeOkFunc = [self = shared_from_this()]()
	{
		self->Response.result(http::status::ok);
		self->Response.set(http::field::server, "GateServer");
		self->WriteResponse();
	};

	if (Request.method() == http::verb::get)
	{
		PreParseGetParam();
		bool success = SLogicSystem::GetInstance().HandleGet(GetUrl, shared_from_this());
		if (!success)
		{
			UrlNotFoundFunc();
			return;
		}

		ResponeOkFunc();
		return;
	}

	if (Request.method() == http::verb::post)
	{
		bool success = SLogicSystem::GetInstance().HandlePost(Request.target(), shared_from_this());
		if (!success)
		{
			UrlNotFoundFunc();
			return;
		}

		ResponeOkFunc();
		return;
	}
}

void CHttpConnection::PreParseGetParam()
{
	// 提取 URI  
	auto uri = Request.target();

	// 查找查询字符串的开始位置（即 '?' 的位置）  
	auto query_pos = uri.find('?');
	if (query_pos == std::string::npos) {
		GetUrl = uri;
		return;
	}

	GetUrl = uri.substr(0, query_pos);
	std::string query_string = uri.substr(query_pos + 1);
	std::string key;
	std::string value;
	size_t pos = 0;

	//分割 & = 键值对
	while ((pos = query_string.find('&')) != std::string::npos) {
		auto pair = query_string.substr(0, pos);
		size_t eq_pos = pair.find('=');
		if (eq_pos != std::string::npos) {
			key = PubFunc::UrlDecode(pair.substr(0, eq_pos)); // 假设有 url_decode 函数来处理URL解码  
			value = PubFunc::UrlDecode(pair.substr(eq_pos + 1));
			GetParams.emplace(key,value);
		}
		query_string.erase(0, pos + 1);
	}

	// 处理最后一个参数对（如果没有 & 分隔符）  
	if (!query_string.empty()) {
		size_t eq_pos = query_string.find('=');
		if (eq_pos != std::string::npos) {
			key = PubFunc::UrlDecode(query_string.substr(0, eq_pos));
			value = PubFunc::UrlDecode(query_string.substr(eq_pos + 1));
			GetParams.emplace(key, value);
		}
	}
}

void CHttpConnection::WriteResponse()
{
	Response.content_length(Response.body().size());//设置包体大小，让HTTP切包
	http::async_write(Socket, Response, [self = shared_from_this()](beast::error_code ec, std::size_t)
	{
		//http是短链接，所以发送完数据后不需要再监听对方链接，直接断开发送端
		self->Socket.shutdown(tcp::socket::shutdown_send, ec);//close是全关，shundown是关一端
		self->Deadline.cancel();//已经手动关闭套接字了，因此不需要定时器监测超时，关闭定时器
	});
}

void CHttpConnection::CheckDeadline()
{

	Deadline.async_wait([self = shared_from_this()](beast::error_code ec)
	{
		self->Socket.close(ec);
	});
}
