#include "Session.h"

#include <iostream>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "ChatLogic.h"
#include "MsgNode.h"
#include "Server.h"

CSession::CSession(boost::asio::io_context& io_context, CServer* server) :
	Socket(io_context),
	Server(server)
{
	boost::uuids::uuid a_uuid = boost::uuids::random_generator()();
	UUid = boost::uuids::to_string(a_uuid);
	RcvHeadNode = std::make_shared<IMsgNode>(ChatServer::HEAD_TOTAL_LEN);
}

void CSession::Start()
{
	AsyncReadHead(ChatServer::HEAD_TOTAL_LEN);
}

void CSession::AsyncReadHead(int TotalLen)
{
	auto self = shared_from_this();
	AsyncReadFull(ChatServer::HEAD_TOTAL_LEN, [self](const boost::system::error_code& ec, const std::size_t BytesTransfered)
	{
		self->ReadHeadCallHandle(ec, BytesTransfered);
	});
}

void CSession::AsyncReadBody(const unsigned short TotalLen)
{
	auto self = shared_from_this();

	AsyncReadFull(TotalLen, [self, this, TotalLen](const boost::system::error_code& ec, const std::size_t bytes_transfered)
		{
			self->ReadBodyCallHandle(ec, bytes_transfered, TotalLen);
		});
}

void CSession::AsyncReadFull(const std::size_t maxLength, const FReadDataCallback& Callback)
{
	::memset(Data, 0, ChatServer::MAX_LENGTH);

	AsyncReadLen(0, maxLength, Callback);//从第0读取到第maxLength个数据
}


void CSession::AsyncReadLen(std::size_t read_len, std::size_t total_len, FReadDataCallback Callback)
{
	auto self = shared_from_this();

	//从Data + read_len读取total_len - read_len个数据 也就是从已读的最后位置到还未读取的大小位置
	Socket.async_read_some(boost::asio::buffer(Data + read_len, total_len - read_len),
	                       [read_len, total_len, Callback,self]
							//bytesTransfered为该次读取的大小
                       (const boost::system::error_code& ec, const std::size_t bytesTransfered)
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

void CSession::ReadHeadCallHandle(const boost::system::error_code& ec, const std::size_t BytesTransfered)
{
	try
	{
		if (ec)
		{
			std::cout << "回调读取包头错误： " << ec.what() << '\n';
			Close();
			Server->ClearSession(UUid);
			return;
		}

		//转换的数据长度比包头还少
		if (BytesTransfered < ChatServer::HEAD_TOTAL_LEN)
		{
			std::cout << "读取包头长度不匹配, 读取 [" << BytesTransfered << "] , 全部是 ["
				<< ChatServer::HEAD_TOTAL_LEN << "]" << '\n';
			Close();
			Server->ClearSession(UUid);
			return;
		}

		RcvHeadNode->Clear();
		memcpy(RcvHeadNode->Data, Data, BytesTransfered);

		//获取头部MSGID数据
		unsigned short msg_id = 0;
		memcpy(&msg_id, RcvHeadNode->Data, ChatServer::HEAD_ID_LEN);

		//网络字节序转化为本地字节序
		msg_id = boost::asio::detail::socket_ops::network_to_host_short(msg_id);
		std::cout << "msg_id = " << msg_id << '\n';

		//id非法
		if (msg_id > ChatServer::MAX_LENGTH)
		{
			std::cout << "非法的MSGID " << msg_id << '\n';
			Server->ClearSession(UUid);
			return;
		}

		unsigned short msg_len = 0;
		memcpy(&msg_len, RcvHeadNode->Data + ChatServer::HEAD_ID_LEN, ChatServer::HEAD_DATA_LEN);//写入head

		//网络字节序转化为本地字节序
		msg_len = boost::asio::detail::socket_ops::network_to_host_short(msg_len);
		std::cout << "信息长 = " << msg_len << '\n';

		//长度非法
		if (msg_len > ChatServer::MAX_LENGTH)
		{
			std::cout << "非法数据长度 =" << msg_len << '\n';
			Server->ClearSession(UUid);
			return;
		}

		RcvMsgNode = std::make_shared<CRecvNode>(msg_len, msg_id);
		AsyncReadBody(msg_len);
	}
	catch (std::exception& e)
	{
		std::cout << "读取包头异常码：" << e.what() << '\n';
	}
}


void CSession::ReadBodyCallHandle(const boost::system::error_code& ec, const std::size_t BytesTransfered, const unsigned short TotalLen)
{
	try
	{
		if (ec)
		{
			std::cout << "回调读取消息体错误: " << ec.what() << '\n';
			Close();
			Server->ClearSession(UUid);
			return;
		}

		if (BytesTransfered < TotalLen)
		{
			std::cout << "读取消息体长度不正确：[" << BytesTransfered << "] , total ["<< TotalLen << "]" << '\n';
			Close();
			Server->ClearSession(UUid);
			return;
		}

		memcpy(RcvMsgNode->Data, Data, BytesTransfered);
		RcvMsgNode->CurLen += static_cast<unsigned short>(BytesTransfered);
		RcvMsgNode->Data[RcvMsgNode->TotalLen] = '\0';
		std::cout << "receive data is " << RcvMsgNode->Data << '\n';

		//此处将消息投递到逻辑队列中
		SChatLogic::GetInstance().PostMsgToQue(make_shared<CLogicNode>(shared_from_this(), RcvMsgNode));

		/*
		 * 读取包体完成后，继续读包头。循环往复直到读完所有数据
		 * 采用的是asio异步的读写操作，不影响主线程
		 */
		AsyncReadHead(ChatServer::HEAD_TOTAL_LEN);
	}
	catch (std::exception& e)
	{
		std::cout << "读取信息体捕获异常：" << e.what() << '\n';
	}
}

void CSession::Close() {
	Socket.close();
	bClose = true;
}

void CSession::Send(const std::string& msg, short msgid) {
	std::lock_guard<std::mutex> lock(SendLock);

	auto send_que_size = SendQueue.size();
	if (send_que_size > ChatServer::MAX_SENDQUE) {
		std::cout << "会话: " << UUid << "发送队列错误，队列大小 " << ChatServer::MAX_SENDQUE << '\n';
		return;
	}

	SendQueue.push(std::make_shared<CSendNode>(msg.c_str(), msg.length(), msgid));

	/*
	 * push前队列内有消息不允许发送，需要等之前的发送之后（一次性消耗完成）才能再发送
	 * 但是return后SendLock已经解锁，其他线程又可以向队列push（如果回调不占有send锁）
	 * 也就是说只有第一个向队列投递消息的才能触发发送数据；触发async_write
	 *	 - async_write 回调函数会加锁，让队列被一次性消耗空，才能继续向队列添加消息
	 */
	if (send_que_size > 0) {
		return;
	}

	auto& msgnode = SendQueue.front();//获取但是不弹出

	auto self = shared_from_this();
	boost::asio::async_write(Socket, boost::asio::buffer(msgnode->Data, msgnode->TotalLen),
	                         [self,this](const boost::system::error_code& error, size_t bytes_transferred)
	                         {
		                         this->HandleWrite(error, self);
	                         });
}

void CSession::HandleWrite(const boost::system::error_code& error, std::shared_ptr<CSession> shared_self)
{
	//增加异常处理
	try
	{
		if (!error)
		{
			/*
			 * 与Send同锁，多个线程向队列里面此时不能添加消息
			 * 该回调会不断的递归调用，直到队列调用空，因此这里相当于一次性处理队列所有消息
			 */
			std::lock_guard<std::mutex> lock(SendLock);

			//cout << "send data " << _send_que.front()->_data+HEAD_LENGTH << endl;
			SendQueue.pop();//弹出

			/* 继续获取队列，如果存在消息，则继续发送，直到队列不再存在成员*/
			if (!SendQueue.empty())
			{
				auto& msgnode = SendQueue.front();
				auto self = shared_from_this();
				boost::asio::async_write(Socket, boost::asio::buffer(msgnode->Data, msgnode->TotalLen),
					[self, this](const boost::system::error_code& error, size_t bytes_transferred)
					{
						this->HandleWrite(error, self);
					});
			}
		}
		else
		{
			std::cout << "回调写数据错误：" << error.what() << '\n';
			Close();
			Server->ClearSession(UUid);
		}
	}
	catch (std::exception& e)
	{
		std::cerr << "回调写数据异常: " << e.what() << '\n';
	}
}

CLogicNode::CLogicNode(std::shared_ptr<CSession> session, std::shared_ptr<CRecvNode> recvnode):
	Session(session),
	RecvNode(recvnode)
{

}
