#include "ChatGrpcClient.h"

#include <grpcpp/create_channel.h>

#include <MysqlDAO.h>
#include "ConfigMgr.h"


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
	return AddFriendRsp();
}

bool CChatGrpcClient::GetBaseInfo(std::string base_key, int uid, std::shared_ptr<UserInfo>& userinfo)
{
	return true;
}
