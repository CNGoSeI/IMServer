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
	 * ΪGrpc�ķ���ˣ�����Proto�ж���ķ���GetChatServer������Rpc��������һ�����������
	 * @param context Rpc���ù���
	 * @param request Proto�ж����Э�飬��������
	 * @param reply Proto�ж����Э�飬���ڻظ�
	 * @return Grpc��ִ�з���״̬
	 */
	Status GetChatServer
			(ServerContext* context, const GetChatServerReq* request,GetChatServerRsp* reply) override;

	std::vector<ChatServer> Servers;
	int ServerIndex;
	std::mutex ServerMtx;
};
#endif // STATUSSERVICEIMPL_H
