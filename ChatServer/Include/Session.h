#ifndef SESSION_H
#define SESSION_H
#include "ChatDefine.h"
#include <memory>
#include <queue>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;
class CServer;
class SLogicSystem;

class CSession:public std::enable_shared_from_this<CSession>
{
	using namespace std;
public:
	CSession(boost::asio::io_context& io_context, CServer* server);
	~CSession();

	//读取包头
	void AsyncReadHead(int TotalLen);
	//读取指定长度的数据
	void AsyncReadLen(std::size_t read_len, std::size_t total_len, std::function<void(const boost::system::error_code&, std::size_t)> Callback);
	//读取完整长度
	void AsyncReadFull(std::size_t maxLength, std::function<void(const boost::system::error_code&, std::size_t)> Callback);
	//读取包体
	void AsyncReadBody(short msg_len);
	void ReadHeadCallHandle(const boost::system::error_code&, std::size_t);
	void ReadBodyCallHandle(const boost::system::error_code& ec, std::size_t BytesTransfered, short TotalLen);

private:
	//void HandleRead(const boost::system::error_code& error, size_t  bytes_transferred, std::shared_ptr<CSession> shared_self);
	//void HandleWrite(const boost::system::error_code& error, std::shared_ptr<CSession> shared_self);
	void Start();
	void Close();
	tcp::socket Socket;
	std::string UUid;
	char Data[ChatServer::MAX_LENGTH];
	CServer* Server;
	bool bClose{false};
	std::queue<std::shared_ptr<SendNode> > SendQueue;
	std::mutex _send_lock;
	//收到的消息结构
	std::shared_ptr<RecvNode> RcvMsgNode;
	bool bHeadParse{ false };
	//收到的头部结构
	std::shared_ptr<MsgNode> RcvHeadNode;
};

class LogicNode {
	friend class LogicSystem;
public:
	LogicNode(std::shared_ptr<CSession>, std::shared_ptr<RecvNode>);
private:
	std::shared_ptr<CSession> _session;
	std::shared_ptr<RecvNode> _recvnode;
};
#endif // SESSION_H
