#ifndef REDISMGR_H
#define REDISMGR_H

#include <string>
#include "SingletonTemplate.h"
#include <sw/redis++/redis++.h>

/* Redis操作类 */
class SRedisMgr:public TSingleton<SRedisMgr>
{
	friend class TSingleton<SRedisMgr>;
public:
	~SRedisMgr();
	bool Connect(const std::string& host, int port);
	bool Get(const std::string& key, std::string& value);//获取key对应的value
	bool Set(const std::string& key, const std::string& value);//设置key和value
	bool Auth(const std::string& password);//密码认证
	bool LPush(const std::string& key, const std::string& value);//左侧push
	bool LPop(const std::string& key, std::string& value);
	bool RPush(const std::string& key, const std::string& value);
	bool RPop(const std::string& key, std::string& value);
	bool HSet(const std::string& key, const std::string& hkey, const std::string& value);
	bool HSet(const char* key, const char* hkey, const char* hvalue, size_t hvaluelen);
	std::string HGet(const std::string& key, const std::string& hkey);
	bool Del(const std::string& key);
	bool ExistsKey(const std::string& key);
	void Close();
	bool ConnecterIsVaild()const;
private:

	redisContext* Connecter{nullptr};//链接者
	redisReply* Reply{ nullptr };//每次操作的返回值
};
#endif // REDISMGR_H
