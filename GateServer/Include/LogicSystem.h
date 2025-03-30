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
	bool HandleGet(std::string, std::shared_ptr<CHttpConnection>);
	/* 注册Get请求的键值对 */
	void RegGet(std::string, HttpHandler handler);

	void RegPost(std::string, HttpHandler handler);
	bool HandlePost(std::string, std::shared_ptr<CHttpConnection>);

private:

	SLogicSystem();
	void RegFuns();

	std::map<std::string, HttpHandler> PostHandlers; //Post请求的回调,URL映射回调
	std::map<std::string, HttpHandler> GetHandlers; //Get请求的回调,URL映射回调
};

#endif //
