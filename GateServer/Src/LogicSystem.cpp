#include "LogicSystem.h"
#include "HttpConnection.h"

SLogicSystem::SLogicSystem()
{
	RegGet("/get_test", [](std::shared_ptr<CHttpConnection> connection)
	{
		beast::ostream(connection->Response.body()) << "收到Get请求：receive get_test req";
	});
}

SLogicSystem::~SLogicSystem()
{
}

void SLogicSystem::RegGet(std::string url, HttpHandler handler)
{
	GetHandlers.emplace(url, handler);
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
