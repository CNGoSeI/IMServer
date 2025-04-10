#include "ChatLogic.h"

#include <iostream>
#include <json/reader.h>

#include "ChatDefine.h"
#include "Session.h"
#include "MsgNode.h"

SChatLogic::~SChatLogic()
{
}

SChatLogic::SChatLogic()
{
	RegisterCallBacks();
	WorkThread = std::thread(&SChatLogic::DealMsg, this);
}

void SChatLogic::PostMsgToQue(std::shared_ptr<CLogicNode> msg)
{
	std::unique_lock<std::mutex> lock(Mutex);
	MsgQue.push(msg);
	if(MsgQue.size() == 1)
	{
		lock.unlock();
		Consume.notify_one();
	}
}

void SChatLogic::DealMsg()
{
	/* ����ѭ�������Ķ��У�һ��ȡ�� */
	for (;;)
	{
		std::unique_lock<std::mutex> unique_lk(Mutex);
		//�ж϶���Ϊ�������������������ȴ������ͷ���
		while (MsgQue.empty() && !bStop)
		{
			Consume.wait(unique_lk);
		}

		//�ж��Ƿ�Ϊ�ر�״̬���������߼�ִ��������˳�ѭ��
		if (bStop)
		{
			while (!MsgQue.empty())
			{
				auto msg_node = MsgQue.front();
				std::cout << "���д�����Ϣ��ID��" << msg_node->RecvNode->MsgID << std::endl;

				auto call_back_iter = MsgId2Callback.find(msg_node->RecvNode->MsgID);
				if (call_back_iter == MsgId2Callback.end())
				{
					MsgQue.pop();
					continue;
				}
				call_back_iter->second(msg_node->Session, msg_node->RecvNode->MsgID,
				                       std::string(msg_node->RecvNode->Data, msg_node->RecvNode->CurLen));
				MsgQue.pop();
			}
			break;
		}

		//���û��ͣ������˵��������������
		auto msg_node = MsgQue.front();
		std::cout << "���Ķ����߳��յ�ID: " << msg_node->RecvNode->MsgID << std::endl;

		auto call_back_iter = MsgId2Callback.find(msg_node->RecvNode->MsgID);
		if (call_back_iter == MsgId2Callback.end())
		{
			MsgQue.pop();
			std::cout << "���Ķ����߳��յ�ID: [" << msg_node->RecvNode->MsgID << "] δ�ҵ�ע��Ļص�" << std::endl;
			continue;
		}

		call_back_iter->second(msg_node->Session, msg_node->RecvNode->MsgID,
		                       std::string(msg_node->RecvNode->Data, msg_node->RecvNode->CurLen));
		MsgQue.pop();
	}
}

void SChatLogic::RegisterCallBacks()
{
	MsgId2Callback.emplace(static_cast<short>(MSG_IDS::MSG_CHAT_LOGIN), [this](std::shared_ptr<CSession>& session, const short& msg_id, const std::string& msg_data)
		{
			this->LoginHandler(session, msg_id, msg_data);
		});
}

void SChatLogic::HelloWordCallBack(std::shared_ptr<CSession>& session, const short& msg_id, const std::string& msg_data)
{

}

void SChatLogic::LoginHandler(std::shared_ptr<CSession>& session, const short& msg_id, const std::string& msg_data) {
	Json::Reader reader;
	Json::Value root;
	reader.parse(msg_data, root);
	auto uid = root["uid"].asInt();
	std::cout << "UID: " << uid << "token��" << root["token"].asString() <<"��¼"<<std::endl;

	std::string return_str = root.toStyledString();
	session->Send(return_str, msg_id);

	//��״̬��������ȡtokenƥ���Ƿ�׼ȷ
	//auto rsp = SStatusGrpcClient::GetInstance()->Login(uid, root["token"].asString());
	//Json::Value  rtvalue;
	//Defer defer([this, &rtvalue, session]() {
	//	std::string return_str = rtvalue.toStyledString();
	//	session->Send(return_str, MSG_CHAT_LOGIN_RSP);
	//	});
	//
	//rtvalue["error"] = rsp.error();
	//if (rsp.error() != ErrorCodes::Success) {
	//	return;
	//}
	//
	////�ڴ��в�ѯ�û���Ϣ
	//auto find_iter = _users.find(uid);
	//std::shared_ptr<UserInfo> user_info = nullptr;
	//if (find_iter == _users.end()) {
	//	//��ѯ���ݿ�
	//	user_info = MysqlMgr::GetInstance()->GetUser(uid);
	//	if (user_info == nullptr) {
	//		rtvalue["error"] = ErrorCodes::UidInvalid;
	//		return;
	//	}
	//
	//	_users[uid] = user_info;
	//}
	//else {
	//	user_info = find_iter->second;
	//}
	//
	//rtvalue["uid"] = uid;
	//rtvalue["token"] = rsp.token();
	//rtvalue["name"] = user_info->name;
}
