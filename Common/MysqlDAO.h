#ifndef MYSQLDAO_H
#define MYSQLDAO_H
#include "SingletonTemplate.h"
#include "ThreadWorkerTemplate.h"
#include <mysql/jdbc.h>

using SqlConnection_Unique=std::unique_ptr<sql::Connection>;
using SqlConnecPool_Unique = TThreadWoker<SqlConnection_Unique>;

class SMysqlDao:public TSingleton<SMysqlDao>
{
	friend class TSingleton<SMysqlDao>;
public:

	~SMysqlDao();
	int RegUser(const std::string& name, const std::string& email, const std::string& pwd);
	std::unique_ptr <SqlConnecPool_Unique> SqlPool{ nullptr };
	std::string Schema;//�������ݿ����ӵ�Ĭ��ģʽ
private:
	SMysqlDao();

};
#endif // MYSQLDAO_H
