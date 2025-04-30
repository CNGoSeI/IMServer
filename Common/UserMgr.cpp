#include "UserMgr.h"

#include <string>

SUserMgr::~SUserMgr()
{

}

std::shared_ptr<CSession> SUserMgr::GetSession(int uid)
{
	std::lock_guard<std::mutex> lock(SessionMtx);
	auto iter = Uid2Session.find(uid);
	if (iter == Uid2Session.end()) {
		return nullptr;
	}

	return iter->second;
}

void SUserMgr::SetUserSession(int uid, std::shared_ptr<CSession> session)
{
	std::lock_guard<std::mutex> lock(SessionMtx);
	Uid2Session[uid] = session;
}

void SUserMgr::RmvUserSession(int uid)
{
	auto uid_str = std::to_string(uid);
	//因为再次登录可能是其他服务器，所以会造成本服务器删除key，其他服务器注册key的情况
	// 有可能其他服务登录，本服删除key造成找不到key的情况
	//RedisMgr::GetInstance()->Del(USERIPPREFIX + uid_str);

	{
		std::lock_guard<std::mutex> lock(SessionMtx);
		Uid2Session.erase(uid);
	}

}
