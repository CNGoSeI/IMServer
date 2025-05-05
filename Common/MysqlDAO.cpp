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

std::shared_ptr<UserInfo> SMysqlDao::GetUser(int uid)
{
	auto con = SqlPool->GetWorker();

	auto ExeFunc = [con = con.get()](int uid)
	{
		// 准备SQL语句
		std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("SELECT * FROM user WHERE uid = ?"));
		pstmt->setInt(1, uid); // 将uid替换为你要查询的uid

		// 执行查询
		std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
		std::shared_ptr<UserInfo> user_ptr = nullptr;

		// 遍历结果集
		while (res->next())
		{
			user_ptr.reset(new UserInfo);
			user_ptr->pwd = res->getString("pwd");
			user_ptr->email = res->getString("email");
			user_ptr->name = res->getString("name");
			user_ptr->icon = res->getString("icon");
			user_ptr->uid = uid;
			break;
		}
		return user_ptr;
	};

	try
	{
		if (con == nullptr)
		{
			return nullptr;
		}

		const auto res = ExeFunc(uid);
		SqlPool->ReturnWorker(std::move(con));
		return res;

	}catch(sql::SQLException& e)
	{
		CatchError(std::move(con), e);
		return nullptr;
	}

}


std::shared_ptr<UserInfo> SMysqlDao::GetUser(std::string name)
{
	auto con = SqlPool->GetWorker();

	auto ExeFunc = [con = con.get(), name]()
		{
			// 准备SQL语句
			std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("SELECT * FROM user WHERE name = ?"));
			pstmt->setString(1, name); // 将uid替换为你要查询的uid

			// 执行查询
			std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
			std::shared_ptr<UserInfo> user_ptr = nullptr;
			// 遍历结果集
			while (res->next()) {
				user_ptr.reset(new UserInfo);
				user_ptr->pwd = res->getString("pwd");
				user_ptr->email = res->getString("email");
				user_ptr->name = res->getString("name");
				//user_ptr->nick = res->getString("nick");
				//user_ptr->desc = res->getString("desc");
				//user_ptr->sex = res->getInt("sex");
				user_ptr->uid = res->getInt("uid");
				break;
			}
			return user_ptr;
		};

	try {
		if (con == nullptr)
		{
			return nullptr;
		}

		const auto res = ExeFunc();
		SqlPool->ReturnWorker(std::move(con));
		return res;
	}
	catch (sql::SQLException& e) {
		CatchError(std::move(con), e);
		return nullptr;
	}
}

bool SMysqlDao::AddFriendApply(int from, int to)
{
	auto con = SqlPool->GetWorker();

	auto ExeFunc = [con = con.get(), from,to]()
	{
		// 准备SQL语句
		std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement(
			"INSERT INTO friend_apply (from_uid, to_uid) values (?,?) "
			"ON DUPLICATE KEY UPDATE from_uid = from_uid, to_uid = to_uid")); //如果发生键冲突，则执行更新
		pstmt->setInt(1, from); // from id
		pstmt->setInt(2, to);
		// 执行更新
		int rowAffected = pstmt->executeUpdate();
		if (rowAffected < 0)
		{
			return false;
		}
		return true;
	};

	try
	{
		if (con == nullptr)
		{
			return false;
		}

		const auto res = ExeFunc();
		SqlPool->ReturnWorker(std::move(con));
		return res;
	}
	catch (sql::SQLException& e)
	{
		CatchError(std::move(con), e);
		return false;
	}
}

bool SMysqlDao::GetApplyList(int touid, std::vector<FApplyInfo>& applyList, int offset, int limit)
{
	auto con = SqlPool->GetWorker();

	auto ExeFunc = [con = con.get(), &touid, offset, limit,&applyList]()
		{
			std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("select apply.from_uid, apply.status, user.name, "
				"user.nick, user.sex from friend_apply as apply join user on apply.from_uid = user.uid where apply.to_uid = ? "
				"and apply.id > ? order by apply.id ASC LIMIT ? "));

			pstmt->setInt(1, touid); // 将uid替换为你要查询的uid
			pstmt->setInt(2, offset); // 起始id
			pstmt->setInt(3, limit); //偏移量
			// 执行查询
			std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
			// 遍历结果集
			while (res->next()) {
				auto name = res->getString("name");
				auto uid = res->getInt("from_uid");
				auto status = res->getInt("status");
				//auto nick = res->getString("nick");
				//auto sex = res->getInt("sex");
				applyList.emplace_back(FApplyInfo(uid, name, "", "", "", 0, status));
			}
			return true;
		};

	try
	{
		if (con == nullptr)
		{
			return false;
		}

		const auto res = ExeFunc();
		SqlPool->ReturnWorker(std::move(con));
		return res;
	}
	catch (sql::SQLException& e)
	{
		CatchError(std::move(con), e);
		return false;
	}
}

bool SMysqlDao::AuthFriendApply(const int from, const int to)
{
	auto con = SqlPool->GetWorker();

	auto ExeFunc = [con = con.get(), from, to]()
	{
		// 准备SQL语句
		std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("UPDATE friend_apply SET status = 1 "
			"WHERE from_uid = ? AND to_uid = ?"));
		//反过来的申请时from，验证时to
		pstmt->setInt(1, to); // from id
		pstmt->setInt(2, from);
		// 执行更新
		int rowAffected = pstmt->executeUpdate();
		if (rowAffected < 0)
		{
			return false;
		}
		return true;
	};

	try
	{
		if (con == nullptr)
		{
			return false;
		}

		const auto res = ExeFunc();
		SqlPool->ReturnWorker(std::move(con));
		return res;
	}
	catch (sql::SQLException& e)
	{
		CatchError(std::move(con), e);
		return false;
	}
}

bool SMysqlDao::AddFriend(const int from, const int to, const std::string& back_name)
{
	auto con = SqlPool->GetWorker();

	auto ExeFunc = [con = con.get(), from, to,&back_name]()
	{
		//开始事务
		con->setAutoCommit(false);

		// 准备第一个SQL语句, 插入认证方好友数据
		std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement(
			"INSERT IGNORE INTO friend(self_id, friend_id, back) "
			"VALUES (?, ?, ?) "
		));
		//反过来的申请时from，验证时to
		pstmt->setInt(1, from); // from id
		pstmt->setInt(2, to);
		pstmt->setString(3, back_name);
		// 执行更新
		int rowAffected = pstmt->executeUpdate();
		if (rowAffected < 0)
		{
			con->rollback();
			return false;
		}

		//准备第二个SQL语句，插入申请方好友数据
		std::unique_ptr<sql::PreparedStatement> pstmt2(con->prepareStatement(
			"INSERT IGNORE INTO friend(self_id, friend_id, back) "
			"VALUES (?, ?, ?) "
		));
		//反过来的申请时from，验证时to
		pstmt2->setInt(1, to); // from id
		pstmt2->setInt(2, from);
		pstmt2->setString(3, "");
		// 执行更新
		int rowAffected2 = pstmt2->executeUpdate();
		if (rowAffected2 < 0)
		{
			con->rollback();
			return false;
		}

		// 提交事务
		con->commit();
		std::cout << "addfriend insert friends success" << std::endl;

		return true;
	};

	try
	{
		if (con == nullptr)
		{
			return false;
		}

		const auto res = ExeFunc();
		SqlPool->ReturnWorker(std::move(con));
		return res;
	}
	catch (sql::SQLException& e)
	{
		con->rollback();
		CatchError(std::move(con), e);
		return false;
	}
}

bool SMysqlDao::GetFriendList(int self_id, std::vector<UserInfo>& user_info_lst)
{
	auto con = SqlPool->GetWorker();

	auto ExeFunc = [con = con.get(), &user_info_lst, self_id, this]()
	{
		// 准备SQL语句, 根据起始id和限制条数返回列表
		std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("select * from friend where self_id = ? "));

		pstmt->setInt(1, self_id); // 将uid替换为你要查询的uid

		// 执行查询
		std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
		// 遍历结果集
		while (res->next())
		{
			auto friend_id = res->getInt("friend_id");
			auto back = res->getString("back");
			//再一次查询friend_id对应的信息
			auto user_info = GetUser(friend_id);
			if (user_info == nullptr)
			{
				continue;
			}

			user_info->back = user_info->name;
			user_info_lst.push_back(*user_info);
		}

		return true;
	};

	try
	{
		if (con == nullptr)
		{
			return false;
		}

		const auto res = ExeFunc();
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

FApplyInfo::FApplyInfo(int uid, const std::string& name, const std::string& desc, const std::string& icon,
	const std::string& nick, int sex, int status):
	Uid(uid),
	Name(std::move(name)),
	Desc(std::move(desc)),
	Icon(std::move(icon)),
	Nick(std::move(nick)),
	Sex(sex),
	Status(status)
{
}
