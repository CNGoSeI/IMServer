#include "ChatServiceImpl.h"

CChatServiceImpl::CChatServiceImpl()
{
}

Status CChatServiceImpl::NotifyAddFriend(ServerContext* context, const AddFriendReq* request, AddFriendRsp* reply)
{
	return Status::OK;
}

bool CChatServiceImpl::GetBaseInfo(std::string base_key, int uid, std::shared_ptr<UserInfo>& userinfo)
{
	return true;
}
