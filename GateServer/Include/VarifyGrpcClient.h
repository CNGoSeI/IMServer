#ifndef  VARIFYGRPCCLIENT_H
#define  VARIFYGRPCCLIENT_H

/* 管理注册验证的类 */
#include <queue>
#include <grpcpp/grpcpp.h>
#include "message.grpc.pb.h"
#include "const.h"
#include "SingletonTemplate.h"

using grpc::Channel;
using grpc::Status;
using grpc::ClientContext;
using message::GetVarifyReq;
using message::GetVarifyRsp;
using message::VarifyService;


class RPConPool {
public:
	RPConPool(size_t poolSize, std::string host, std::string port)
		: PoolSize(poolSize), Host(host), Port(port), bStop(false) {
		for (size_t i = 0; i < PoolSize; ++i) {

			std::shared_ptr<Channel> channel = 
				grpc::CreateChannel(host + ":" + port,grpc::InsecureChannelCredentials());

			Connections.push(VarifyService::NewStub(channel));
		}
	}

	~RPConPool() {
		std::lock_guard<std::mutex> lock(mutex_);
		Close();
		while (!Connections.empty()) {
			Connections.pop();
		}
	}

	std::unique_ptr<VarifyService::Stub> GetConnection()
	{
		std::unique_lock<std::mutex> lock(mutex_);
		Cond.wait(lock, [this]
		{
			if (bStop)
			{
				return true;
			}
			return !Connections.empty();
		});
		//如果停止则直接返回空指针
		if (bStop)
		{
			return nullptr;
		}
		auto context = std::move(Connections.front());
		Connections.pop();
		return context;
	}

	/* 使用完成stub之后还回连接池 */
	void ReturnConnection(std::unique_ptr<VarifyService::Stub> context) {
		std::lock_guard<std::mutex> lock(mutex_);
		if (bStop) 
		{
			return;
		}

		Connections.push(std::move(context));
		Cond.notify_one();//还回来Stub,有余量了，可通知条件变量进行解锁判断
	}

	void Close() {
		bStop = true;
		Cond.notify_all();
	}

private:
	std::atomic<bool> bStop{false};
	size_t PoolSize;
	std::string Host;
	std::string Port;
	std::queue<std::unique_ptr<VarifyService::Stub>> Connections;//Stub是protoc编译器生成的类，作为客户端存根，用于处理与服务器的通信
	std::mutex mutex_;
	std::condition_variable Cond;
};


class CVerifyGrpcClient:public TSingleton<CVerifyGrpcClient>
{
	friend class TSingleton<CVerifyGrpcClient>;
public:
	GetVarifyRsp GetVarifyCode(const std::string& email) const;//获取验证码

private:
	CVerifyGrpcClient();
	std::unique_ptr<RPConPool> ConnectPool{nullptr};
};

#endif