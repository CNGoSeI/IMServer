#include "const.h"
#include "AsioIOServicePool.h"
#include <iostream>
#include "ConfigMgr.h"
#include "Server.h"

bool bstop = false;
std::condition_variable cond_quit;
std::mutex mutex_quit;

int main()
{
	try
	{
		auto& cfg = Mgr::GetConfigHelper();
		auto& pool = SAsioIOServicePool::GetInstance();
		boost::asio::io_context io_context;
		boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);

		signals.async_wait([&io_context, pool](auto, auto)
		{
			io_context.stop();
			pool.Stop();
		});
		auto Port = cfg.get<unsigned short>("SelfServer.Port");
		CServer s(io_context, Port);//启动TCP服务
		io_context.run();
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << std::endl;
	}
}
