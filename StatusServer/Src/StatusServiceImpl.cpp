#include "StatusServiceImpl.h"
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include "ConfigMgr.h"
#include "const.h"

std::string GenerateUniqueString() {
	// 创建UUID对象
	boost::uuids::uuid uuid = boost::uuids::random_generator()();
	// 将UUID转换为字符串
	std::string unique_string = to_string(uuid);

	return unique_string;
}

StatusServiceImpl::StatusServiceImpl():ServerIndex(0)
{
	auto& Config = Mgr::GetConfigHelper();

	const auto Addr = Config.get<std::string>("StatusServer.Host") + ":" + Config.get<std::string>("StatusServer.Port");

	ChatServer server;
	server.port = Config.get<std::string>("ChatServer1.Port");
	server.host = Config.get<std::string>("ChatServer1.Host");
	Servers.emplace(server.name,server);

	server.port = Config.get<std::string>("ChatServer2.Port");
	server.host = Config.get<std::string>("ChatServer2.Host");
	Servers.emplace(server.name, server);
}

Status StatusServiceImpl::GetChatServer(ServerContext* context, const GetChatServerReq* request,GetChatServerRsp* reply)
{

	auto server = SelectChatServer();

	reply->set_host(server.host);
	reply->set_port(server.port);
	reply->set_error(ErrorCodes::Success);
	reply->set_token(GenerateUniqueString());
	
	std::cout << "Status 收到连接请求，UID: " << request->uid() << " Token: " << reply->token() << "\n";

	std::lock_guard<std::mutex> guard(TokenMtx);
	Tokens[request->uid()] = reply->token();
	return Status::OK;
}

ChatServer StatusServiceImpl::SelectChatServer()
{
	std::lock_guard<std::mutex> guard(ServerMtx);

	/*
	 * 在两个服务器间连接处理,负载均衡解压
	 * 而中转由 StatusServer 处理
	 */
	auto minServer = Servers.begin()->second;

	//找到连接数最小的服务，并且返回
	for (const auto& server : Servers) {
		if (server.second.ConCount < minServer.ConCount) {
			minServer = server.second;
		}
	}

	return minServer;
}

Status StatusServiceImpl::Login(ServerContext* context, const LoginReq* request, LoginRsp* reply)
{
	auto uid = request->uid();
	auto token = request->token();

	std::lock_guard<std::mutex> guard(TokenMtx);
	auto iter = Tokens.find(uid);
	if (iter == Tokens.end())
	{
		reply->set_error(ErrorCodes::UidInvalid);
		return Status::OK;
	}
	if (iter->second != token)
	{
		reply->set_error(ErrorCodes::TokenInvalid);
		return Status::OK;
	}
	reply->set_error(ErrorCodes::Success);
	reply->set_uid(uid);
	reply->set_token(token);

	return Status::OK;
}
