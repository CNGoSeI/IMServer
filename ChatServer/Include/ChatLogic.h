#ifndef CHATLOGIC_H
#define CHATLOGIC_H

#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <json/value.h>

#include "SingletonTemplate.h"

struct UserInfo;
class CLogicNode;
class CSession;
typedef std::function<void(std::shared_ptr<CSession>&, const short& msg_id, const std::string& msg_data)> FunCallBack;

class SChatLogic:public TSingleton<SChatLogic>
{
	friend class TSingleton<SChatLogic>;
public:
	~SChatLogic();

	/**
	 * 向消息队列投递消息，如果投递之后发现是首个消息，则通知工作线程进行消息处理
	 * 如果不是首个消息，则不断的投递，而工作线程则不断的处理
	 * @param msg 投递的信息
	 */
	void PostMsgToQue(std::shared_ptr <CLogicNode> msg);

/* ---请求回调分割线--- */
private:
	void LoginHandler(std::shared_ptr<CSession>& session, const short& msg_id, const std::string& msg_data);//注册的回调
	void SearchInfoHandler(std::shared_ptr<CSession> session, const short& msg_id, const std::string& msg_data);//查找用户的回调
	void AddFriendApplyHandler(std::shared_ptr<CSession> session, const short& msg_id, const std::string& msg_data);
	void AuthFriendApplyHandler(std::shared_ptr<CSession> session, const short msg_id, const std::string& msg_data);
	void DealChatTextMsg(std::shared_ptr<CSession> session, const short msg_id, const std::string& msg_data);
/* ---请求回调分割线--- */

private:
	
	SChatLogic();
	void DealMsg();
	void RegisterCallBacks();
	bool GetBaseInfo(std::string base_key, int uid, std::shared_ptr<UserInfo>& userinfo);
	void GetUserByUid(const std::string& uid_str, Json::Value& rtvalue);
	void GetUserByName(const std::string& name, Json::Value& rtvalue);

	std::thread WorkThread;
	std::queue<std::shared_ptr<CLogicNode>> MsgQue;
	std::mutex Mutex;
	std::condition_variable Consume;//条件变量
	bool bStop{false};
	std::map<unsigned short, FunCallBack> MsgId2Callback;
};

#endif // CHATLOGIC_H
