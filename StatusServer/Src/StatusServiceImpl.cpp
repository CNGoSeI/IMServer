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
	Servers.push_back(server);

	server.port = Config.get<std::string>("ChatServer2.Port");
	server.host = Config.get<std::string>("ChatServer2.Host");
	Servers.push_back(server);
}

Status StatusServiceImpl::GetChatServer(ServerContext* context, const GetChatServerReq* request,GetChatServerRsp* reply)
{
	static std::string prefix("IM 状态服务器收到获取链接请求,UID: ");
	std::cout << prefix << request->uid();
	std::lock_guard<std::mutex> gurd(ServerMtx);
	/*
	 * 在两个服务器间连接处理,负载均衡解压
	 * 而中转由 StatusServer 处理
	 */
	ServerIndex = (ServerIndex++) % (Servers.size());

	auto& server = Servers[ServerIndex];

	reply->set_host(server.host);
	reply->set_port(server.port);
	reply->set_error(ErrorCodes::Success);
	reply->set_token(GenerateUniqueString());

	return Status::OK;
}
