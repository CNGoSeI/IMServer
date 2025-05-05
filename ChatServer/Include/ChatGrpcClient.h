#ifndef CHATSERVER_CHATGRPCCLIENT_H
#define CHATSERVER_CHATGRPCCLIENT_H
#include "ThreadWorkerTemplate.h"
#include "message.grpc.pb.h"
#include "message.pb.h"
#include "SingletonTemplate.h"
#include "StatusService.pb.h"
#include "StatusService.grpc.pb.h"

/* grpc通信调用类，实际相关处理查看CChatServiceImpl */

namespace Json
{
	class Value;
}

struct UserInfo;
using grpc::Channel;
using grpc::Status;
using grpc::ClientContext;

using message::AddFriendReq;
using message::AddFriendRsp;

using message::GetChatServerRsp;
using message::LoginRsp;
using message::LoginReq;
using message::ChatService;
using message::AuthFriendRsp;
using message::AuthFriendReq;
using message::TextChatMsgRsp;
using message::TextChatMsgReq;

using ChatServiceStubUnique = std::unique_ptr<ChatService::Stub>;
using StubPool_Unique = TThreadWoker<ChatServiceStubUnique>;

class CChatGrpcClient:public TSingleton<CChatGrpcClient>
{
	friend class TSingleton<CChatGrpcClient>;
public:
	AddFriendRsp NotifyAddFriend(std::string server_ip, const AddFriendReq& req);
	AuthFriendRsp NotifyAuthFriend(std::string server_ip, const AuthFriendReq& req);
	bool GetBaseInfo(std::string base_key, int uid, std::shared_ptr<UserInfo>& userinfo);
	message::TextChatMsgRsp NotifyTextChatMsg(std::string server_ip, const message::TextChatMsgReq& req, const Json::Value& rtvalue);
	//TextChatMsgRsp NotifyTextChatMsg(std::string server_ip, const TextChatMsgReq& req, const Json::Value& rtvalue);
private:
	CChatGrpcClient();
	std::unordered_map<std::string, std::unique_ptr<StubPool_Unique>> Pools;
};
#endif // CHATSERVER_CHATGRPCCLIENT_H
