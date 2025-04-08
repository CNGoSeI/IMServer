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
	constexpr int VarifyExpired{ Error_Json+2 };//验证码未找到
	constexpr int UserExist{ Error_Json+3 };//用户已经存在
	constexpr int VarifyCodeErr{ Error_Json + 4 };//验证码不正确
	constexpr int EmailNotMatch{ Error_Json + 5 };//邮箱对不上
	constexpr int PasswdUpFailed{ Error_Json + 6 };//重制密码失败
	constexpr int PasswdInvalid{ Error_Json + 7 };//用户或密码错误
	constexpr int RPCGetFailed{ Error_Json + 8 };//获取Rpc失败
}

namespace URI
{
	using namespace std;

	const string Get_Test{"/get_test"};//连接测试
	const string Get_Varifycode{"/get_varifycode"};//投递获取验证码
	const string User_Register{"/user_register"};//投递注册
	const string Rest_Passwd{ "/reset_pwd" };//重置密码
	const string User_Login{ "/user_login" };//登录
}

//修饰词
namespace Prefix
{
	using namespace std;
	const string CODEPREFIX{"code_"};
}

#endif
