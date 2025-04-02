#ifndef CONST_H
#define CONST_H

#include <boost/beast/http.hpp>
#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include <boost/property_tree/ini_parser.hpp>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

namespace ErrorCodes
{
	constexpr int Success{ 0 };
	constexpr int Error_Json{ 1001};//Json解析错误
	constexpr int RPCFailed{ 1002 };//RPC请求错误
}

#endif
