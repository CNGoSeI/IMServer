#include <iostream>
#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>
#include "const.h"
#include "Server.h"

int main()
{
	try
	{
		 unsigned short port = GateConfig::GetConfigHelper().get<unsigned short>("GateServer.Port");
		net::io_context ioc{1};//预分配的执行线程数

		//监听操作系统的 ​**SIGINT（Ctrl+C 终止信号）​** 和 ​**SIGTERM（请求终止进程的信号）将其关联到ioc
		boost::asio::signal_set signals(ioc, SIGINT, SIGTERM);

		//signals 监听到 SIGINT 或 SIGTERM 信号时，触发回调函数
		signals.async_wait([&ioc](const boost::system::error_code& error, int signal_number)
		{
			if (error)
			{
				return;
			}

			ioc.stop();
		});

		std::make_shared<CServer>(ioc, port)->Start();

		//运行事件循环
		ioc.run();
	}
	catch (std::exception const& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}
}
