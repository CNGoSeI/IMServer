#include "MsgNode.h"

#include "ChatDefine.h"
#include "const.h"

IMsgNode::IMsgNode(const unsigned short max_len) : TotalLen(max_len)
{
	Data = new char[TotalLen + 1]();
	Data[TotalLen] = '\0';
}

IMsgNode::~IMsgNode()
{
	std::cout << "析构 IMsgNode" << endl;
	delete[] Data;
}

void IMsgNode::Clear()
{
	memset(Data, 0, TotalLen);
	CurLen = 0;
}

CRecvNode::CRecvNode(const unsigned short max_len, const unsigned short msg_id):
	IMsgNode(max_len),
	MsgID(msg_id)
{

}

CSendNode::CSendNode(const char* msg, const unsigned short max_len, const unsigned short msg_id) :
	IMsgNode(max_len + ChatServer::HEAD_TOTAL_LEN),
	MsgID(msg_id)
{
	//先发送id, 转为网络字节序
	unsigned short msg_id_host = boost::asio::detail::socket_ops::host_to_network_short(msg_id);
	memcpy(Data, &msg_id_host, ChatServer::HEAD_ID_LEN);//ID

	//转为网络字节序
	unsigned short max_len_host = boost::asio::detail::socket_ops::host_to_network_short(max_len);
	memcpy(Data + ChatServer::HEAD_ID_LEN, &max_len_host, ChatServer::HEAD_DATA_LEN);//消息长度
	memcpy(Data + ChatServer::HEAD_ID_LEN + ChatServer::HEAD_DATA_LEN, msg, max_len);//消息体
}
