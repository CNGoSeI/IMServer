#ifndef REDISMGR_H
#define REDISMGR_H

#include <string>
#include "SingletonTemplate.h"
#include <sw/redis++/redis++.h>
#include "ThreadWorkerTemplate.h"

namespace Deleter
{
	/* redisReply释放回收器 */
	struct RedisReplyDeleter {
		void operator()(redisReply*& reply) const noexcept {
			if (reply)
				freeReplyObject(reply);
			reply = nullptr;
		}
	};

	/* redisContext释放回收器 */
	struct RedisContextDeleter {
		void operator()(redisContext*& Context) const noexcept {
			if (Context)
				redisFree(Context);
			Context = nullptr;
		}
	};
}

using RedisReply_Unique = std::unique_ptr<redisReply, Deleter::RedisReplyDeleter>;//使用该方式方便操作之后自动回收redisReply
using RedisContext_Unique = std::unique_ptr<redisContext, Deleter::RedisContextDeleter>;

using RedisConnectPool_Unique = TThreadWoker<RedisContext_Unique>;//线程模板类，模板类成员为更改析构器的redisContext唯一指针

/* Redis操作类 */
class SRedisMgr:public TSingleton<SRedisMgr>
{
	friend class TSingleton<SRedisMgr>;
public:
	~SRedisMgr() override;
	bool Get(const std::string& key, std::string& value);//获取key对应的value
	bool Set(const std::string& key, const std::string& value);//设置key和value
	bool LPush(const std::string& key, const std::string& value);//左侧push
	bool LPop(const std::string& key, std::string& value);//从左侧弹出首个
	bool RPush(const std::string& key, const std::string& value);//从右侧push
	bool RPop(const std::string& key, std::string& value);
	bool HSet(const std::string& key, const std::string& hkey, const std::string& value);//设置哈希键值对
	bool HSet(const char* key, const char* hkey, const char* hvalue, size_t hvaluelen);//char指针的输入形式调用Hset，方便二进制数据写入
	std::string HGet(const std::string& key, const std::string& hkey);
	bool Del(const std::string& key);//删除数据
	bool ExistsKey(const std::string& key);//查找key是否存在
	bool ConnecterIsVaild(redisContext* Connecter)const;
private:
	SRedisMgr();

	std::unique_ptr<RedisConnectPool_Unique> ConnectPool;
};
#endif // REDISMGR_H
