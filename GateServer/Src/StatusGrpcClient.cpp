#include "StatusGrpcClient.h"
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>
#include "ConfigMgr.h"
#include "const.h"

GetChatServerRsp SStatusGrpcClient::GetChatServer(int uid)
{
	ClientContext context;
	GetChatServerRsp reply;
	GetChatServerReq request;
	request.set_uid(uid);

	auto stub = ConnectPool->GetWorker();

	/* 查阅协议，该Rpc，输入GetChatServerReq，返回GetChatServerRsp */
	Status status = stub->GetChatServer(&context, request, &reply);

	ConnectPool->ReturnWorker(std::move(stub));

	if (status.ok()) {
		return reply;
	}
	else {
		reply.set_error(ErrorCodes::RPCFailed);
		return reply;
	}
}

SStatusGrpcClient::SStatusGrpcClient()
{
	const auto& ConfigMgr = Mgr::GetConfigHelper();
	auto Host = ConfigMgr.get<std::string>("StatusServer.Host");
	auto Port = ConfigMgr.get<std::string>("StatusServer.Port");
	ConnectPool = StatusStubPool_Unique::CreateWorkThread([&Host, &Port]()
		{
			auto chanel = grpc::CreateChannel(
				Host + ":" + Port,
				grpc::InsecureChannelCredentials()
			);
			return std::move(StatusService::NewStub(chanel));
		},
		5
	);
}
