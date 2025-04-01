#include "RedisMgr.h"

bool SRedisMgr::Connect(const std::string& host, int port)
{
	this->Connecter = redisConnect(host.c_str(), port);

	if (this->Connecter != NULL && this->Connecter->err)
	{
		std::cout << "Redis链接错误 " << this->Connecter->errstr << std::endl;
		Connecter = nullptr;
		return false;
	}
	return true;
}

bool SRedisMgr::Get(const std::string& key, std::string& value)
{
	if (!ConnecterIsVaild())return false;

	this->Reply = (redisReply*)redisCommand(this->Connect, "GET %s", key.c_str());
	if (this->Reply == NULL) {
		std::cout << "[ GET  " << key << " ] 失败" << std::endl;
		freeReplyObject(this->Reply);
		return false;
	}
	//返回的不是字符串，有问题
	if (this->Reply->type != REDIS_REPLY_STRING) {
		std::cout << "[ GET  " << key << " ] 失败，返回值类型非字符串" << std::endl;
		freeReplyObject(this->Reply);
		return false;
	}

	value = this->Reply->str;
	freeReplyObject(this->Reply);

	std::cout << "成功执行命令 [ GET " << key << "  ]" << std::endl;
	return true;
}

bool SRedisMgr::Set(const std::string& key, const std::string& value)
{
	if (!ConnecterIsVaild())return false;

	//执行redis命令行
	this->Reply = (redisReply*)redisCommand(this->Connecter, "SET %s %s", key.c_str(), value.c_str());

	//如果返回NULL则说明执行失败
	if (this->Reply==NULL)
	{
		std::cout << "执行操作 [ SET " << key << "  " << value << " ] 失败 ! " << std::endl;
		freeReplyObject(this->Reply);
		return false;
	}
	//如果执行失败则释放连接
	if (!(this->Reply->type == REDIS_REPLY_STATUS && (strcmp(this->Reply->str, "OK") == 0 || strcmp(this->Reply->str, "ok") == 0)))
	{
		std::cout << "执行操作 [ SET " << key << "  " << value << " ] 失败 ! " << std::endl;
		freeReplyObject(this->Reply);
		return false;
	}

	//执行成功 释放redisCommand执行后返回的redisReply所占用的内存
	freeReplyObject(this->_reply);
	std::cout << "执行操作 [ SET " << key << "  " << value << " ] 成功 ! " << std::endl;
	return true;
}

bool SRedisMgr::Auth(const std::string& password)
{
	if (!ConnecterIsVaild())return false;

	this->Reply = (redisReply*)redisCommand(this->Connecter, "AUTH %s", password.c_str());
	if (this->Reply->type == REDIS_REPLY_ERROR) {
		std::cout << "认证失败" << std::endl;
		freeReplyObject(this->Reply);
		return false;
	}
	else {
		//执行成功 释放redisCommand执行后返回的redisReply所占用的内存
		freeReplyObject(this->Reply);
		std::cout << "认证成功" << std::endl;
		return true;
	}
}

bool SRedisMgr::LPush(const std::string& key, const std::string& value)
{
	if (!ConnecterIsVaild())return false;

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
	this->Reply = (redisReply*)redisCommand(this->Connecter, "LPUSH %s %s", key.c_str(), value.c_str());
	if (NULL == this->Reply)
	{
		std::cout << "执行操作 [ LPUSH " << key << "  " << value << " ] 错误 ! " << std::endl;
		freeReplyObject(this->Reply);
		return false;
	}

	if (this->Reply->type != REDIS_REPLY_INTEGER || this->Reply->integer <= 0) {
		std::cout << "执行操作 [ LPUSH " << key << "  " << value << " ] 错误 ! " << std::endl;
		freeReplyObject(this->Reply);
		return false;
	}

	std::cout << "执行操作 [ LPUSH " << key << "  " << value << " ] 成功 ! " << std::endl;
	freeReplyObject(this->Reply);
	return true;
}

bool SRedisMgr::ConnecterIsVaild() const
{
	if (!Connecter)
	{
		std::cout << "[ GET  " << key << " ] 失败，Connecter不有效" << std::endl;
		return false;
	}

	return true;
}
