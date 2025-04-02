#include <iostream>
#include <queue>
#include<sw/redis++/redis++.h>
#include "RedisMgr.h"

namespace RedisPool
{
	class RedisConPool {
	public:
		RedisConPool(size_t poolSize, const char* host, int port, const char* pwd)
			: poolSize_(poolSize), host_(host), port_(port), b_stop_(false), pwd_(pwd), counter_(0) {
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
					std::cout << "认证失败" << std::endl;
					//执行成功 释放redisCommand执行后返回的redisReply所占用的内存
					freeReplyObject(reply);
					continue;
				}

				//执行成功 释放redisCommand执行后返回的redisReply所占用的内存
				freeReplyObject(reply);
				std::cout << "认证成功" << std::endl;
				connections_.push(context);
			}

			check_thread_ = std::thread([this]() {
				while (!b_stop_) {
					counter_++;
					if (counter_ >= 60) {
						checkThread();
						counter_ = 0;
					}

					std::this_thread::sleep_for(std::chrono::seconds(1)); // 每隔 30 秒发送一次 PING 命令
				}
				});

		}

		~RedisConPool() {

		}

		void ClearConnections() {
			std::lock_guard<std::mutex> lock(mutex_);
			while (!connections_.empty()) {
				auto* context = connections_.front();
				redisFree(context);
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
			//如果停止则直接返回空指针
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
			check_thread_.join();
		}

	private:
		void checkThread() {
			std::lock_guard<std::mutex> lock(mutex_);
			if (b_stop_) {
				return;
			}
			auto pool_size = connections_.size();
			for (int i = 0; i < pool_size && !b_stop_; i++) {
				auto* context = connections_.front();
				connections_.pop();
				try {
					auto reply = (redisReply*)redisCommand(context, "PING");
					if (!reply) {
						std::cout << "redis ping failed" << std::endl;
						connections_.push(context);
						continue;
					}
					freeReplyObject(reply);
					connections_.push(context);
				}
				catch (std::exception& exp) {
					std::cout << "Error keeping connection alive: " << exp.what() << std::endl;
					redisFree(context);
					context = redisConnect(host_, port_);
					if (context == nullptr || context->err != 0) {
						if (context != nullptr) {
							redisFree(context);
						}
						continue;
					}

					auto reply = (redisReply*)redisCommand(context, "AUTH %s", pwd_);
					if (reply->type == REDIS_REPLY_ERROR) {
						std::cout << "认证失败" << std::endl;
						//执行成功 释放redisCommand执行后返回的redisReply所占用的内存
						freeReplyObject(reply);
						continue;
					}

					//执行成功 释放redisCommand执行后返回的redisReply所占用的内存
					freeReplyObject(reply);
					std::cout << "认证成功" << std::endl;
					connections_.push(context);
				}
			}
		}
		std::atomic<bool> b_stop_;
		size_t poolSize_;
		const char* host_;
		const char* pwd_;
		int port_;
		std::queue<redisContext*> connections_;
		std::mutex mutex_;
		std::condition_variable cond_;
		std::thread  check_thread_;
		int counter_;
	};
}

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