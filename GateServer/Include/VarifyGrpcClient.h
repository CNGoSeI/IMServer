#ifndef  VARIFYGRPCCLIENT_H
#define  VARIFYGRPCCLIENT_H

/**
 * <mysql/jdbc.h>和<mysqlx/xdevapi.h>两个头文件中的内容与GRPC存在冲突
 * 如果这两个头文件中的内容先于该文件包含，则会报很多语法错误
 * 因此最好优先包含这个，或者尽量减少.h中头文件的引入
 */

#include <grpcpp/grpcpp.h>
#include "message.grpc.pb.h"
#include "const.h"
#include "SingletonTemplate.h"
#include "ThreadWorkerTemplate.h"

using grpc::Channel;
using grpc::Status;
using grpc::ClientContext;
using message::GetVarifyReq;
using message::GetVarifyRsp;
using message::VarifyService;

using StubUnique = std::unique_ptr<VarifyService::Stub>;
using StubPool_Unique = TThreadWoker<StubUnique>;

/* 管理注册验证的类 */
class CVerifyGrpcClient:public TSingleton<CVerifyGrpcClient>
{
	friend class TSingleton<CVerifyGrpcClient>;
public:
	GetVarifyRsp GetVarifyCode(const std::string& email) const;//获取验证码

private:
	CVerifyGrpcClient();
	std::unique_ptr<StubPool_Unique> ConnectPool;
};

#endif