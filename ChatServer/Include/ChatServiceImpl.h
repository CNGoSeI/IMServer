#ifndef CHATSERVER_CHATSERVICEIMPL_H
#define CHATSERVER_CHATSERVICEIMPL_H
#include "message.grpc.pb.h"

struct UserInfo;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using message::AddFriendReq;
using message::AddFriendRsp;
using message::AuthFriendReq;
using message::AuthFriendRsp;
using message::TextChatMsgReq;
using message::TextChatMsgRsp;

class CChatServiceImpl : public message::ChatService::Service
{
public:
	CChatServiceImpl();
	Status NotifyAddFriend(ServerContext* context, const AddFriendReq* request,AddFriendRsp* reply) override;

	Status NotifyAuthFriend(ServerContext* context,const AuthFriendReq* request, AuthFriendRsp* response) override;

	//Status NotifyTextChatMsg(::grpc::ServerContext* context,const TextChatMsgReq* request, TextChatMsgRsp* response) override;
	Status NotifyTextChatMsg(::grpc::ServerContext* context, const TextChatMsgReq* request, TextChatMsgRsp* reply) override;
	bool GetBaseInfo(std::string base_key, int uid, std::shared_ptr<UserInfo>& userinfo);
};
#endif // CHATSERVER_CHATSERVICEIMPL_H
