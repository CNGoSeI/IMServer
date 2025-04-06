#include "LogicSystem.h"
#include <iostream>
#include <json/reader.h>
#include <json/value.h>
#include "HttpConnection.h"
#include "MysqlDAO.h"
#include "VarifyGrpcClient.h"
#include "RedisMgr.h"


SLogicSystem::SLogicSystem()
{
	RegFuncs();
}

void SLogicSystem::RegFuncs()
{
	RegGet(URI::Get_Test, [](std::shared_ptr<CHttpConnection> connection)
	{
		beast::ostream(connection->Response.body()) << "收到Get请求：receive get_test req";

		int i = 0;
		for (auto& elem : connection->GetParams)
		{
			i++;
			beast::ostream(connection->Response.body()) << "param" << i << " key is " << elem.first;
			beast::ostream(connection->Response.body()) << ", " << " value is " << elem.second << std::endl;
		}
	});

	//获取注册验证码
	RegPost(URI::Get_Varifycode, [](std::shared_ptr<CHttpConnection> connection)
	{
		auto body_str = boost::beast::buffers_to_string(connection->Request.body().data());
		std::cout << "receive body is " << body_str << std::endl;

		connection->Response.set(http::field::content_type, "text/json");
		Json::Value root; //回给客户端的
		Json::Reader reader;
		Json::Value src_root;

		bool parse_success = reader.parse(body_str, src_root);

		if (!parse_success)
		{
			std::cout << "未能成功解析Josn!" << std::endl;
			root["error"] = ErrorCodes::Error_Json;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->Response.body()) << jsonstr;
			return true;
		}

		if (!src_root.isMember("email"))
		{
			std::cout << "请求验证码，没有邮箱！" << std::endl;
			root["error"] = "Not Email Key";
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->Response.body()) << jsonstr;
			return true;
		}

		auto email = src_root["email"].asString();
		GetVarifyRsp rsp = CVerifyGrpcClient::GetInstance().GetVarifyCode(email);
		std::cout << "email is " << email << std::endl;
		root["error"] = rsp.error();
		root["email"] = src_root["email"];
		std::string jsonstr = root.toStyledString();
		beast::ostream(connection->Response.body()) << jsonstr;
		return true;
	});

	RegPost(URI::User_Register, [this](std::shared_ptr<CHttpConnection> conn)
	{
		this->ReqPostRegister(conn);
	});

	RegPost(URI::Rest_Passwd, [this](std::shared_ptr<CHttpConnection> conn)
	{
		this->ReqRestPasswd(conn);
	});
}

bool SLogicSystem::ReqPostRegister(std::shared_ptr<CHttpConnection> Connection)
{
	auto body_str = boost::beast::buffers_to_string(Connection->Request.body().data());
	std::cout << "收到注册请求，内容： " << body_str << std::endl;

	Connection->Response.set(http::field::content_type, "text/json");
	Json::Value ReqRoot;//回应的Json值
	Json::Reader reader;
	Json::Value SrcRoot;//解析出投递请求的Json值

	bool parse_success = reader.parse(body_str, SrcRoot);

	if (!parse_success) {
		std::cout << "解析注册投递Json失败!" << std::endl;
		ReqRoot["error"] = ErrorCodes::Error_Json;
		std::string jsonstr = ReqRoot.toStyledString();
		beast::ostream(Connection->Response.body()) << jsonstr;
		return true;
	}

	auto email = SrcRoot["email"].asString();
	auto name = SrcRoot["user"].asString();
	auto pwd = SrcRoot["passwd"].asString();
	auto confirm = SrcRoot["confirm"].asString();

	//先查找redis中email对应的验证码是否合理
	std::string  VarifyCode;
	bool bFindVarify = SRedisMgr::GetInstance().Get(Prefix::CODEPREFIX+email, VarifyCode);

	//未找到验证码或者验证码不对
	if ((!bFindVarify)||( VarifyCode != SrcRoot["varifycode"].asString())) {
		std::cout << " 未找到有效验证码" << std::endl;
		ReqRoot["error"] = ErrorCodes::VarifyExpired;
		std::string jsonstr = ReqRoot.toStyledString();
		beast::ostream(Connection->Response.body()) << jsonstr;
		return true;
	}

	//查找数据库判断用户是否存在,UID每注册一次+1
	int uid = SMysqlDao::GetInstance().RegUser(name, email, pwd);
	if (uid == 0 || uid == -1) {
		std::cout << "用户或邮箱已存在" << std::endl;
		ReqRoot["error"] = ErrorCodes::UserExist;
		std::string jsonstr = ReqRoot.toStyledString();
		beast::ostream(Connection->Response.body()) << jsonstr;
		return true;
	}

	ReqRoot["error"] = 0;
	ReqRoot["email"] = SrcRoot["email"];
	ReqRoot["user"] = SrcRoot["user"].asString();
	ReqRoot["passwd"] = SrcRoot["passwd"].asString();
	ReqRoot["confirm"] = SrcRoot["confirm"].asString();
	ReqRoot["varifycode"] = SrcRoot["varifycode"].asString();
	std::string jsonstr = ReqRoot.toStyledString();
	beast::ostream(Connection->Response.body()) << jsonstr;
	return true;
}

bool SLogicSystem::ReqRestPasswd(std::shared_ptr<CHttpConnection> Connection)
{
	auto body_str = boost::beast::buffers_to_string(Connection->Request.body().data());
	std::cout << "收到数据体：" << body_str << std::endl;

	Connection->Response.set(http::field::content_type, "text/json");
	Json::Value ResRoot;//回应的Json
	Json::Reader reader;
	Json::Value ReqRoot;//收到的Json

	bool parse_success = reader.parse(body_str, ReqRoot);

	if (!parse_success)
	{
		std::cout << "解析Json失败!" << std::endl;
		ResRoot["error"] = ErrorCodes::Error_Json;
		std::string jsonstr = ResRoot.toStyledString();
		beast::ostream(Connection->Response.body()) << jsonstr;
		return true;
	}

	auto email = ReqRoot["email"].asString();
	auto name = ReqRoot["user"].asString();
	auto pwd = ReqRoot["passwd"].asString();

	//先查找redis中email对应的验证码是否合理
	std::string varify_code;
	bool b_get_varify = SRedisMgr::GetInstance().Get(Prefix::CODEPREFIX + email, varify_code);
	if (!b_get_varify)
	{
		std::cout << email <<" 重置密码"<<"未找到对应验证码" << std::endl;
		ResRoot["error"] = ErrorCodes::VarifyExpired;
		std::string jsonstr = ResRoot.toStyledString();
		beast::ostream(Connection->Response.body()) << jsonstr;
		return true;
	}

	if (varify_code != ReqRoot["varifycode"].asString())
	{
		std::cout << email << " 重置密码" << "验证码不正确" << std::endl;
		ResRoot["error"] = ErrorCodes::VarifyCodeErr;
		std::string jsonstr = ResRoot.toStyledString();
		beast::ostream(Connection->Response.body()) << jsonstr;
		return true;
	}

	//查询数据库判邮箱是否存在
	bool email_valid = SMysqlDao::GetInstance().CheckEmail(name, email);
	if (!email_valid)
	{
		std::cout << "用户邮箱和用户名不配对" << std::endl;
		ResRoot["error"] = ErrorCodes::EmailNotMatch;
		std::string jsonstr = ResRoot.toStyledString();
		beast::ostream(Connection->Response.body()) << jsonstr;
		return true;
	}

	//更新密码为最新密码
	bool b_up = SMysqlDao::GetInstance().UpdatePwd(name, pwd);
	if (!b_up)
	{
		std::cout << "重制密码失败" << std::endl;
		ResRoot["error"] = ErrorCodes::PasswdUpFailed;
		std::string jsonstr = ResRoot.toStyledString();
		beast::ostream(Connection->Response.body()) << jsonstr;
		return true;
	}

	std::cout << "成功重制用户密码 " <<email<< pwd << std::endl;
	ResRoot["error"] = 0;
	ResRoot["email"] = email;
	ResRoot["user"] = name;
	ResRoot["passwd"] = pwd;
	ResRoot["varifycode"] = ReqRoot["varifycode"].asString();
	std::string jsonstr = ResRoot.toStyledString();
	beast::ostream(Connection->Response.body()) << jsonstr;
	return true;
}

SLogicSystem::~SLogicSystem()
{
}

void SLogicSystem::RegGet(const std::string& url, HttpHandler handler)
{
	GetHandlers.emplace(url, handler);
}

void SLogicSystem::RegPost(const std::string& url, HttpHandler handler)
{
	PostHandlers.emplace(url, handler);
}

bool SLogicSystem::HandlePost(const std::string& path, std::shared_ptr<CHttpConnection> con)
{
	const auto it = PostHandlers.find(path);
	if (it == PostHandlers.end())
	{
		return false;
	}
	it->second(con);

	return true;
}

bool SLogicSystem::HandleGet(const std::string& path, std::shared_ptr<CHttpConnection> con)
{
	const auto it = GetHandlers.find(path);
	if (it == GetHandlers.end())
	{
		return false;
	}
	it->second(con);
	return true;
}