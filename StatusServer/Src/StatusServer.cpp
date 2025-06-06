﻿// StatusServer.cpp: 目标的源文件。

#include "StatusServer.h"
#include <grpcpp/server_builder.h>
#include <iostream>
#include "const.h"
#include "ConfigMgr.h"
//#include "RedisMgr.h"
#include "AsioIOServicePool.h"
#include <memory>
#include <string>
#include <thread>
#include <boost/asio.hpp>
#include "StatusServiceImpl.h"

void RunServer() {
	auto& Config = Mgr::GetConfigHelper();
	
	const auto Addr = Config.get<std::string>("StatusServer.Host") + ":" + Config.get<std::string>("StatusServer.Port");
	
	std::string server_address(Addr);
	std::cout << "Status连接地址 " << Addr << std::endl;
	StatusServiceImpl service;

	grpc::ServerBuilder builder;

	// 监听端口和添加服务
	builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
	builder.RegisterService(&service);

	// 构建并启动gRPC服务器
	std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
	std::cout << "Status服务器监听 " << server_address << std::endl;

	// 创建Boost.Asio的io_context
	boost::asio::io_context io_context;

	// 创建signal_set用于捕获SIGINT
	boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
	// 设置异步等待SIGINT信号
	signals.async_wait([&server](const boost::system::error_code& error, int signal_number) {
		if (!error) {
			std::cout << "Shutting down server..." << std::endl;
			server->Shutdown(); // 优雅地关闭服务器
		}
		});

	// 在单独的线程中运行io_context
	std::thread([&io_context]() { io_context.run(); }).detach();

	// 等待服务器关闭
	server->Wait();
	io_context.stop(); // 停止io_context
}

int main()
{
	try {
		RunServer();
	}
	catch (std::exception const& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return 0;
}
