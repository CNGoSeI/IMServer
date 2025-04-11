#ifndef STATUSGRPCCLIENT_H
#define STATUSGRPCCLIENT_H
#include "SingletonTemplate.h"
#include "StatusService.grpc.pb.h"
#include "ThreadWorkerTemplate.h"

using grpc::Channel;
using grpc::Status;
using grpc::ClientContext;

using message::GetChatServerReq;
using message::GetChatServerRsp;
using message::StatusService;
using message::LoginRsp;
using message::LoginReq;

using StatusStubUnique = std::unique_ptr<StatusService::Stub>;
using StatusStubPool_Unique = TThreadWoker<StatusStubUnique>;

/* 调用RPC接口，查询服务器状态，获取服务器连接 */
class SStatusGrpcClient :public TSingleton<SStatusGrpcClient>
{
	friend class TSingleton<SStatusGrpcClient>;
public:
	~SStatusGrpcClient() override = default;

	GetChatServerRsp GetChatServer(int uid);
	LoginRsp Login(int uid, std::string token);
private:
	SStatusGrpcClient();
	std::unique_ptr<StatusStubPool_Unique> ConnectPool;
	
};

#endif // STATUSGRPCCLIENT_H
