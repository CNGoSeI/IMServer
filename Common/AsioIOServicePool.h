#ifndef ASIOIOSERVICEPOOL_H
#define ASIOIOSERVICEPOOL_H

/* IOC链接库池 */

#include <vector>
#include <boost/asio/executor_work_guard.hpp>
#include <boost/asio/io_context.hpp>
#include "SingletonTemplate.h"

class SAsioIOServicePool : public TSingleton<SAsioIOServicePool>
{
	friend TSingleton<SAsioIOServicePool>;

public:
	using IOService = boost::asio::io_context;
	using Work = boost::asio::executor_work_guard<boost::asio::io_context::executor_type>;
	using WorkPtr = std::unique_ptr<Work>;
	~SAsioIOServicePool();

	// 使用 round-robin 的方式返回一个 io_service
	boost::asio::io_context& GetIOService();
	void Stop();

private:
	SAsioIOServicePool(std::size_t size = 2/*std::thread::hardware_concurrency()*/);
	std::vector<IOService> IoServices;
	std::vector<WorkPtr> Works;
	std::vector<std::thread> Threads;
	std::size_t NextIOService; //下次Get的IOC数组标号
};

#endif // ASIOIOSERVICEPOOL_H
