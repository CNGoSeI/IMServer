#ifndef MYSQLDAO_H
#define MYSQLDAO_H
#include "SingletonTemplate.h"
#include "ThreadWorkerTemplate.h"

namespace sql
{
	class Connection;
	class SQLException;
}

using SqlConnection_Unique=std::unique_ptr<sql::Connection>;
using SqlConnecPool_Unique = TThreadWoker<SqlConnection_Unique>;

class SMysqlDao:public TSingleton<SMysqlDao>
{
	friend class TSingleton<SMysqlDao>;
public:

	~SMysqlDao();
	int RegUser(const std::string& name, const std::string& email, const std::string& pwd);
	bool CheckEmail(const std::string& name, const std::string& email);//检测邮箱是否存在
	bool UpdatePwd(const std::string& name, const std::string& newpwd);//更新密码

private:
	SMysqlDao();
	void CatchError(SqlConnection_Unique, sql::SQLException& e);//捕获到执行sql异常
	std::unique_ptr <SqlConnecPool_Unique> SqlPool{ nullptr };
	std::string Schema;//设置数据库连接的默认模式
};
#endif // MYSQLDAO_H
