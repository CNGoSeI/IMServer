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
	 * ����Ϣ����Ͷ����Ϣ�����Ͷ��֮�������׸���Ϣ����֪ͨ�����߳̽�����Ϣ����
	 * ��������׸���Ϣ���򲻶ϵ�Ͷ�ݣ��������߳��򲻶ϵĴ���
	 * @param msg Ͷ�ݵ���Ϣ
	 */
	void PostMsgToQue(std::shared_ptr <CLogicNode> msg);

/* ---����ص��ָ���--- */
private:
	void LoginHandler(std::shared_ptr<CSession>& session, const short& msg_id, const std::string& msg_data);//ע��Ļص�
	void SearchInfoHandler(std::shared_ptr<CSession> session, const short& msg_id, const std::string& msg_data);//�����û��Ļص�
	void AddFriendApplyHandler(std::shared_ptr<CSession> session, const short& msg_id, const std::string& msg_data);
	void AuthFriendApplyHandler(std::shared_ptr<CSession> session, const short msg_id, const std::string& msg_data);
	void DealChatTextMsg(std::shared_ptr<CSession> session, const short msg_id, const std::string& msg_data);
/* ---����ص��ָ���--- */

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
	std::condition_variable Consume;//��������
	bool bStop{false};
	std::map<unsigned short, FunCallBack> MsgId2Callback;
};

#endif // CHATLOGIC_H
