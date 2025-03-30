#include "AsioIOServicePool.h"
#include <iostream>

using namespace std;

SAsioIOServicePool::SAsioIOServicePool(std::size_t size) : IoServices(size),
                                                           Works(size), NextIOService(0)
{
	//用work 监管IoServices，免得线程执行完一次之后直接退出，从而导致关闭io
	for (std::size_t i = 0; i < size; ++i)
	{
		Works[i] = std::unique_ptr<Work>(new Work(IoServices[i].get_executor()));
	}

	//遍历多个ioservice，创建多个线程，每个线程内部启动ioservice
	for (std::size_t i = 0; i < IoServices.size(); ++i)
	{
		Threads.emplace_back([this, i]()
		{
			IoServices[i].run(); //被executor_Work接管，执行完成也不会线程结束
		});
	}
}

SAsioIOServicePool::~SAsioIOServicePool()
{
	Stop();
	std::cout << "AsioIOServicePool 析构" << endl;
}

boost::asio::io_context& SAsioIOServicePool::GetIOService()
{
	auto& service = IoServices[NextIOService++];
	if (NextIOService == IoServices.size())
	{
		NextIOService = 0;
	}
	return service;
}

void SAsioIOServicePool::Stop()
{
	//因为仅仅执行work.reset并不能让iocontext从run的状态中退出
	//当iocontext已经绑定了读或写的监听事件后，还需要手动stop该服务。

	//把服务先停止
	for(auto& Ioc: IoServices)
	{
		Ioc.stop();
	}

	for (auto& work : Works)
	{
		work.reset();
	}

	for (auto& t : Threads)
	{
		t.join();
	}
}
