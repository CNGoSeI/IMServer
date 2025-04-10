#include "Session.h"

#include <iostream>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "ChatLogic.h"
#include "Server.h"

CSession::CSession(boost::asio::io_context& io_context, CServer* server) :
	Socket(io_context),
	Server(server)
{
	boost::uuids::uuid a_uuid = boost::uuids::random_generator()();
	UUid = boost::uuids::to_string(a_uuid);
	RcvHeadNode = std::make_shared<MsgNode>(ChatServer::HEAD_TOTAL_LEN);
}

void CSession::Start()
{
	AsyncReadHead(ChatServer::HEAD_TOTAL_LEN);
}

void CSession::AsyncReadHead(int TotalLen)
{
	auto self = shared_from_this();
	AsyncReadFull(ChatServer::HEAD_TOTAL_LEN, [self](const boost::system::error_code& ec, std::size_t BytesTransfered)
	{
		self->ReadHeadCallHandle(ec, BytesTransfered);
	});
}

void CSession::AsyncReadBody(short TotalLen)
{
	auto self = shared_from_this();

	AsyncReadFull(TotalLen, [self, this, TotalLen](const boost::system::error_code& ec, std::size_t bytes_transfered)
		{
			self->ReadBodyCallHandle(ec, bytes_transfered, TotalLen);
		});
}

void CSession::AsyncReadLen(std::size_t read_len, std::size_t total_len, std::function<void(const boost::system::error_code&, std::size_t)> Callback)
{
	auto self = shared_from_this();
	Socket.async_read_some(boost::asio::buffer(Data + read_len, total_len - read_len),
	                       [read_len, total_len, Callback,self](const boost::system::error_code& ec,
	                                                            std::size_t bytesTransfered)
	                       {
		                       if (ec)
		                       {
			                       // 出现错误，调用回调函数
								   Callback(ec, read_len + bytesTransfered);
			                       return;
		                       }

		                       if (read_len + bytesTransfered >= total_len)
		                       {
			                       //长度够了就调用回调函数
								   Callback(ec, read_len + bytesTransfered);
			                       return;
		                       }

		                       // 没有错误，且长度不足则继续读取
		                       self->AsyncReadLen(read_len + bytesTransfered, total_len, Callback);
	                       });
}

void CSession::AsyncReadFull(std::size_t maxLength, std::function<void(const boost::system::error_code&, std::size_t)> Callback)
{
	::memset(Data, 0, ChatServer::MAX_LENGTH);
	AsyncReadLen(0, maxLength, Callback);
}

void CSession::ReadHeadCallHandle(const boost::system::error_code& ec, std::size_t BytesTransfered)
{
	try
	{
		if (ec)
		{
			std::cout << "回调读取包头错误： " << ec.what() << std::endl;
			Close();
			Server->ClearSession(UUid);
			return;
		}

		//转换的数据长度比包头还少
		if (BytesTransfered < ChatServer::HEAD_TOTAL_LEN)
		{
			std::cout << "读取包头长度不匹配, 读取 [" << BytesTransfered << "] , 全部是 ["
				<< ChatServer::HEAD_TOTAL_LEN << "]" << std::endl;
			Close();
			Server->ClearSession(UUid);
			return;
		}

		RcvHeadNode->Clear();
		memcpy(RcvHeadNode->Data, Data, BytesTransfered);

		//获取头部MSGID数据
		short msg_id = 0;
		memcpy(&msg_id, RcvHeadNode->Data, ChatServer::HEAD_ID_LEN);

		//网络字节序转化为本地字节序
		msg_id = boost::asio::detail::socket_ops::network_to_host_short(msg_id);
		std::cout << "msg_id = " << msg_id << std::endl;

		//id非法
		if (msg_id > ChatServer::MAX_LENGTH)
		{
			std::cout << "非法的MSGID " << msg_id << std::endl;
			Server->ClearSession(UUid);
			return;
		}

		short msg_len = 0;
		memcpy(&msg_len, RcvHeadNode->Data + ChatServer::HEAD_ID_LEN, ChatServer::HEAD_DATA_LEN);

		//网络字节序转化为本地字节序
		msg_len = boost::asio::detail::socket_ops::network_to_host_short(msg_len);
		std::cout << "信息长 = " << msg_len << std::endl;

		//长度非法
		if (msg_len > ChatServer::MAX_LENGTH)
		{
			std::cout << "非法数据长度 =" << msg_len << std::endl;
			Server->ClearSession(UUid);
			return;
		}

		RcvMsgNode = std::make_shared<RecvNode>(msg_len, msg_id);
		AsyncReadBody(msg_len);
	}
	catch (std::exception& e)
	{
		std::cout << "读取包头异常码：" << e.what() << std::endl;
	}
}


void CSession::ReadBodyCallHandle(const boost::system::error_code& ec, std::size_t BytesTransfered, short TotalLen)
{
	try
	{
		if (ec)
		{
			std::cout << "回调读取消息体错误: " << ec.what() << std::endl;
			Close();
			Server->ClearSession(UUid);
			return;
		}

		if (BytesTransfered < TotalLen)
		{
			std::cout << "读取消息体长度不正确：[" << BytesTransfered << "] , total ["
				<< TotalLen << "]" << std::endl;
			Close();
			Server->ClearSession(UUid);
			return;
		}

		memcpy(RcvMsgNode->Data, Data, BytesTransfered);
		RcvMsgNode->CurLen += BytesTransfered;
		RcvMsgNode->Data[RcvMsgNode->TotalLen] = '\0';
		std::cout << "receive data is " << RcvMsgNode->Data << std::endl;
		//此处将消息投递到逻辑队列中
		SChatLogic::GetInstance().PostMsgToQue(make_shared<LogicNode>(shared_from_this(), RcvMsgNode));

		/*
		 * 读取包体完成后，继续读包头。循环往复直到读完所有数据
		 * 采用的是asio异步的读写操作，不影响主线程
		 */
		AsyncReadHead(ChatServer::HEAD_TOTAL_LEN);
	}
	catch (std::exception& e)
	{
		std::cout << "读取信息体捕获异常：" << e.what() << std::endl;
	}
}

void CSession::Close() {
	Socket.close();
	bClose = true;
}

void CSession::Send(std::string msg, short msgid) {
	std::lock_guard<std::mutex> lock(_send_lock);
	int send_que_size = SendQueue.size();
	if (send_que_size > ChatServer::MAX_SENDQUE) {
		std::cout << "会话: " << UUid << "发送队列错误，队列大小 " << ChatServer::MAX_SENDQUE << std::endl;
		return;
	}

	SendQueue.push(std::make_shared<SendNode>(msg.c_str(), msg.length(), msgid));
	if (send_que_size > 0) {
		return;
	}
	auto& msgnode = SendQueue.front();
	boost::asio::async_write(Socket, boost::asio::buffer(msgnode->Data, msgnode->TotalLen),
		std::bind(&CSession::HandleWrite, this, std::placeholders::_1, SharedSelf()));
}