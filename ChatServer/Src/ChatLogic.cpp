#include "ChatLogic.h"

#include <iostream>
#include <json/reader.h>

#include "ChatDefine.h"
#include "ChatGrpcClient.h"
#include "ConfigMgr.h"
#include "const.h"
#include "message.pb.h"
#include "Session.h"
#include "MsgNode.h"
#include "MysqlDAO.h"
#include "PubFuncLib.h"
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
	/* 不断循环的消耗队列，一次取完 */
	for (;;)
	{
		std::unique_lock<std::mutex> unique_lk(Mutex);
		//判断队列为空则用条件变量阻塞等待，并释放锁
		while (MsgQue.empty() && !bStop)
		{
			Consume.wait(unique_lk);
		}

		//判断是否为关闭状态，把所有逻辑执行完后则退出循环
		if (bStop)
		{
			while (!MsgQue.empty())
			{
				auto msg_node = MsgQue.front();
				std::cout << "队列处理信息，ID：" << msg_node->RecvNode->MsgID << std::endl;

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

		//如果没有停服，且说明队列中有数据
		auto msg_node = MsgQue.front();
		std::cout << "消耗队列线程收到ID: " << msg_node->RecvNode->MsgID << std::endl;

		auto CallbackIter = MsgId2Callback.find(msg_node->RecvNode->MsgID);
		if (CallbackIter == MsgId2Callback.end())
		{
			MsgQue.pop();
			std::cout << "消耗队列线程收到ID: [" << msg_node->RecvNode->MsgID << "] 未找到注册的回调" << std::endl;
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

	MsgId2Callback.emplace(static_cast<short>(MSG_IDS::ID_SEARCH_USER_REQ), [this](std::shared_ptr<CSession>& session, const short& msg_id, const std::string& msg_data)
		{
			this->SearchInfoHandler(session, msg_id, msg_data);
		});

	MsgId2Callback.emplace(static_cast<short>(MSG_IDS::ID_ADD_FRIEND_REQ), [this](std::shared_ptr<CSession>& session, const short& msg_id, const std::string& msg_data)
		{
			this->AddFriendApplyHandler(session, msg_id, msg_data);
		});

	MsgId2Callback.emplace(static_cast<short>(MSG_IDS::ID_AUTH_FRIEND_REQ), [this](std::shared_ptr<CSession>& session, const short& msg_id, const std::string& msg_data)
		{
			this->AuthFriendApplyHandler(session, msg_id, msg_data);
		});
}

void SChatLogic::LoginHandler(std::shared_ptr<CSession>& session, const short& msg_id, const std::string& msg_data) {
	Json::Reader reader;
	Json::Value root;
	reader.parse(msg_data, root);
	auto uid = root["uid"].asInt();
	auto token = root["token"].asString();
	std::cout << "UID: " << uid << "token：" << token <<"登录"<<std::endl;

	//从状态服务器获取token匹配是否准确
	auto rsp = SStatusGrpcClient::GetInstance().Login(uid, root["token"].asString());
	Json::Value  rtvalue;

	auto exe = [&rsp,&rtvalue,this,uid, token, session]()
	{

			//从redis获取用户token是否正确
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

			//从数据库获取申请列表
			std::vector<FApplyInfo> apply_list;
			auto b_apply = SMysqlDao::GetInstance().GetApplyList(uid, apply_list);
			if (b_apply) {
				for (auto& apply : apply_list) {
					Json::Value obj;
					obj["name"] = apply.Name;
					obj["uid"] = apply.Uid;
					obj["icon"] = apply.Icon;
					obj["nick"] = apply.Nick;
					obj["sex"] = apply.Sex;
					obj["desc"] = apply.Desc;
					obj["status"] = apply.Status;
					rtvalue["apply_list"].append(obj);
				}
			}

			//获取好友列表
			std::vector<UserInfo> friend_list;
			bool b_friend_list = SMysqlDao::GetInstance().GetFriendList(uid, friend_list);
			for (auto& friend_ele : friend_list) {
				Json::Value obj;
				obj["name"] = friend_ele.name;
				obj["uid"] = friend_ele.uid;
				obj["icon"] = friend_ele.icon;
				obj["nick"] = friend_ele.nick;
				obj["sex"] = friend_ele.sex;
				obj["desc"] = friend_ele.desc;
				obj["back"] = friend_ele.back;
				rtvalue["friend_list"].append(obj);
			}

			auto server_name = Mgr::GetConfigHelper().get<std::string>("SelfServer.Name");
			//将登录数量增加
			auto rd_res = SRedisMgr::GetInstance().HGet(Prefix::LOGIN_COUNT, server_name);
			int count = 0;
			if (!rd_res.empty()) {
				count = std::stoi(rd_res);
			}

			count++;

			auto count_str = std::to_string(count);
			SRedisMgr::GetInstance().HSet(Prefix::LOGIN_COUNT, server_name, count_str);

			//session绑定用户uid
			session->SetUserId(uid);

			//为用户设置登录ip server的名字
			std::string  ipkey = Prefix::USERIPPREFIX + uid_str;
			SRedisMgr::GetInstance().Set(ipkey, server_name);

			//uid和session绑定管理,方便以后踢人操作
			SUserMgr::GetInstance().SetUserSession(uid, session);

	};

	exe();
	std::string return_str = rtvalue.toStyledString();
	session->Send(return_str, static_cast<short>(MSG_IDS::MSG_CHAT_LOGIN_RSP));
}

void SChatLogic::SearchInfoHandler(std::shared_ptr<CSession> session, const short& msg_id, const std::string& msg_data)
{
	Json::Reader reader;
	Json::Value root;
	reader.parse(msg_data, root);
	std::string uid_str = root["uid"].asString();
	std::cout << "查找的用户UID为  " << uid_str << endl;

	Json::Value rtvalue;

	auto exe = [&rtvalue, this,session,&uid_str]()
	{
		bool b_digit = PubFunc::IsPureDigit(uid_str);
		if (b_digit)
		{
			GetUserByUid(uid_str, rtvalue);
		}
		else
		{
			GetUserByName(uid_str, rtvalue);
		}
	};

	exe();
	std::string return_str = rtvalue.toStyledString();
	session->Send(return_str, static_cast<short>(MSG_IDS::ID_SEARCH_USER_RSP));

}

void SChatLogic::AddFriendApplyHandler(std::shared_ptr<CSession> session, const short& msg_id,
                                       const std::string& msg_data)
{
	Json::Reader reader;
	Json::Value root;
	reader.parse(msg_data, root);
	auto uid = root["uid"].asInt();
	auto applyname = root["applyname"].asString();
	auto bakname = root["bakname"].asString();
	auto touid = root["touid"].asInt();
	std::cout << "user login uid is  " << uid << " applyname  is "
		<< applyname << " bakname is " << bakname << " touid is " << touid << endl;

	Json::Value  rtvalue;
	rtvalue["error"] = ErrorCodes::Success;

	auto exe = [&rtvalue, this, session,uid,applyname, bakname, touid]()
	{
			//先更新数据库
			SMysqlDao::GetInstance().AddFriendApply(uid, touid);

			//查询redis 查找touid对应的server ip
			auto to_str = std::to_string(touid);
			auto to_ip_key = Prefix::USERIPPREFIX + to_str;
			std::string to_ip_value = "";
			bool b_ip = SRedisMgr::GetInstance().Get(to_ip_key, to_ip_value);
			if (!b_ip) {
				return;
			}

			auto& cfg =  Mgr::GetConfigHelper();
			auto self_name = cfg.get<std::string>("SelfServer.Name");

			//直接通知对方有申请消息
			if (to_ip_value == self_name) {
				auto session = SUserMgr::GetInstance().GetSession(touid);
				if (session) {
					//在内存中则直接发送通知对方
					Json::Value  notify;
					notify["error"] = ErrorCodes::Success;
					notify["applyuid"] = uid;
					notify["name"] = applyname;
					notify["desc"] = "";
					std::string return_str = notify.toStyledString();
					session->Send(return_str, static_cast<short>(MSG_IDS::ID_NOTIFY_ADD_FRIEND_REQ));
				}

				return;
			}

			std::string base_key = Prefix::USER_BASE_INFO + std::to_string(uid);
			auto apply_info = std::make_shared<UserInfo>();
			bool b_info = GetBaseInfo(base_key, uid, apply_info);

			message::AddFriendReq add_req;
			add_req.set_applyuid(uid);
			add_req.set_touid(touid);
			add_req.set_name(applyname);
			add_req.set_desc("");
			if (b_info) {
				add_req.set_icon(apply_info->icon);
				add_req.set_sex(apply_info->sex);
				add_req.set_nick(apply_info->nick);
			}

			//发送通知
			CChatGrpcClient::GetInstance().NotifyAddFriend(to_ip_value, add_req);
		};

	exe();
	std::string return_str = rtvalue.toStyledString();
	session->Send(return_str, static_cast<short>(MSG_IDS::ID_ADD_FRIEND_RSP));
}

void SChatLogic::AuthFriendApplyHandler(std::shared_ptr<CSession> session, const short msg_id,
	const std::string& msg_data)
{

	Json::Value  rtvalue;
	auto exe = [&rtvalue, &msg_data, this]()
	{
			Json::Reader reader;
			Json::Value root;
			reader.parse(msg_data, root);

			auto uid = root["fromuid"].asInt();
			auto touid = root["touid"].asInt();
			auto back_name = root["back"].asString();
			std::cout << "from " << uid << " auth friend to " << touid << std::endl;

			
			rtvalue["error"] = ErrorCodes::Success;
			auto user_info = std::make_shared<UserInfo>();

			std::string base_key = Prefix::USER_BASE_INFO + std::to_string(touid);
			bool b_info = GetBaseInfo(base_key, touid, user_info);
			if (b_info) {
				rtvalue["name"] = user_info->name;
				rtvalue["nick"] = user_info->nick;
				rtvalue["icon"] = user_info->icon;
				rtvalue["sex"] = user_info->sex;
				rtvalue["uid"] = touid;
			}
			else {
				rtvalue["error"] = ErrorCodes::UidInvalid;
			}

			//先更新数据库
			SMysqlDao::GetInstance().AuthFriendApply(uid, touid);

			//更新数据库添加好友
			SMysqlDao::GetInstance().AddFriend(uid, touid, back_name);

			//查询redis 查找touid对应的server ip
			auto to_str = std::to_string(touid);
			auto to_ip_key = Prefix::USERIPPREFIX + to_str;
			std::string to_ip_value = "";
			bool b_ip = SRedisMgr::GetInstance().Get(to_ip_key, to_ip_value);
			if (!b_ip) {
				return;
			}

			auto& cfg = Mgr::GetConfigHelper();
			auto self_name = cfg.get<std::string>("SelfServer.Name");
			//直接通知对方有认证通过消息
			if (to_ip_value == self_name) {
				auto session = SUserMgr::GetInstance().GetSession(touid);
				if (session) {
					//在内存中则直接发送通知对方
					Json::Value  notify;
					notify["error"] = ErrorCodes::Success;
					notify["fromuid"] = uid;
					notify["touid"] = touid;
					std::string base_key = Prefix::USER_BASE_INFO + std::to_string(uid);
					auto user_info = std::make_shared<UserInfo>();
					bool b_info = GetBaseInfo(base_key, uid, user_info);
					if (b_info) {
						notify["name"] = user_info->name;
						notify["nick"] = user_info->nick;
						notify["icon"] = user_info->icon;
						notify["sex"] = user_info->sex;
					}
					else {
						notify["error"] = ErrorCodes::UidInvalid;
					}


					std::string return_str = notify.toStyledString();
					session->Send(return_str, static_cast<short>(MSG_IDS::ID_NOTIFY_AUTH_FRIEND_REQ));
				}

				return;
			}

			message::AuthFriendReq auth_req;
			auth_req.set_fromuid(uid);
			auth_req.set_touid(touid);

			//发送通知
			CChatGrpcClient::GetInstance().NotifyAuthFriend(to_ip_value, auth_req);
	};
	exe();
	std::string return_str = rtvalue.toStyledString();
	session->Send(return_str, static_cast<short>(MSG_IDS::ID_AUTH_FRIEND_RSP));
}

bool SChatLogic::GetBaseInfo(std::string base_key, int uid, std::shared_ptr<UserInfo>& userinfo)
{
	//优先查redis中查询用户信息
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
		//redis中没有则查询mysql
		//查询数据库
		std::shared_ptr<UserInfo> user_info = nullptr;
		user_info = SMysqlDao::GetInstance().GetUser(uid);
		if (user_info == nullptr) {
			return false;
		}

		userinfo = user_info;

		//将数据库内容写入redis缓存
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


void SChatLogic::GetUserByUid(const std::string& uid_str, Json::Value& rtvalue)
{
	rtvalue["error"] = ErrorCodes::Success;

	std::string base_key = Prefix::USER_BASE_INFO + uid_str;

	//优先查redis中查询用户信息
	std::string info_str = "";
	bool b_base = SRedisMgr::GetInstance().Get(base_key, info_str);
	if (b_base) {
		Json::Reader reader;
		Json::Value root;
		reader.parse(info_str, root);
		auto uid = root["uid"].asInt();
		auto name = root["name"].asString();
		auto pwd = root["pwd"].asString();
		auto email = root["email"].asString();
		auto nick = root["nick"].asString();
		auto desc = root["desc"].asString();
		auto sex = root["sex"].asInt();
		auto icon = root["icon"].asString();
		std::cout << "查询到 UID： " << uid << " 名称： "
			<< name << " 密码： " << pwd << " 邮箱： " << email << " 图标：" << icon << endl;

		rtvalue["uid"] = uid;
		rtvalue["pwd"] = pwd;
		rtvalue["name"] = name;
		rtvalue["email"] = email;
		rtvalue["nick"] = nick;
		rtvalue["desc"] = desc;
		rtvalue["sex"] = sex;
		rtvalue["icon"] = icon;
		return;
	}

	auto uid = std::stoi(uid_str);
	//redis中没有则查询mysql
	//查询数据库
	std::shared_ptr<UserInfo> user_info = nullptr;
	user_info = SMysqlDao::GetInstance().GetUser(uid);
	if (user_info == nullptr) {
		rtvalue["error"] = ErrorCodes::UidInvalid;
		return;
	}

	//将数据库内容写入redis缓存,方便下次直接从redis读取
	Json::Value redis_root;
	redis_root["uid"] = user_info->uid;
	redis_root["pwd"] = user_info->pwd;
	redis_root["name"] = user_info->name;
	redis_root["email"] = user_info->email;
	redis_root["nick"] = user_info->nick;
	redis_root["desc"] = user_info->desc;
	redis_root["sex"] = user_info->sex;
	redis_root["icon"] = user_info->icon;

	SRedisMgr::GetInstance().Set(base_key, redis_root.toStyledString());

	//返回数据
	rtvalue["uid"] = user_info->uid;
	rtvalue["pwd"] = user_info->pwd;
	rtvalue["name"] = user_info->name;
	rtvalue["email"] = user_info->email;
	rtvalue["nick"] = user_info->nick;
	rtvalue["desc"] = user_info->desc;
	rtvalue["sex"] = user_info->sex;
	rtvalue["icon"] = user_info->icon;
}

void SChatLogic::GetUserByName(const std::string& name, Json::Value& rtvalue)
{
	rtvalue["error"] = ErrorCodes::Success;

	std::string base_key = Prefix::NAME_INFO + name;

	//优先查redis中查询用户信息
	std::string info_str = "";
	bool b_base = SRedisMgr::GetInstance().Get(base_key, info_str);
	if (b_base) {
		Json::Reader reader;
		Json::Value root;
		reader.parse(info_str, root);
		auto uid = root["uid"].asInt();
		auto name = root["name"].asString();
		auto pwd = root["pwd"].asString();
		auto email = root["email"].asString();
		auto nick = root["nick"].asString();
		auto desc = root["desc"].asString();
		auto sex = root["sex"].asInt();
		std::cout << "查询到 UID： " << uid << " 名称： "
			<< name << " 密码： " << pwd << " 邮箱： " << email << endl;

		rtvalue["uid"] = uid;
		rtvalue["pwd"] = pwd;
		rtvalue["name"] = name;
		rtvalue["email"] = email;
		rtvalue["nick"] = nick;
		rtvalue["desc"] = desc;
		rtvalue["sex"] = sex;
		return;
	}

	//redis中没有则查询mysql
	//查询数据库
	std::shared_ptr<UserInfo> user_info = nullptr;
	user_info = SMysqlDao::GetInstance().GetUser(name);
	if (user_info == nullptr) {
		rtvalue["error"] = ErrorCodes::UidInvalid;
		return;
	}

	//将数据库内容写入redis缓存
	Json::Value redis_root;
	redis_root["uid"] = user_info->uid;
	redis_root["pwd"] = user_info->pwd;
	redis_root["name"] = user_info->name;
	redis_root["email"] = user_info->email;
	redis_root["nick"] = user_info->nick;
	redis_root["desc"] = user_info->desc;
	redis_root["sex"] = user_info->sex;

	SRedisMgr::GetInstance().Set(base_key, redis_root.toStyledString());

	//返回数据
	rtvalue["uid"] = user_info->uid;
	rtvalue["pwd"] = user_info->pwd;
	rtvalue["name"] = user_info->name;
	rtvalue["email"] = user_info->email;
	rtvalue["nick"] = user_info->nick;
	rtvalue["desc"] = user_info->desc;
	rtvalue["sex"] = user_info->sex;
}