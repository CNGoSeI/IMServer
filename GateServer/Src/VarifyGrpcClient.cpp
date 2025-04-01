#include "VarifyGrpcClient.h"

/* gRpc@1.71 */
CVerifyGrpcClient::CVerifyGrpcClient()
{
	const auto& ConfigMgr = GateConfig::GetConfigHelper();
	auto Host = ConfigMgr.get<std::string>("VarifyServer.Host");
	auto Port= ConfigMgr.get<std::string>("VarifyServer.Port");
	ConnectPool = std::make_unique<RPConPool>(5, Host, Port);
}

GetVarifyRsp CVerifyGrpcClient::GetVarifyCode(const std::string& email) const
{
	ClientContext context;
	GetVarifyRsp reply;
	GetVarifyReq request;

	request.set_email(email);

	auto Stub = ConnectPool->GetConnection();
	Status status = Stub->GetVarifyCode(&context, request, &reply);

	if (status.ok()) {
		ConnectPool->ReturnConnection(std::move(Stub));
		return reply;
	}
	else {
		ConnectPool->ReturnConnection(std::move(Stub));
		reply.set_error(ErrorCodes::RPCFailed);
		return reply;
	}
}
