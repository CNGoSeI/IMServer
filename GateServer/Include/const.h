#ifndef CONST_H
#define CONST_H

#include <boost/beast/http.hpp>
#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include <string>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

namespace ErrorCodes
{
	constexpr int Success{ 0 };
	constexpr int Error_Json{ 1001};//Json解析错误
	constexpr int RPCFailed{ Error_Json+1 };//RPC请求错误
	constexpr int VarifyExpired{ Error_Json+2 };//验证码无效
	constexpr int UserExist{ Error_Json+3 };//用户已经存在
}

namespace URI
{
	using namespace std;

	const string Get_Test{"/get_test"};//连接测试
	const string Get_Varifycode{"/get_varifycode"};//投递获取验证码
	const string User_Register{"/user_register"};//投递注册
}

#endif
