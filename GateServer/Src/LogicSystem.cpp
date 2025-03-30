#include "LogicSystem.h"

#include <iostream>
#include <json/reader.h>
#include <json/value.h>

#include "HttpConnection.h"
#include "VarifyGrpcClient.h"

SLogicSystem::SLogicSystem()
{
	RegFuns();
}

void SLogicSystem::RegFuns()
{
	RegGet("/get_test", [](std::shared_ptr<CHttpConnection> connection)
		{
			beast::ostream(connection->Response.body()) << "收到Get请求：receive get_test req";

			int i = 0;
			for (auto& elem : connection->GetParams) {
				i++;
				beast::ostream(connection->Response.body()) << "param" << i << " key is " << elem.first;
				beast::ostream(connection->Response.body()) << ", " << " value is " << elem.second << std::endl;
			}
		});

	//获取注册验证码
	RegPost("/get_varifycode", [](std::shared_ptr<CHttpConnection> connection)
	{
		auto body_str = boost::beast::buffers_to_string(connection->Request.body().data());
		std::cout << "receive body is " << body_str << std::endl;

		connection->Response.set(http::field::content_type, "text/json");
		Json::Value root;//回给客户端的
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

		if(!src_root.isMember("email"))
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
}

SLogicSystem::~SLogicSystem()
{
}

void SLogicSystem::RegGet(std::string url, HttpHandler handler)
{
	GetHandlers.emplace(url, handler);
}

void SLogicSystem::RegPost(std::string url, HttpHandler handler)
{
	PostHandlers.emplace(url, handler);
}

bool SLogicSystem::HandlePost(std::string path, std::shared_ptr<CHttpConnection> con)
{
	if (PostHandlers.find(path) == PostHandlers.end())
	{
		return false;
	}

	PostHandlers[path](con);
	return true;
}

bool SLogicSystem::HandleGet(std::string path, std::shared_ptr<CHttpConnection> con)
{
	if (GetHandlers.find(path) == GetHandlers.end())
	{
		return false;
	}
	GetHandlers[path](con);
	return true;
}