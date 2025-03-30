#ifndef HTTPCONNECTION_H
#define HTTPCONNECTION_H

#include <chrono>
#include <memory>
#include "const.h"

class CHttpConnection : public std::enable_shared_from_this<CHttpConnection>
{
	friend class SLogicSystem;

public:
	CHttpConnection(boost::asio::io_context& ioc);
	void Start();

	/* 将输入的十进制转为十六进制 char字符 */
	static unsigned char ToHex(unsigned char Input);
	/* 将输入的十六进制转为十进制 char字符 */
	static unsigned char FromHex(unsigned char Hex);
	/* URL 转码 */
	static std::string UrlEncode(const std::string& str);
	/* URL 解码 */
	std::string UrlDecode(const std::string& str);

	tcp::socket& GetSocket() { return Socket; }
private:
	/**
	 * 检测超时的函数
	 * http处理请求需要有一个时间约束，发送的数据包不能超时。所以在发送时我们启动一个定时器，收到发送的回调后取消定时器
	 */
	void CheckDeadline();
	void WriteResponse();
	void HandleReq();//分发不同的请求类型

	void PreParseGetParam();//解析get请求参数

	tcp::socket Socket;
	// 读取数据 buffer.
	beast::flat_buffer Buffer{8192};

	// 接受Http请求类，采用dynamic_body，能更灵活的接受多种请求(文本、二进制..)
	http::request<http::dynamic_body> Request;

	// 回应客户端的内容
	http::response<http::dynamic_body> Response;

	/**
	 *	http为短链接，请求完成则关闭
	 *	如果发送给客户端的时间超过期望值，则关闭连接
	 *	定时器判断请求是否超时.
	 *	关于参数1
	 *	  — 异步对象（包括定时器）都需要绑定到一个 I/O 上下文，这个上下文负责事件的调度和回调的执行。
	 *	  — 使用 I/O 上下文可以确保所有异步操作在同一个事件循环中有序执行
	 */
	net::steady_timer Deadline{Socket.get_executor(), std::chrono::seconds(60)};

	std::string GetUrl;
	std::unordered_map<std::string, std::string> GetParams;//解析到的Get方法的键值对
};

#endif
