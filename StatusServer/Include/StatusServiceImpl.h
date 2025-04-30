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
using message::LoginReq;
using message::LoginRsp;

struct ChatServer {
	std::string host;
	std::string port;
	std::string name;
	int ConCount{0};//��������

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

	ChatServer SelectChatServer();
	Status Login(ServerContext* context, const LoginReq* request, LoginRsp* reply);
	void InsertToken(int uid, std::string token);

	std::unordered_map<std::string, ChatServer> Servers;
	std::unordered_map<int, std::string> Tokens;
	int ServerIndex;
	std::mutex ServerMtx;
	std::mutex TokenMtx;
};

#endif // STATUSSERVICEIMPL_H
