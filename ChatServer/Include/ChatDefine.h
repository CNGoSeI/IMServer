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
	ID_SEARCH_USER_REQ = 1007, //用户搜索请求
	ID_SEARCH_USER_RSP = 1008, //搜索用户回包
	ID_ADD_FRIEND_REQ = 1009, //申请添加好友请求
	ID_ADD_FRIEND_RSP = 1010, //申请添加好友回复
	ID_NOTIFY_ADD_FRIEND_REQ = 1011,  //通知用户添加好友申请
	ID_AUTH_FRIEND_REQ = 1013,  //认证好友请求
	ID_AUTH_FRIEND_RSP = 1014,  //认证好友回复
	ID_NOTIFY_AUTH_FRIEND_REQ = 1015, //通知用户认证好友申请
	ID_TEXT_CHAT_MSG_REQ = 1017, //文本聊天信息请求
	ID_TEXT_CHAT_MSG_RSP = 1018, //文本聊天信息回复
	ID_NOTIFY_TEXT_CHAT_MSG_REQ = 1019, //通知用户文本聊天信息
	ID_NOTIFY_OFF_LINE_REQ = 1021, //通知用户下线
	ID_HEART_BEAT_REQ = 1023,      //心跳请求
	ID_HEARTBEAT_RSP = 1024,       //心跳回复
};

#endif // !CHATDEFINE_H