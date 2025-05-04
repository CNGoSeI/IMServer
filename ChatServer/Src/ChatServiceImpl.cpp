#include "ChatServiceImpl.h"

#include <json/value.h>

#include "ChatDefine.h"
#include "const.h"
#include "MysqlDAO.h"
#include "Session.h"
#include "UserMgr.h"

CChatServiceImpl::CChatServiceImpl()
{
}

Status CChatServiceImpl::NotifyAddFriend(ServerContext* context, const AddFriendReq* request, AddFriendRsp* reply)
{
	//查找用户是否在本服务器
	auto touid = request->touid();
	auto session = SUserMgr::GetInstance().GetSession(touid);
	Json::Value  rtvalue;

	auto exe = [&session,this, &request,&rtvalue]()
	{
		//用户不在内存中则直接返回
		if (session == nullptr)
		{
			return;
		}

		//在内存中则直接发送通知对方
		rtvalue["error"] = ErrorCodes::Success;
		rtvalue["applyuid"] = request->applyuid();
		rtvalue["name"] = request->name();
		rtvalue["desc"] = request->desc();
		rtvalue["icon"] = request->icon();
		rtvalue["sex"] = request->sex();
		rtvalue["nick"] = request->nick();

		std::string return_str = rtvalue.toStyledString();

		session->Send(return_str, static_cast<short>(MSG_IDS::ID_NOTIFY_ADD_FRIEND_REQ));
	};

	exe();

	reply->set_error(ErrorCodes::Success);
	reply->set_applyuid(request->applyuid());
	reply->set_touid(request->touid());
	return Status::OK;
}

Status CChatServiceImpl::NotifyAuthFriend(ServerContext* context, const AuthFriendReq* request, AuthFriendRsp* response)
{
	//查找用户是否在本服务器
	auto touid = request->touid();
	auto fromuid = request->fromuid();
	auto session = SUserMgr::GetInstance().GetSession(touid);

	auto exe = [&session, &request,this, &fromuid]()
	{
		//用户不在内存中则直接返回
		if (session == nullptr)
		{
			return;
		}

		//在内存中则直接发送通知对方
		Json::Value rtvalue;
		rtvalue["error"] = ErrorCodes::Success;
		rtvalue["fromuid"] = request->fromuid();
		rtvalue["touid"] = request->touid();

		std::string base_key = Prefix::USER_BASE_INFO + std::to_string(fromuid);
		auto user_info = std::make_shared<UserInfo>();
		bool b_info = GetBaseInfo(base_key, fromuid, user_info);
		if (b_info)
		{
			rtvalue["name"] = user_info->name;
			rtvalue["nick"] = user_info->nick;
			rtvalue["icon"] = user_info->icon;
			rtvalue["sex"] = user_info->sex;
		}
		else
		{
			rtvalue["error"] = ErrorCodes::UidInvalid;
		}

		std::string return_str = rtvalue.toStyledString();

		session->Send(return_str, static_cast<short>(MSG_IDS::ID_NOTIFY_AUTH_FRIEND_REQ));
	};

	exe();
	response->set_error(ErrorCodes::Success);
	response->set_fromuid(request->fromuid());
	response->set_touid(request->touid());
	return Status::OK;
}

bool CChatServiceImpl::GetBaseInfo(std::string base_key, int uid, std::shared_ptr<UserInfo>& userinfo)
{
	return true;
}
