#ifndef MYSQLDAO_H
#define MYSQLDAO_H
#include "SingletonTemplate.h"
#include "ThreadWorkerTemplate.h"

namespace sql
{
	class Connection;
	class SQLException;
}

struct UserInfo {
	std::string name{ "" };
	std::string pwd{ "" };
	int uid{ 0 };
	std::string email{ "" };
	std::string nick{ "" };
	std::string desc{ "" };
	int sex{ 0 };
	std::string icon{ "" };
	std::string back{ "" };
};

struct FApplyInfo {
	FApplyInfo(int uid, const std::string& name, const std::string& desc,
		const std::string& icon, const std::string& nick, int sex, int status);

	int Uid{ 0 };
	std::string Name{ "" };
	std::string Desc{ "" };
	std::string Icon{ "" };
	std::string Nick{ "" };
	int Sex{ 0 };
	int Status{ 0 };
};

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
	std::shared_ptr<UserInfo> GetUser(std::string name);
	bool AddFriendApply(int from,int to);//�����ݿ���Ӻ������������
	/**
	 * ��ȡ���������б�
	 * @param touid Ŀ��uid
	 * @param applyList ��ȡ������Ϣ�б�
	 * @param offset �ӵڼ�����ʼ��ȡ
	 * @param limit ��ȡ���ٸ�
	 * @return 
	 */
	bool GetApplyList(int touid, std::vector<FApplyInfo>& applyList, int offset=0, int limit=6);
	bool AuthFriendApply(const int from, const int to);
	bool AddFriend(const int from, const int to, const std::string& back_name);
private:
	SMysqlDao();
	void CatchError(SqlConnection_Unique, sql::SQLException& e);//����ִ��sql�쳣
	std::unique_ptr <SqlConnecPool_Unique> SqlPool{ nullptr };
	std::string Schema;//�������ݿ����ӵ�Ĭ��ģʽ
};



#endif // MYSQLDAO_H
