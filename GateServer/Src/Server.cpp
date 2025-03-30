#include "Server.h"

#include <iostream>

#include "AsioIOServicePool.h"
#include "HttpConnection.h"

CServer::CServer(boost::asio::io_context& ioc, unsigned short& port) :
	Ioc(ioc),
	Acceptor(ioc, tcp::endpoint(tcp::v4(), port)) //tcp::v4() 相当于0.0.0.0 本机所有地址
{
}

void CServer::Start()
{
	/**
	 * 通过this构造shared_ptr会导致多个独立的控制块，引用计数不共享，对象被多次释放
	 * 安全地获取当前对象的shared_ptr，避免直接使用this
	 * 不能在构造函数中使用；必须由shared_ptr管理对象；必须public继承enable_shared_from_this
	 */
	auto self = shared_from_this();

	auto& IoContext = SAsioIOServicePool::GetInstance().GetIOService();//从链接池获取IOC
	auto new_con = std::make_shared<CHttpConnection>(IoContext);

	/**
	 * 接受连接，如若成功，转交给对应类处理，如果失败，继续等待连接
	 */
	Acceptor.async_accept(new_con->GetSocket(), [self, new_con](beast::error_code ec)
	{
		try
		{
			//出错则放弃这个连接，继续监听新链接
			if (ec)
			{
				self->Start();
				return;
			}

			/**
			 * 处理新链接，创建CHpptConnection类管理该类套接字的新连接
			 * 移动之后，当前套接字被转交给Http管理类，此对象不再拥有这个套接字所有权
			 * 下一次accept 将会得到一个新的套接字连接
			 */
			new_con->Start();
			//继续监听
			self->Start();
		}
		catch (std::exception& exp)
		{
			std::cout << "exception is " << exp.what() << std::endl;
			self->Start();
		}
	});
}
