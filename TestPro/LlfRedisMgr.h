#ifndef LLFREDISMGR_H
#define LLFREDISMGR_H
#include <sw/redis++/redis++.h>
#include "SingletonTemplate.h"
#include <string>
#include <atomic>
#include <queue>
#include <iostream>

class RedisConPool {
public:
	RedisConPool(size_t poolSize, const char* host, int port, const char* pwd)
		: poolSize_(poolSize), host_(host), port_(port), b_stop_(false) {
		for (size_t i = 0; i < poolSize_; ++i) {
			auto* context = redisConnect(host, port);
			if (context == nullptr || context->err != 0) {
				if (context != nullptr) {
					redisFree(context);
				}
				continue;
			}

			auto reply = (redisReply*)redisCommand(context, "AUTH %s", pwd);
			if (reply->type == REDIS_REPLY_ERROR) {
				std::cout << "��֤ʧ��" << std::endl;
				//ִ�гɹ� �ͷ�redisCommandִ�к󷵻ص�redisReply��ռ�õ��ڴ�
				freeReplyObject(reply);
				continue;
			}

			//ִ�гɹ� �ͷ�redisCommandִ�к󷵻ص�redisReply��ռ�õ��ڴ�
			freeReplyObject(reply);
			std::cout << "��֤�ɹ�" << std::endl;
			connections_.push(context);
		}

	}

	~RedisConPool() {
		std::lock_guard<std::mutex> lock(mutex_);
		while (!connections_.empty()) {
			connections_.pop();
		}
	}

	redisContext* getConnection() {
		std::unique_lock<std::mutex> lock(mutex_);
		cond_.wait(lock, [this] {
			if (b_stop_) {
				return true;
			}
			return !connections_.empty();
			});
		//���ֹͣ��ֱ�ӷ��ؿ�ָ��
		if (b_stop_) {
			return  nullptr;
		}
		auto* context = connections_.front();
		connections_.pop();
		return context;
	}

	void returnConnection(redisContext* context) {
		std::lock_guard<std::mutex> lock(mutex_);
		if (b_stop_) {
			return;
		}
		connections_.push(context);
		cond_.notify_one();
	}

	void Close() {
		b_stop_ = true;
		cond_.notify_all();
	}

private:
	std::atomic<bool> b_stop_;
	size_t poolSize_;
	const char* host_;
	int port_;
	std::queue<redisContext*> connections_;
	std::mutex mutex_;
	std::condition_variable cond_;
};

class LlfRedisMgr : public TSingleton<LlfRedisMgr>
{
	friend class TSingleton<LlfRedisMgr>;
public:
	~LlfRedisMgr();
	bool Get(const std::string& key, std::string& value);
	bool Set(const std::string& key, const std::string& value);
	bool LPush(const std::string& key, const std::string& value);
	bool LPop(const std::string& key, std::string& value);
	bool RPush(const std::string& key, const std::string& value);
	bool RPop(const std::string& key, std::string& value);
	bool HSet(const std::string& key, const std::string& hkey, const std::string& value);
	bool HSet(const char* key, const char* hkey, const char* hvalue, size_t hvaluelen);
	std::string HGet(const std::string& key, const std::string& hkey);
	bool Del(const std::string& key);
	bool ExistsKey(const std::string& key);
	void Close() {
		_con_pool->Close();
	}
private:
	LlfRedisMgr();
	std::unique_ptr<RedisConPool>  _con_pool;
};

#endif // LLFREDISMGR_H
