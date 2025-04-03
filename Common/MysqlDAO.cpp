#include "MysqlDAO.h"
#include "ConfigMgr.h"

SMysqlDao::~SMysqlDao()
{
}

int SMysqlDao::RegUser(const std::string& name, const std::string& email, const std::string& pwd)
{
	auto coner = SqlPool->GetWorker();

	auto ExeFunc = [con= coner.get()](const std::string& name, const std::string& email, const std::string& pwd)
	{
			if (con == nullptr) {
				return 0;
			}
			// call 表示调用存储过程，该逻辑名称为 reg_user；
			std::unique_ptr < sql::PreparedStatement > stmt
					(con->prepareStatement("CALL reg_user(?,?,?,@result)"));//@result 相当于定义了一个变量，接受输出参数
			// 设置输入参数
			stmt->setString(1, name);
			stmt->setString(2, email);
			stmt->setString(3, pwd);

			// 由于PreparedStatement不直接支持注册输出参数，我们需要使用会话变量或其他方法来获取输出参数的值

			stmt->execute();// 执行存储过程

			// 如果存储过程设置了会话变量或有其他方式获取输出参数的值，你可以在这里执行SELECT查询来获取它们
		   // 例如，如果存储过程设置了一个会话变量@result来存储输出结果，可以这样获取：
			std::unique_ptr<sql::Statement> stmtResult(con->createStatement());

		/**
		 * AS 表示别名 这里可以理解用 result 表示 @result；像引用？
		 * 这里主要想要获取上次执行语句的输出参数
		 */
			std::unique_ptr<sql::ResultSet> res(stmtResult->executeQuery("SELECT @result AS result"));
			if (res->next()) {
				int result = res->getInt("result");
				std::cout << "执行reg_user 返回值: " << result << std::endl;
				return result;
			}
			return -1;
	};
	try {
		const auto res = ExeFunc(name, email, pwd);
		SqlPool->ReturnWorker(std::move(coner));
		return res;
	}
	catch (sql::SQLException& e) {
		SqlPool->ReturnWorker(std::move(coner));
		std::cerr << "SQL 异常: " << e.what();
		std::cerr << " (MySQL 错误码: " << e.getErrorCode();
		std::cerr << ", SQL 状态: " << e.getSQLState() << " )" << std::endl;
		return -1;
	}

}

SMysqlDao::SMysqlDao()
{
	const auto& Config = Mgr::GetConfigHelper();
	auto Host = Config.get<std::string>("Mysql.Host");
	auto Port = Config.get<std::string>("Mysql.Port");
	auto User = Config.get<std::string>("Mysql.User");
	auto Pwd = Config.get<std::string>("Mysql.Passwd");
	auto SchemaCif = Config.get<std::string>("Mysql.Schema");
	Schema = SchemaCif;
	SqlPool = SqlConnecPool_Unique::CreateWorkThread([&Host, &Port,&Pwd,&User,&SchemaCif]()
	{
		sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
		std::unique_ptr<sql::Connection> con(driver->connect(Host + ":" + Port, User, Pwd));
		con->setSchema(SchemaCif);
		return std::move(con);
	}
	);
}
