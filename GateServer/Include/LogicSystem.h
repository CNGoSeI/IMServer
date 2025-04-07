#ifndef LOGICSYSTEM_H
#define LOGICSYSTEM_H

/************************************
 * http解析URL后，在此类执行相应方法	*
 *	采用键值对映射URL和执行的方法		*
 ************************************/
#include "SingletonTemplate.h"
#include <functional>
#include <map>
#include "const.h"

class CHttpConnection;
typedef std::function<void(std::shared_ptr<CHttpConnection>)> HttpHandler;

class SLogicSystem : public TSingleton<SLogicSystem>
{
	friend class TSingleton<SLogicSystem>;

public:
	~SLogicSystem();

	/* 执行Get请求 */
	bool HandleGet(const std::string&, std::shared_ptr<CHttpConnection>);
	bool HandlePost(const std::string&, std::shared_ptr<CHttpConnection>);

	/* 注册Get请求的键值对 */
	void RegGet(const std::string&, HttpHandler handler);
	void RegPost(const std::string&, HttpHandler handler);


private:

	SLogicSystem();
	void RegFuncs();//注册请求方法的键值对
	bool ReqPostRegister(std::shared_ptr<CHttpConnection> Connection);//回应注册投递
	bool ReqRestPasswd(std::shared_ptr<CHttpConnection> Connection);//回应重置密码
	bool ReqLogin(std::shared_ptr<CHttpConnection> Connection);//回应登录请求

	std::map<const std::string, HttpHandler> PostHandlers; //Post请求的回调,URL映射回调
	std::map<const std::string, HttpHandler> GetHandlers; //Get请求的回调,URL映射回调
};

#endif //
