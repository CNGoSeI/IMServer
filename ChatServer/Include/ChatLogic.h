#ifndef CHATLOGIC_H
#define CHATLOGIC_H

#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

#include "SingletonTemplate.h"

class LogicNode;
class CSession;
typedef std::function<void(std::shared_ptr<CSession>&, const short& msg_id, const std::string& msg_data)> FunCallBack;

class SChatLogic:public TSingleton<SChatLogic>
{
	friend class TSingleton<SChatLogic>;
public:
	~SChatLogic();
	void PostMsgToQue(std::shared_ptr <LogicNode> msg);
private:
	SChatLogic();
	void DealMsg();
	void RegisterCallBacks();
	void HelloWordCallBack(std::shared_ptr<CSession>&, const short& msg_id, const std::string& msg_data);
	void LoginHandler(std::shared_ptr<CSession>& session, const short& msg_id, const std::string& msg_data);
	std::thread WorkThread;
	std::queue<std::shared_ptr<LogicNode>> MsgQue;
	std::mutex Mutex;
	std::condition_variable Consume;//条件变量
	bool bStop{false};
	std::map<short, FunCallBack> MsgId2Callback;
};
#endif // CHATLOGIC_H
