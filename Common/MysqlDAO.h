#ifndef MYSQLDAO_H
#define MYSQLDAO_H
#include "SingletonTemplate.h"
#include "ThreadWorkerTemplate.h"

namespace sql
{
	class Connection;
	class SQLException;
}

struct UserInfo;

using SqlConnection_Unique=std::unique_ptr<sql::Connection>;
using SqlConnecPool_Unique = TThreadWoker<SqlConnection_Unique>;

class SMysqlDao:public TSingleton<SMysqlDao>
{
	friend class TSingleton<SMysqlDao>;
public:

	~SMysqlDao();
	int RegUser(const std::string& name, const std::string& email, const std::string& pwd);
	bool CheckEmail(const std::string& name, const std::string& email);//��������Ƿ����
	bool UpdatePwd(const std::string& name, const std::string& newpwd);//��������
	bool CheckPwd(const std::string& name, const std::string& pwd, UserInfo& userInfo);
	std::shared_ptr<UserInfo> GetUser(int uid);

private:
	SMysqlDao();
	void CatchError(SqlConnection_Unique, sql::SQLException& e);//����ִ��sql�쳣
	std::unique_ptr <SqlConnecPool_Unique> SqlPool{ nullptr };
	std::string Schema;//�������ݿ����ӵ�Ĭ��ģʽ
};

struct UserInfo {
	std::string name{""};
	std::string pwd{ "" };
	int uid{0};
	std::string email{ "" };
	std::string nick{ "" };
	std::string desc{ "" };
	int sex{0};
	std::string icon{ "" };
	std::string back{ "" };
};

#endif // MYSQLDAO_H
