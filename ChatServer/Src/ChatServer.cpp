#include "const.h"
#include "AsioIOServicePool.h"
#include <iostream>
#include <grpcpp/server_builder.h>

#include "ChatServiceImpl.h"
#include "ConfigMgr.h"
#include "RedisMgr.h"
#include "Server.h"

bool bstop = false;
std::condition_variable cond_quit;
std::mutex mutex_quit;

int main()
{
	auto& cfg = Mgr::GetConfigHelper();
	auto ServerName = cfg.get<std::string>("SelfServer.Name");
	try
	{
		auto& pool = SAsioIOServicePool::GetInstance();
		SRedisMgr::GetInstance().HSet(Prefix::LOGIN_COUNT, ServerName, "0");

		std::string ServerAddress(cfg.get<std::string>("SelfServer.Host") + ':' + cfg.get<std::string>("SelfServer.RPCPort"));
		CChatServiceImpl service;
		grpc::ServerBuilder builder;
		// 监听端口和添加服务
		builder.AddListeningPort(ServerAddress, grpc::InsecureServerCredentials());
		builder.RegisterService(&service);
		// 构建并启动gRPC服务器
		std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
		std::cout << "RPC Server 监听 " << ServerAddress << std::endl;
		//单独启动一个线程处理grpc服务
		std::thread grpc_server_thread([&server]()
		{
			server->Wait();
		});

		boost::asio::io_context io_context;
		boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);

		signals.async_wait([&io_context, &pool, &server](auto, auto)
		{
			io_context.stop();
			pool.Stop();
			server->Shutdown();
		});
		auto Port = cfg.get<unsigned short>("SelfServer.Port");
		CServer s(io_context, Port);//启动TCP服务
		io_context.run();

		SRedisMgr::GetInstance().HDel(Prefix::LOGIN_COUNT, ServerName);
		grpc_server_thread.join();
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << std::endl;
		SRedisMgr::GetInstance().HDel(Prefix::LOGIN_COUNT, ServerName);
	}
}
