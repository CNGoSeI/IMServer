#include "Server.h"
#include <iostream>
#include <functional>
#include "AsioIOServicePool.h"
#include "Session.h"
#include "UserMgr.h"

CServer::CServer(boost::asio::io_context& io_context, const short port):
	IoContext(io_context),
	Port(port),
	Acceptor(io_context, tcp::endpoint(tcp::v4(), port))
{
	std::cout << "服务器启动成功, 监听端口 : " << Port << std::endl;
	StartAccept();
}

CServer::~CServer()
{
}

void CServer::ClearSession(const std::string& UUID)
{

	auto it = Str2Session.find(UUID);
	if (it != Str2Session.end()) {
		//移除用户和session的关联
		
		SUserMgr::GetInstance().RmvUserSession(it->second->GetUserId());
	}

	{
		std::lock_guard<std::mutex> lock(Mutex);
		Str2Session.erase(UUID);
	}
}

void CServer::HandleAccept(std::shared_ptr<CSession> Session, const boost::system::error_code& error)
{
	if (!error) {
		Session->Start();
		std::lock_guard<std::mutex> lock(Mutex);
		Str2Session.emplace(Session->GetUUID(), Session);
	}
	else {
		std::cout << "会话 accept 错误：" << error.what() << std::endl;
	}

	StartAccept();
}

void CServer::StartAccept()
{
	auto& io_context = SAsioIOServicePool::GetInstance().GetIOService();
	//新建会话
	std::shared_ptr<CSession> new_session = make_shared<CSession>(io_context, this);//将IO管理给会话服务用

	//对会话套接字进行Acceptor
	Acceptor.async_accept(new_session->GetSocket(),
	                      [this,new_session](const boost::system::error_code& error)
	                      {
		                      HandleAccept(new_session, error);
	                      });
}
