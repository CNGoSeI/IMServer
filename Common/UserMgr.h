#ifndef COMMON_USERMGR_H
#define COMMON_USERMGR_H
#include <mutex>
#include <unordered_map>

#include "SingletonTemplate.h"

class CSession;

class SUserMgr:public TSingleton<SUserMgr>
{
	friend class TSingleton<SUserMgr>;
public:
	~SUserMgr();
	std::shared_ptr<CSession> GetSession(int uid);
	void SetUserSession(int uid, std::shared_ptr<CSession> session);
	void RmvUserSession(int uid);

private:
	SUserMgr()=default;
	std::mutex SessionMtx;
	std::unordered_map<int, std::shared_ptr<CSession>> Uid2Session;
};
#endif // COMMON_USERMGR_H
