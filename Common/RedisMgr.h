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
	~SRedisMgr() override;
	bool Connect(const std::string& host, int port);
	bool Get(const std::string& key, std::string& value);//获取key对应的value
	bool Set(const std::string& key, const std::string& value);//设置key和value
	bool Auth(const std::string& password);//密码认证
	bool LPush(const std::string& key, const std::string& value);//左侧push
	bool LPop(const std::string& key, std::string& value);//从左侧弹出首个
	bool RPush(const std::string& key, const std::string& value);//从右侧push
	bool RPop(const std::string& key, std::string& value);
	bool HSet(const std::string& key, const std::string& hkey, const std::string& value);//设置哈希键值对
	bool HSet(const char* key, const char* hkey, const char* hvalue, size_t hvaluelen);//char指针的输入形式调用Hset，方便二进制数据写入
	std::string HGet(const std::string& key, const std::string& hkey);
	bool Del(const std::string& key);//删除数据
	bool ExistsKey(const std::string& key);//查找key是否存在
	void Close();
	bool ConnecterIsVaild()const;
private:
	SRedisMgr() = default;
	redisContext* Connecter{nullptr};//链接者
};
#endif // REDISMGR_H
