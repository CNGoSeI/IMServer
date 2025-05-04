#include "StatusServiceImpl.h"
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include "ConfigMgr.h"
#include "const.h"
#include "RedisMgr.h"

std::string GenerateUniqueString() {
	// 创建UUID对象
	boost::uuids::uuid uuid = boost::uuids::random_generator()();
	// 将UUID转换为字符串
	std::string unique_string = to_string(uuid);

	return unique_string;
}

StatusServiceImpl::StatusServiceImpl(): ServerIndex(0)
{
	auto& Config = Mgr::GetConfigHelper();

	const auto Addr = Config.get<std::string>("StatusServer.Host") + ":" + Config.get<std::string>("StatusServer.Port");

	auto ServerList = Config.get<std::string>("ChatServers.Name");
	std::vector<std::string> words;
	std::stringstream ss(ServerList);
	std::string word;

	while (std::getline(ss, word, ','))
	{
		words.push_back(word);
	}

	for (auto& word : words)
	{
		auto TempName = Config.get<std::string>(word + "." + "Name");
		if (TempName.empty())
		{
			continue;
		}

		ChatServer server;
		server.port = Config.get<std::string>(TempName + "." + "Port");
		server.host = Config.get<std::string>(TempName + "." + "Host");
		server.name = Config.get<std::string>(TempName + "." + "Name");
		Servers.emplace(server.name, server);
	}
}

Status StatusServiceImpl::GetChatServer(ServerContext* context, const GetChatServerReq* request,GetChatServerRsp* reply)
{

	auto server = SelectChatServer();

	reply->set_host(server.host);
	reply->set_port(server.port);
	reply->set_error(ErrorCodes::Success);
	reply->set_token(GenerateUniqueString());
	InsertToken(request->uid(), reply->token());
	std::cout << "Status 收到连接请求，UID: " << request->uid() << " Token: " << reply->token() << "\n";

	std::lock_guard<std::mutex> guard(TokenMtx);
	Tokens[request->uid()] = reply->token();
	return Status::OK;
}

ChatServer StatusServiceImpl::SelectChatServer()
{
	std::lock_guard<std::mutex> guard(ServerMtx);
	auto minServer = Servers.begin()->second;
	auto count_str = SRedisMgr::GetInstance().HGet(Prefix::LOGIN_COUNT, minServer.name);
	if (count_str.empty())
	{
		//不存在则默认设置为最大
		minServer.ConCount = INT_MAX;
	}
	else
	{
		minServer.ConCount = std::stoi(count_str);
	}

	// 使用范围基于for循环
	for (auto& server : Servers)
	{
		if (server.second.name == minServer.name)
		{
			continue;
		}

		auto count_str = SRedisMgr::GetInstance().HGet(Prefix::LOGIN_COUNT, server.second.name);
		if (count_str.empty())
		{
			server.second.ConCount = INT_MAX;
		}
		else
		{
			server.second.ConCount = std::stoi(count_str);
		}

		if (server.second.ConCount < minServer.ConCount)
		{
			minServer = server.second;
		}
	}

	return minServer;
}

Status StatusServiceImpl::Login(ServerContext* context, const LoginReq* request, LoginRsp* reply)
{
	auto uid = request->uid();
	auto token = request->token();

	std::string uid_str = std::to_string(uid);
	std::string token_key = Prefix::USERTOKENPREFIX + uid_str;
	std::string token_value = "";
	bool success = SRedisMgr::GetInstance().Get(token_key, token_value);
	if (success) {
		reply->set_error(ErrorCodes::UidInvalid);
		return Status::OK;
	}

	if (token_value != token) {
		reply->set_error(ErrorCodes::TokenInvalid);
		return Status::OK;
	}
	reply->set_error(ErrorCodes::Success);
	reply->set_uid(uid);
	reply->set_token(token);
	return Status::OK;
}

void StatusServiceImpl::InsertToken(int uid, std::string token)
{
	std::string uid_str = std::to_string(uid);
	std::string token_key = Prefix::USERTOKENPREFIX + uid_str;
	SRedisMgr::GetInstance().Set(token_key, token);
}
