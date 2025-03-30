#ifndef SERVER_H
#define SERVER_H

#include <memory>
#include "const.h"

class CServer : public std::enable_shared_from_this<CServer>
{
public:
	CServer(boost::asio::io_context& ioc, unsigned short& port);
	void Start();

private:
	tcp::acceptor Acceptor;
	//监听IO的上下文，所有异步 I/O 操作的调度中心
	net::io_context& Ioc;
};

#endif
