#include "RedisMgr.h"
#include <iostream>
#include <string>
#include "ConfigMgr.h"

SRedisMgr::~SRedisMgr()
{
	if(ConnectPool)
	{
		ConnectPool->Close();
	}
	ConnectPool = nullptr;
}

bool SRedisMgr::Get(const std::string& key, std::string& value)
{
	auto Connect = ConnectPool->GetWorker();
	auto Connecter = Connect.get();

	//Lambda是为了在执行命令之后有个回收连接池成员的调用 未用守卫
	auto res = [Connecter,this, key, &value]()
	{
		if (!ConnecterIsVaild(Connecter))return false;

		const RedisReply_Unique Reply(static_cast<redisReply*>(redisCommand(Connecter, "GET %s", key.c_str())));

		if (Reply == nullptr)
		{
			std::cout << "[ GET  " << key << " ] 失败" << std::endl;
			return false;
		}
		//返回的不是字符串，有问题
		if (Reply->type != REDIS_REPLY_STRING)
		{
			std::cout << "[ GET  " << key << " ] 失败，返回值类型非字符串" << std::endl;
			return false;
		}

		value = Reply->str;
		std::cout << "成功执行命令 [ GET " << key << "  ]" << std::endl;
		return true;
	}();

	ConnectPool->ReturnWorker(std::move(Connect));
	return res;
}

bool SRedisMgr::Set(const std::string& key, const std::string& value)
{
	auto Connect = ConnectPool->GetWorker();
	auto Connecter = Connect.get();

	auto res = [Connecter, this, key, value]()
	{
		if (!ConnecterIsVaild(Connecter))return false;

		//执行redis命令行
		const RedisReply_Unique Reply(
			static_cast<redisReply*>(redisCommand(Connecter, "SET %s %s", key.c_str(), value.c_str())));

		//如果返回NULL则说明执行失败
		if (Reply == nullptr)
		{
			std::cout << "执行操作 [ SET " << key << "  " << value << " ] 失败 ! " << std::endl;
			return false;
		}
		/**
		 * REDIS_REPLY_STATUS 表示返回的非二进制安全的状态信息（如 +OK\r\n 或 +PONG\r\n 等简单字符串）
		 * 返回 类型为状态信息 并且 信息为OK 的情况下才不算失败
		 */
		if (!(Reply->type == REDIS_REPLY_STATUS && (strcmp(Reply->str, "OK") == 0 || strcmp(Reply->str, "ok") == 0)))
		{
			std::cout << "执行操作 [ SET " << key << "  " << value << " ] 失败 ! " << std::endl;
			return false;
		}

		std::cout << "执行操作 [ SET " << key << "  " << value << " ] 成功 ! " << std::endl;
		return true;
	}();

	ConnectPool->ReturnWorker(std::move(Connect));
	return res;
}

bool SRedisMgr::LPush(const std::string& key, const std::string& value)
{
	auto Connect = ConnectPool->GetWorker();
	auto Connecter = Connect.get();

	auto res = [Connecter, this, key, value]()
	{
		if (!ConnecterIsVaild(Connecter))return false;

		/**
		* LPush 作用是将一个或多个元素插入到列表的头部​（左侧)
		*	> lpush name sei		#向 name 左边插入 sei
		*	 1						#返回当前列表长度为1
		*	> lpush name Go			#向 name 左边插入 Go
		*	2						#返回当前列表长度为2
		*	> LRANGE name 0 1		#显示 name列表第0到1个成员
		*	Go						#第0个
		*	sei						#第1个
		*/
		const RedisReply_Unique Reply(
			static_cast<redisReply*>(redisCommand(Connecter, "LPUSH %s %s", key.c_str(), value.c_str())));
		if (nullptr == Reply)
		{
			std::cout << "执行操作 [ LPUSH " << key << "  " << value << " ] 错误 ! " << std::endl;
			return false;
		}

		/* LPUSH 操作返回类型会是数字类型 可在命令窗口尝试验证 */
		if (Reply->type != REDIS_REPLY_INTEGER || Reply->integer <= 0)
		{
			std::cout << "执行操作 [ LPUSH " << key << "  " << value << " ] 错误 ! " << std::endl;
			return false;
		}

		std::cout << "执行操作 [ LPUSH " << key << "  " << value << " ] 成功 ! " << std::endl;
		return true;
	}();

	ConnectPool->ReturnWorker(std::move(Connect));
	return res;
}

bool SRedisMgr::LPop(const std::string& key, std::string& value)
{
	auto Connect = ConnectPool->GetWorker();
	auto Connecter = Connect.get();

	auto res = [Connecter, this, key, &value]()
	{
		if (!ConnecterIsVaild(Connecter))return false;

		const RedisReply_Unique Reply(static_cast<redisReply*>(redisCommand(Connecter, "LPOP %s ", key.c_str())));

		if (Reply == nullptr || Reply->type == REDIS_REPLY_NIL)
		{
			std::cout << "执行操作 [ LPOP " << key << " ] 错误 ! " << std::endl;
			return false;
		}
		value = Reply->str;
		std::cout << "执行操作 [ LPOP " << key << " ] 成功 ! " << std::endl;
		return true;
	}();

	ConnectPool->ReturnWorker(std::move(Connect));
	return res;
}

bool SRedisMgr::RPush(const std::string& key, const std::string& value)
{
	auto Connect = ConnectPool->GetWorker();
	auto Connecter = Connect.get();

	auto res = [Connecter, this, key, value]()
	{
		if (!ConnecterIsVaild(Connecter))return false;
		const RedisReply_Unique Reply(
			static_cast<redisReply*>(redisCommand(Connecter, "RPUSH %s %s", key.c_str(), value.c_str())));
		if (nullptr == Reply)
		{
			std::cout << "执行操作 [ RPUSH " << key << "  " << value << " ] 错误 ! " << std::endl;
			return false;
		}

		if (Reply->type != REDIS_REPLY_INTEGER || Reply->integer <= 0)
		{
			std::cout << "执行操作 [ RPUSH " << key << "  " << value << " ] 错误 ! " << std::endl;
			return false;
		}

		std::cout << "执行操作 [ RPUSH " << key << "  " << value << " ] 成功 ! " << std::endl;
		return true;
	}();

	ConnectPool->ReturnWorker(std::move(Connect));
	return res;
}

bool SRedisMgr::RPop(const std::string& key, std::string& value)
{
	auto Connect = ConnectPool->GetWorker();
	auto Connecter = Connect.get();

	auto res = [Connecter, this, key, &value]()
	{
		if (!ConnecterIsVaild(Connecter))return false;

		const RedisReply_Unique Reply(static_cast<redisReply*>(redisCommand(Connecter, "RPOP %s ", key.c_str())));
		if (Reply == nullptr || Reply->type == REDIS_REPLY_NIL)
		{
			std::cout << "执行操作 [ RPOP " << key << " ] 错误 ! " << std::endl;
			return false;
		}
		value = Reply->str;
		std::cout << "执行操作 [ RPOP " << key << " ] 成功 ! " << std::endl;
		return true;
	}();

	ConnectPool->ReturnWorker(std::move(Connect));
	return res;
}

bool SRedisMgr::HSet(const std::string& key, const std::string& hkey, const std::string& value)
{
	auto Connect = ConnectPool->GetWorker();
	auto Connecter = Connect.get();

	auto res = [Connecter, this, key, value, hkey]()
	{
		if (!ConnecterIsVaild(Connecter))return false;

		/**
		* HSET key field value					#将哈希表 key 中的字段 field 的值设为 value
		* HSET user:1 name "John" age 30		#键为 user:1，值是一个包含 name→John、age→30 的哈希表
		* hkey 是redis的 key的子建
		*/
		const RedisReply_Unique Reply(
			static_cast<redisReply*>(
				redisCommand(Connecter, "HSET %s %s %s", key.c_str(), hkey.c_str(), value.c_str())));
		if (Reply == nullptr || Reply->type != REDIS_REPLY_INTEGER)
		{
			std::cout << "执行操作 [ HSet " << key << "  " << hkey << "  " << value << " ] 错误 ! " << std::endl;
			return false;
		}
		std::cout << "执行操作 [ HSet " << key << "  " << hkey << "  " << value << " ] 成功 ! " << std::endl;
		return true;
	}();

	ConnectPool->ReturnWorker(std::move(Connect));
	return res;
}

bool SRedisMgr::HSet(const char* key, const char* hkey, const char* hvalue, size_t hvaluelen)
{
	auto Connect = ConnectPool->GetWorker();
	auto Connecter = Connect.get();

	auto res = [Connecter, this, key, hvalue, hkey, hvaluelen]()
	{
		if (!ConnecterIsVaild(Connecter))return false;

		const char* argv[4]{""};
		size_t argvlen[4]{0};

		argv[0] = "HSET";
		argvlen[0] = 4;

		argv[1] = key;
		argvlen[1] = strlen(key);

		argv[2] = hkey;
		argvlen[2] = strlen(hkey);

		argv[3] = hvalue;
		argvlen[3] = hvaluelen;

		const RedisReply_Unique Reply(static_cast<redisReply*>(redisCommandArgv(Connecter, 4, argv, argvlen)));

		/* 可以尝试在窗口调用 HSET之后的返回值是 二级键的数量；故返回值不是数值类型则错误*/
		if (Reply == nullptr || Reply->type != REDIS_REPLY_INTEGER)
		{
			std::cout << "执行操作 [ HSet " << key << "  " << hkey << "  " << hvalue << " ] 错误 ! " << std::endl;
			return false;
		}
		std::cout << "执行操作 [ HSet " << key << "  " << hkey << "  " << hvalue << " ] 成功 ! " << std::endl;
		return true;
	}();

	ConnectPool->ReturnWorker(std::move(Connect));
	return res;
}

std::string SRedisMgr::HGet(const std::string& key, const std::string& hkey)
{
	auto Connect = ConnectPool->GetWorker();
	auto Connecter = Connect.get();

	auto res = [Connecter, this, key, hkey]()
	{
		if (!ConnecterIsVaild(Connecter))return std::string("");

		const char* argv[3]{""};
		size_t argvlen[3]{0};
		argv[0] = "HGET";
		argvlen[0] = 4;
		argv[1] = key.c_str();
		argvlen[1] = key.length();
		argv[2] = hkey.c_str();
		argvlen[2] = hkey.length();

		const RedisReply_Unique Reply(static_cast<redisReply*>(redisCommandArgv(Connecter, 3, argv, argvlen)));
		if (Reply == nullptr || Reply->type == REDIS_REPLY_NIL)
		{
			std::cout << "执行操作 [ HGet " << key << " " << hkey << "  ] 错误 ! " << std::endl;
			return std::string("");
		}

		std::string value = Reply->str;
		std::cout << "执行操作 [ HGet " << key << " " << hkey << " ] 成功 ! " << std::endl;
		return value;
	}();

	ConnectPool->ReturnWorker(std::move(Connect));
	return res;
}

bool SRedisMgr::Del(const std::string& key)
{
	auto Connect = ConnectPool->GetWorker();
	auto Connecter = Connect.get();

	auto res = [Connecter, this, key]()
	{
		if (!ConnecterIsVaild(Connecter))return false;

		const RedisReply_Unique Reply(static_cast<redisReply*>(redisCommand(Connecter, "DEL %s", key.c_str())));
		if (Reply == nullptr || Reply->type != REDIS_REPLY_INTEGER)
		{
			std::cout << "执行操作 [ Del " << key << " ] 失败 ! " << std::endl;
			return false;
		}
		std::cout << "执行操作 [ Del " << key << " ] 成功 ! " << std::endl;
		return true;
	}();

	ConnectPool->ReturnWorker(std::move(Connect));
	return res;
}

bool SRedisMgr::ExistsKey(const std::string& key)
{
	auto Connect = ConnectPool->GetWorker();
	auto Connecter = Connect.get();

	auto res = [Connecter, this, key]()
	{
		if (!ConnecterIsVaild(Connecter))return false;

		const RedisReply_Unique Reply(static_cast<redisReply*>(redisCommand(Connecter, "exists %s", key.c_str())));
		if (Reply == nullptr || Reply->type != REDIS_REPLY_INTEGER || Reply->integer == 0)
		{
			std::cout << "未找到 [ Key " << key << " ]  ! " << std::endl;
			return false;
		}
		std::cout << " 找到 [ Key " << key << " ]  ! " << std::endl;
		return true;
	}();

	ConnectPool->ReturnWorker(std::move(Connect));
	return res;
}

bool SRedisMgr::ConnecterIsVaild(redisContext* Connecter) const
{
	if (!Connecter)
	{
		std::cout << "执行命令失败，Connecter不有效" << std::endl;
		return false;
	}

	return true;
}

SRedisMgr::SRedisMgr()
{
	const auto& ConfigMgr = Mgr::GetConfigHelper();
	auto Host = ConfigMgr.get<std::string>("Redis.Host");
	auto Port = ConfigMgr.get<int>("Redis.Port");
	auto Pwd = ConfigMgr.get<std::string>("Redis.Passwd");
	std::string Info = std::format("地址：{} 端口：{} 密码：{}", Host, Port, Pwd);// CPP20
	ConnectPool = RedisConnectPool_Unique::CreateWorkThread([&Host, &Port,&Pwd,&Info]()
		{
			RedisContext_Unique context(redisConnect(Host.c_str(), Port));

			if (context == nullptr || context->err != 0) {
	
				return RedisContext_Unique(nullptr);
			}
			const RedisReply_Unique Reply(static_cast<redisReply*>(redisCommand(context.get(), "AUTH %s", Pwd.c_str())));
		
			if (Reply->type == REDIS_REPLY_ERROR) {
				std::cout << Info <<"认证失败！" << std::endl;

				return RedisContext_Unique(nullptr);
			}

			std::cout << Info << "认证成功" << std::endl;
			return std::move(context);
		},
		5
	);
}
