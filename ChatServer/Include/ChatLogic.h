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

class CLogicNode;
class CSession;
typedef std::function<void(std::shared_ptr<CSession>&, const short& msg_id, const std::string& msg_data)> FunCallBack;

class SChatLogic:public TSingleton<SChatLogic>
{
	friend class TSingleton<SChatLogic>;
public:
	~SChatLogic();

	/**
	 * ����Ϣ����Ͷ����Ϣ�����Ͷ��֮�������׸���Ϣ����֪ͨ�����߳̽�����Ϣ����
	 * ��������׸���Ϣ���򲻶ϵ�Ͷ�ݣ��������߳��򲻶ϵĴ���
	 * @param msg Ͷ�ݵ���Ϣ
	 */
	void PostMsgToQue(std::shared_ptr <CLogicNode> msg);
private:
	SChatLogic();
	void DealMsg();
	void RegisterCallBacks();
	void HelloWordCallBack(std::shared_ptr<CSession>&, const short& msg_id, const std::string& msg_data);
	void LoginHandler(std::shared_ptr<CSession>& session, const short& msg_id, const std::string& msg_data);
	std::thread WorkThread;
	std::queue<std::shared_ptr<CLogicNode>> MsgQue;
	std::mutex Mutex;
	std::condition_variable Consume;//��������
	bool bStop{false};
	std::map<unsigned short, FunCallBack> MsgId2Callback;
};

#endif // CHATLOGIC_H
