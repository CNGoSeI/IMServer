#include<sw/redis++/redis++.h>
#include "RedisMgr.h"

void TestRedisMgr() {
	assert(SRedisMgr::GetInstance().Connect("127.0.0.1", 6379));
	assert(SRedisMgr::GetInstance().Auth("123456"));
	assert(SRedisMgr::GetInstance().Set("blogwebsite", "llfc.club"));
	std::string value = "";
	assert(SRedisMgr::GetInstance().Get("blogwebsite", value));
	assert(SRedisMgr::GetInstance().Get("nonekey", value) == false);
	assert(SRedisMgr::GetInstance().HSet("bloginfo", "blogwebsite", "llfc.club"));
	assert(SRedisMgr::GetInstance().HGet("bloginfo", "blogwebsite") != "");
	assert(SRedisMgr::GetInstance().ExistsKey("bloginfo"));
	assert(SRedisMgr::GetInstance().Del("bloginfo"));
	assert(SRedisMgr::GetInstance().Del("bloginfo"));
	assert(SRedisMgr::GetInstance().ExistsKey("bloginfo") == false);
	assert(SRedisMgr::GetInstance().LPush("lpushkey1", "lpushvalue1"));
	assert(SRedisMgr::GetInstance().LPush("lpushkey1", "lpushvalue2"));
	assert(SRedisMgr::GetInstance().LPush("lpushkey1", "lpushvalue3"));
	assert(SRedisMgr::GetInstance().RPop("lpushkey1", value));
	assert(SRedisMgr::GetInstance().RPop("lpushkey1", value));
	assert(SRedisMgr::GetInstance().LPop("lpushkey1", value));
	assert(SRedisMgr::GetInstance().LPop("lpushkey2", value) == false);
	//SRedisMgr::GetInstance().Close(); 由析构调用
}

int main()
{
	TestRedisMgr();
	system("pause");
	return 0;
}