#include "HttpConnection.h"
#include "LogicSystem.h"
#include <iostream>

CHttpConnection::CHttpConnection(tcp::socket socket):
	Socket(std::move(socket))
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

	if (Request.method() == http::verb::get)
	{
		bool success = SLogicSystem::GetInstance().HandleGet(Request.target(), shared_from_this());
		if (!success)
		{
			Response.result(http::status::not_found);
			Response.set(http::field::content_type, "text/plain");
			beast::ostream(Response.body()) << "url not found\r\n";
			WriteResponse();
			return;
		}

		Response.result(http::status::ok);
		Response.set(http::field::server, "GateServer");
		WriteResponse();
		return;
	}
}

void CHttpConnection::WriteResponse()
{
	auto self = shared_from_this();
	Response.content_length(Response.body().size());//设置包体大小，让HTTP切包
	http::async_write(Socket, Response, [self](beast::error_code ec, std::size_t)
	{
		//http是短链接，所以发送完数据后不需要再监听对方链接，直接断开发送端
		self->Socket.shutdown(tcp::socket::shutdown_send, ec);//close是全关，shundown是关一端
		self->Deadline.cancel();//已经手动关闭套接字了，因此不需要定时器监测超时，关闭定时器
	});
}

void CHttpConnection::CheckDeadline()
{
	auto self = shared_from_this();

	Deadline.async_wait([self](beast::error_code ec)
	{
		self->Socket.close(ec);
	});
}
