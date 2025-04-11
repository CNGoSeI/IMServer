#ifndef CHATDEFINE_H
#define CHATDEFINE_H

namespace ChatServer
{
	constexpr int MAX_LENGTH{1024 * 2};
	constexpr int HEAD_TOTAL_LEN{4}; //头部总长度
	constexpr int HEAD_ID_LEN{2}; //头部id长度
	constexpr int HEAD_DATA_LEN{2}; //头部数据长度
	constexpr int MAX_RECVQUE{10000};
	constexpr int MAX_SENDQUE{1000};
}

enum class MSG_IDS
{
	MSG_CHAT_LOGIN=1005,//用户登录
	MSG_CHAT_LOGIN_RSP = 1006, //用户登陆回包
};

#endif // !CHATDEFINE_H