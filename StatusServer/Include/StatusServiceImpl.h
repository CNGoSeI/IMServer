#ifndef STATUSSERVICEIMPL_H
#define STATUSSERVICEIMPL_H
#include "StatusService.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using message::GetChatServerReq;
using message::GetChatServerRsp;
using message::StatusService;

struct ChatServer {
	std::string host;
	std::string port;
};

class StatusServiceImpl final : public StatusService::Service
{
public:
	StatusServiceImpl();

	/**
	 * 为Grpc的服务端，重载Proto中定义的方法GetChatServer，进行Rpc处理，返回一个聊天服务器
	 * @param context Rpc调用关联
	 * @param request Proto中定义的协议，用于请求
	 * @param reply Proto中定义的协议，用于回复
	 * @return Grpc的执行返回状态
	 */
	Status GetChatServer
			(ServerContext* context, const GetChatServerReq* request,GetChatServerRsp* reply) override;

	std::vector<ChatServer> Servers;
	int ServerIndex;
	std::mutex ServerMtx;
};
#endif // STATUSSERVICEIMPL_H
