#ifndef SESSION_H
#define SESSION_H
#include "ChatDefine.h"
#include <memory>
#include <queue>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;
class CServer;
class SLogicSystem;
class CRecvNode;
class IMsgNode;
class CSendNode;


class CSession:public std::enable_shared_from_this<CSession>
{
	using FReadDataCallback = std::function<void(const boost::system::error_code&, std::size_t)>;
public:
	CSession(boost::asio::io_context& io_context, CServer* server);
	~CSession()=default;

	void Start();
	void Close();

	void Send(const std::string& msg, short msgid);
	

	//读取包头
	void AsyncReadHead(int TotalLen);

	/* 读取从read_len到total_len的数据 */
	void AsyncReadLen(std::size_t read_len, std::size_t total_len, FReadDataCallback Callback);

	/**
	 * 读取数据，读取完成之后调用 Callback
	 * @param maxLength 将要读取的大小，读到了的数据=该大小则调用回调
	 * @param Callback  将要调用的回调
	 */
	void AsyncReadFull(std::size_t maxLength, const FReadDataCallback& Callback);

	//读取包体
	void AsyncReadBody(const unsigned short msg_len);
	void ReadHeadCallHandle(const boost::system::error_code&, std::size_t);
	void ReadBodyCallHandle(const boost::system::error_code& ec, std::size_t BytesTransfered, const unsigned short TotalLen);

	const std::string& GetUUID()const { return UUid; };
	tcp::socket& GetSocket() { return Socket; };
	int GetUserId()const { return UserUId; };
	void SetUserId(int id) { UserUId = id; };
private:
	//void HandleRead(const boost::system::error_code& error, size_t  bytes_transferred, std::shared_ptr<CSession> shared_self);
	void HandleWrite(const boost::system::error_code& error, std::shared_ptr<CSession> shared_self);

	tcp::socket Socket;
	std::string UUid;
	int UserUId{0};
	char Data[ChatServer::MAX_LENGTH]{0};
	CServer* Server;
	bool bClose{false};
	std::queue<std::shared_ptr<CSendNode> > SendQueue;
	std::mutex SendLock;
	//收到的消息结构
	std::shared_ptr<CRecvNode> RcvMsgNode;
	bool bHeadParse{ false };
	//收到的头部结构
	std::shared_ptr<IMsgNode> RcvHeadNode;
};

class CLogicNode {
	friend class SChatLogic;
public:
	CLogicNode(std::shared_ptr<CSession>, std::shared_ptr<CRecvNode>);
private:
	std::shared_ptr<CSession> Session;
	std::shared_ptr<CRecvNode> RecvNode;
};

#endif // SESSION_H
