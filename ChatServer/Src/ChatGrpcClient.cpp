#include "ChatGrpcClient.h"

#include <grpcpp/create_channel.h>

#include <MysqlDAO.h>
#include "ConfigMgr.h"
#include "const.h"

CChatGrpcClient::CChatGrpcClient()
{
	const auto& ConfigMgr = Mgr::GetConfigHelper();
	auto ServerList = ConfigMgr.get<std::string>("PeerServer.Servers"); //读取按逗号分割的对端服务器列表

	std::vector<std::string> words;

	std::stringstream ss(ServerList);
	std::string word;

	while (std::getline(ss, word, ','))
	{
		words.push_back(word);
	}

	for (auto& word : words)
	{
		auto ServerName = ConfigMgr.get<std::string>(word + '.' + "Name");
		if (ServerName.empty())
		{
			continue;
		}
		auto Host = ConfigMgr.get<std::string>(word + '.' + "Host");
		auto Port = ConfigMgr.get<std::string>(word + '.' + "Port");

		Pools.emplace(
			ServerName,
			StubPool_Unique::CreateWorkThread([&Host, &Port]()
				{
					auto chanel = grpc::CreateChannel(
						Host + ":" + Port,
						grpc::InsecureChannelCredentials()
					);
					return std::move(ChatService::NewStub(chanel));
				}
			));
	}
}

AddFriendRsp CChatGrpcClient::NotifyAddFriend(std::string server_ip, const AddFriendReq& req)
{
	AddFriendRsp rsp;

	auto exe = [this,&rsp,&server_ip,&req]()
	{
		auto find_iter = Pools.find(server_ip);
		if (find_iter == Pools.end())
		{
			return;
		}

		auto& pool = find_iter->second;
		ClientContext context;
		auto stub = pool->GetWorker();
		Status status = stub->NotifyAddFriend(&context, req, &rsp);

		if (!status.ok())
		{
			rsp.set_error(ErrorCodes::RPCFailed);
		}
		pool->ReturnWorker(std::move(stub));
	};

	exe();
	rsp.set_error(ErrorCodes::Success);
	rsp.set_applyuid(req.applyuid());
	rsp.set_touid(req.touid());

	return rsp;
}

AuthFriendRsp CChatGrpcClient::NotifyAuthFriend(std::string server_ip, const AuthFriendReq& req)
{
	AuthFriendRsp rsp;
	rsp.set_error(ErrorCodes::Success);


	auto find_iter = Pools.find(server_ip);
	if (find_iter == Pools.end()) {
		return rsp;
	}

	auto& pool = find_iter->second;
	ClientContext context;
	auto stub = pool->GetWorker();
	Status status = stub->NotifyAuthFriend(&context, req, &rsp);
	pool->ReturnWorker(std::move(stub));
	rsp.set_fromuid(req.fromuid());
	rsp.set_touid(req.touid());
	if (!status.ok()) {
		rsp.set_error(ErrorCodes::RPCFailed);
		return rsp;
	}

	return rsp;
}

bool CChatGrpcClient::GetBaseInfo(std::string base_key, int uid, std::shared_ptr<UserInfo>& userinfo)
{
	return true;
}

message::TextChatMsgRsp CChatGrpcClient::NotifyTextChatMsg(std::string server_ip, const message::TextChatMsgReq& req,
	const Json::Value& rtvalue)
{
	TextChatMsgRsp rsp;
	rsp.set_error(ErrorCodes::Success);

	auto exe = [&rsp, this, &server_ip, &req]()
	{
			auto find_iter = Pools.find(server_ip);
			if (find_iter == Pools.end()) {
				return;
			}

			auto& pool = find_iter->second;
			ClientContext context;
			auto stub = pool->GetWorker();
			Status status = stub->NotifyTextChatMsg(&context, req, &rsp);
			pool->ReturnWorker(std::move(stub));

			if (!status.ok()) {
				rsp.set_error(ErrorCodes::RPCFailed);
			}

	};

	exe();
	rsp.set_fromuid(req.fromuid());
	rsp.set_touid(req.touid());
	for (const auto& text_data : req.textmsgs()) {
		message::TextChatData* new_msg = rsp.add_textmsgs();
		new_msg->set_msgid(text_data.msgid());
		new_msg->set_msgcontent(text_data.msgcontent());
	}
	return rsp;
}
