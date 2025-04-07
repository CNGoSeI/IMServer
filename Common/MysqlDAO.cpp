#include <mysql/jdbc.h>
#include <mysqlx/xdevapi.h>
#include <jdbc/mysql_connection.h>
#include <string>
#include "MysqlDAO.h"
#include "ConfigMgr.h"

SMysqlDao::~SMysqlDao()
{
	SqlPool->Close();
	SqlPool = nullptr;
};

int SMysqlDao::RegUser(const std::string& name, const std::string& email, const std::string& pwd)
{
	auto coner = SqlPool->GetWorker();
	
	auto ExeFunc = [con= coner.get()](const std::string& name, const std::string& email, const std::string& pwd)
	{
			if (con == nullptr) {
				return 0;
			}
			// 开启事务
			con->setAutoCommit(false);

			// 检查唯一性（加锁）
			std::unique_ptr<sql::PreparedStatement> checkStmt(
				con->prepareStatement("SELECT COUNT(*) FROM `user` WHERE `name`=? OR `email`=? FOR UPDATE")
			);
			checkStmt->setString(1, name);
			checkStmt->setString(2, email);
			std::unique_ptr<sql::ResultSet> checkRes(checkStmt->executeQuery());

			if (checkRes->next() && checkRes->getInt(1) > 0) {
				con->rollback();
				return 0; // 用户已存在
			}

			// 获取新UID并插入用户
			std::unique_ptr<sql::Statement> uidStmt(con->createStatement());
			uidStmt->execute("UPDATE `user_id` SET `id` = `id` + 1");
			std::unique_ptr<sql::ResultSet> uidRes(uidStmt->executeQuery("SELECT `id` FROM `user_id`"));
			uidRes->next();
			int newUid = uidRes->getInt("id");

			std::unique_ptr<sql::PreparedStatement> insertStmt(
				con->prepareStatement("INSERT INTO `user` (`uid`, `name`, `email`, `pwd`) VALUES (?, ?, ?, ?)")
			);
			insertStmt->setInt(1, newUid);
			insertStmt->setString(2, name);
			insertStmt->setString(3, email);
			insertStmt->setString(4, pwd);
			insertStmt->executeUpdate();

			con->commit();
			return newUid; // 返回新UID
	};
	try {
		const auto res = ExeFunc(name, email, pwd);
		SqlPool->ReturnWorker(std::move(coner));
		return res;
	}
	catch (sql::SQLException& e) {
		
		coner->rollback();
		CatchError(std::move(coner), e);
		return -1;
	}

}

bool SMysqlDao::CheckEmail(const std::string& name, const std::string& email)
{
	auto con = SqlPool->GetWorker();

	auto ExeFunc = [con=con.get()](const std::string& name, const std::string& email)
	{
			// 准备查询语句
			std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("SELECT email FROM user WHERE name = ?"));

			// 绑定参数
			pstmt->setString(1, name);

			// 执行查询
			std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

			// 遍历结果集 但是注册用户会检测邮箱唯一性，故只循环一次
			while (res->next()) {
				std::cout << "Check Email: " << res->getString("email") << std::endl;
				if (email != res->getString("email")) {
					return false;
				}
				return true;
			}
			return false;
	};
	try {
		if (con == nullptr) {
			return false;
		}
		const auto res = ExeFunc(name, email);
		SqlPool->ReturnWorker(std::move(con));
		return res;
	}
	catch (sql::SQLException& e) {
		CatchError(std::move(con), e);
		return false;
	}
}

bool SMysqlDao::UpdatePwd(const std::string& name, const std::string& newpwd)
{
	auto con = SqlPool->GetWorker();

	auto ExeFunc = [con = con.get()](const std::string& name, const std::string& newpwd)
	{
		// 准备查询语句
		std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("UPDATE user SET pwd = ? WHERE name = ?"));

		// 绑定参数
		pstmt->setString(1, newpwd);
		pstmt->setString(2, name);

		// 执行更新
		int updateCount = pstmt->executeUpdate();

		std::cout << "更新行: " << updateCount << std::endl;
		return true;
	};

	try
	{
		if (con == nullptr)
		{
			return false;
		}
		const auto res = ExeFunc(name, newpwd);
		SqlPool->ReturnWorker(std::move(con));
		return res;
	}
	catch (sql::SQLException& e)
	{
		CatchError(std::move(con), e);
		return false;
	}
}

bool SMysqlDao::CheckPwd(const std::string& name, const std::string& pwd, UserInfo& userInfo)
{
	auto con = SqlPool->GetWorker();

	auto ExeFunc = [con = con.get()](const std::string& name, const std::string& pwd, UserInfo& userInfo)
		{
			// 准备SQL语句
			std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("SELECT * FROM user WHERE name = ?"));
			pstmt->setString(1, name); 

			// 执行查询
			std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
			std::string origin_pwd = "";
			// 遍历结果集
			while (res->next()) {
				origin_pwd = res->getString("pwd");
				// 输出查询到的密码
				std::cout << "找到密码: " << origin_pwd << std::endl;
				break;
			}

			if (pwd != origin_pwd) {
				return false;
			}

			userInfo.name = name;
			userInfo.email = res->getString("email");
			userInfo.uid = res->getInt("uid");
			userInfo.pwd = origin_pwd;
			return true;
		};

	try
	{
		if (con == nullptr)
		{
			return false;
		}

		const auto res = ExeFunc(name, pwd, userInfo);
		SqlPool->ReturnWorker(std::move(con));
		return res;
	}
	catch (sql::SQLException& e)
	{
		CatchError(std::move(con), e);
		return false;
	}
}

SMysqlDao::SMysqlDao()
{
	const auto& Config = Mgr::GetConfigHelper();
	auto Host = Config.get<std::string>("Mysql.Host");
	auto Port = Config.get<std::string>("Mysql.Port");
	auto User = Config.get<std::string>("Mysql.User");
	auto Pwd = Config.get<std::string>("Mysql.Passwd");
	auto SchemaCif = Config.get<std::string>("Mysql.Schema","");
	Schema = SchemaCif;

	auto driver = sql::mysql::get_mysql_driver_instance();
	SqlPool = SqlConnecPool_Unique::CreateWorkThread([&Host, &Port,&Pwd,&User,&SchemaCif,&driver]()
	{
		const auto url = Host + ":" + Port;
		
		std::unique_ptr<sql::Connection> con(driver->connect(url, User, Pwd));
		con->setSchema(SchemaCif);
		return std::move(con);
	},
		5
	);
}

void SMysqlDao::CatchError(SqlConnection_Unique con, sql::SQLException& e)
{
	con->reconnect();//异常的情况下，为了保险起见，可以进行一次重新连接
	SqlPool->ReturnWorker(std::move(con));
	std::cerr << "SQL 异常: " << e.what();
	std::cerr << " (MySQL 错误码: " << e.getErrorCode();
	std::cerr << ", SQL 状态: " << e.getSQLState() << " )" << std::endl;
}
