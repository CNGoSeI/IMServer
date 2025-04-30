#include "ChatLogic.h"

#include <iostream>
#include <json/reader.h>

#include "ChatDefine.h"
#include "ConfigMgr.h"
#include "const.h"
#include "Session.h"
#include "MsgNode.h"
#include "MysqlDAO.h"
#include "RedisMgr.h"
#include "StatusGrpcClient.h"
#include "UserMgr.h"

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

		auto CallbackIter = MsgId2Callback.find(msg_node->RecvNode->MsgID);
		if (CallbackIter == MsgId2Callback.end())
		{
			MsgQue.pop();
			std::cout << "���Ķ����߳��յ�ID: [" << msg_node->RecvNode->MsgID << "] δ�ҵ�ע��Ļص�" << std::endl;
			continue;
		}

		CallbackIter->second(msg_node->Session, msg_node->RecvNode->MsgID,
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

void SChatLogic::LoginHandler(std::shared_ptr<CSession>& session, const short& msg_id, const std::string& msg_data) {
	Json::Reader reader;
	Json::Value root;
	reader.parse(msg_data, root);
	auto uid = root["uid"].asInt();
	auto token = root["token"].asString();
	std::cout << "UID: " << uid << "token��" << token <<"��¼"<<std::endl;

	//��״̬��������ȡtokenƥ���Ƿ�׼ȷ
	auto rsp = SStatusGrpcClient::GetInstance().Login(uid, root["token"].asString());
	Json::Value  rtvalue;

	auto exe = [&rsp,&rtvalue,this,uid, token, session]()
	{

			//��redis��ȡ�û�token�Ƿ���ȷ
			std::string uid_str = std::to_string(uid);
			std::string token_key = Prefix::USERTOKENPREFIX + uid_str;
			std::string token_value = "";

			bool success = SRedisMgr::GetInstance().Get(token_key, token_value);
			if (!success) {
				rtvalue["error"] = ErrorCodes::UidInvalid;
				return;
			}

			if (token_value != token) {
				rtvalue["error"] = ErrorCodes::TokenInvalid;
				return;
			}

			rtvalue["error"] = ErrorCodes::Success;

			std::string base_key = Prefix::USER_BASE_INFO + uid_str;
			auto user_info = std::make_shared<UserInfo>();
			bool b_base = GetBaseInfo(base_key, uid, user_info);
			if (!b_base) {
				rtvalue["error"] = ErrorCodes::UidInvalid;
				return;
			}

			rtvalue["uid"] = uid;
			rtvalue["token"] = rsp.token();
			rtvalue["name"] = user_info->name;
			rtvalue["email"] = user_info->email;
			rtvalue["nick"] = user_info->nick;
			rtvalue["desc"] = user_info->desc;
			rtvalue["sex"] = user_info->sex;
			rtvalue["icon"] = user_info->icon;

			auto server_name = Mgr::GetConfigHelper().get<std::string>("SelfServer.Name");
			//����¼��������
			auto rd_res = SRedisMgr::GetInstance().HGet(Prefix::LOGIN_COUNT, server_name);
			int count = 0;
			if (!rd_res.empty()) {
				count = std::stoi(rd_res);
			}

			count++;

			auto count_str = std::to_string(count);
			SRedisMgr::GetInstance().HSet(Prefix::LOGIN_COUNT, server_name, count_str);

			//session���û�uid
			session->SetUserId(uid);

			//Ϊ�û����õ�¼ip server������
			std::string  ipkey = Prefix::USERIPPREFIX + uid_str;
			SRedisMgr::GetInstance().Set(ipkey, server_name);

			//uid��session�󶨹���,�����Ժ����˲���
			SUserMgr::GetInstance().SetUserSession(uid, session);

	};

	exe();
	std::string return_str = rtvalue.toStyledString();
	session->Send(return_str, static_cast<short>(MSG_IDS::MSG_CHAT_LOGIN_RSP));
}

bool SChatLogic::GetBaseInfo(std::string base_key, int uid, std::shared_ptr<UserInfo>& userinfo)
{
	//���Ȳ�redis�в�ѯ�û���Ϣ
	std::string info_str = "";
	bool b_base = SRedisMgr::GetInstance().Get(base_key, info_str);
	if (b_base) {
		Json::Reader reader;
		Json::Value root;
		reader.parse(info_str, root);
		userinfo->uid = root["uid"].asInt();
		userinfo->name = root["name"].asString();
		userinfo->pwd = root["pwd"].asString();
		userinfo->email = root["email"].asString();
		userinfo->nick = root["nick"].asString();
		userinfo->desc = root["desc"].asString();
		userinfo->sex = root["sex"].asInt();
		std::cout << "user login uid is  " << userinfo->uid << " name  is "
			<< userinfo->name << " pwd is " << userinfo->pwd << " email is " << userinfo->email << endl;
	}
	else {
		//redis��û�����ѯmysql
		//��ѯ���ݿ�
		std::shared_ptr<UserInfo> user_info = nullptr;
		user_info = SMysqlDao::GetInstance().GetUser(uid);
		if (user_info == nullptr) {
			return false;
		}

		userinfo = user_info;

		//�����ݿ�����д��redis����
		Json::Value redis_root;
		redis_root["uid"] = uid;
		redis_root["pwd"] = userinfo->pwd;
		redis_root["name"] = userinfo->name;
		redis_root["email"] = userinfo->email;
		redis_root["nick"] = userinfo->nick;
		redis_root["desc"] = userinfo->desc;
		redis_root["sex"] = userinfo->sex;

		SRedisMgr::GetInstance().Set(base_key, redis_root.toStyledString());
	}
	return true;
}