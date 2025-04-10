#ifndef SERVER_H
#define SERVER_H

#include <map>
#include <mutex>
#include <boost/asio.hpp>

class CSession;
using boost::asio::ip::tcp;

class CServer
{
public:
	CServer(boost::asio::io_context& io_context, short port);
	~CServer();
	void ClearSession(const std::string&);
private:
	void HandleAccept(std::shared_ptr<CSession>, const boost::system::error_code& error);
	void StartAccept();
	boost::asio::io_context& IoContext;
	short Port;
	tcp::acceptor Acceptor;
	std::map<std::string, std::shared_ptr<CSession>> Str2Session;//UUID映射会话
	std::mutex Mutex;
};

#endif // SERVER_H
