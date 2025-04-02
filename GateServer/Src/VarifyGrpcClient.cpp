#include "VarifyGrpcClient.h"

#include "ConfigMgr.h"

/* gRpc@1.71 */
CVerifyGrpcClient::CVerifyGrpcClient()
{
	const auto& ConfigMgr = Mgr::GetConfigHelper();
	auto Host = ConfigMgr.get<std::string>("VarifyServer.Host");
	auto Port= ConfigMgr.get<std::string>("VarifyServer.Port");
	//ConnectPool = std::make_unique<RPConPool>(5, Host, Port);
	ConnectPool = StubPool_Unique::CreateWorkThread([&Host,&Port]()
	{
			auto chanel = grpc::CreateChannel(
				Host + ":" + Port,
				grpc::InsecureChannelCredentials()
			);
			return std::move(VarifyService::NewStub(chanel));
	},
	5
	);
}

GetVarifyRsp CVerifyGrpcClient::GetVarifyCode(const std::string& email) const
{
	ClientContext context;
	GetVarifyRsp reply;
	GetVarifyReq request;

	request.set_email(email);

	auto Stub = ConnectPool->GetWorker();
	Status status = Stub->GetVarifyCode(&context, request, &reply);

	if (status.ok()) {
		ConnectPool->ReturnWorker(std::move(Stub));
		return reply;
	}
	else {
		ConnectPool->ReturnWorker(std::move(Stub));
		reply.set_error(ErrorCodes::RPCFailed);
		return reply;
	}
}
