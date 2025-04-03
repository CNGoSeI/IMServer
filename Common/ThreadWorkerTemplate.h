#ifndef THREADWORKERTEMPLATE_H
#define THREADWORKERTEMPLATE_H
#include <queue>
#include <iostream>
#include <functional>
#include <mutex>

template <typename T>
class TThreadWoker {
public:

	virtual ~TThreadWoker();

	static std::unique_ptr<TThreadWoker<T>> CreateWorkThread(
		std::function<T()> InitFun,
		int poolSize = 4
	) {
		auto pool = std::make_unique<TThreadWoker<T>>(poolSize, std::move(InitFun));
		pool->Run();
		return pool;
	}

	T GetWorker()
	{
		std::unique_lock<std::mutex> lock(Mutex);
		Cond.wait(lock, [this]
			{
				if (bStop)
				{
					return true;
				}
				return !Workers.empty();
			});
		//如果停止则直接返回空指针
		if (bStop){return nullptr;}
		auto context = std::move(Workers.front());
		Workers.pop();
		return context;
	}

	/* 使用完成Work之后还回连接池 */
	void ReturnWorker(T worker) {
		std::lock_guard<std::mutex> lock(Mutex);
		if (bStop){return;}

		Workers.push(std::move(worker));
		Cond.notify_one();//还Worker,有余量了，可通知条件变量进行解锁判断
	}

	void Close() {
		bStop = true;
		Cond.notify_all();
	}

	// 手动实现移动构造函数
	TThreadWoker<T>(TThreadWoker<T>&& other) noexcept
		: PoolSize(other.PoolSize),
		bStop(other.bStop.load()),
		Workers(std::move(other.Workers)) {
		// 互斥锁和条件变量不可移动，需重新初始化（保留默认状态）
		other.PoolSize = 0;
		other.bStop = true;
		std::cout << "线程类移动构造被调用" << std::endl;
	}

	explicit TThreadWoker(size_t poolSize, std::function<T()>Init) : PoolSize(poolSize),InitFunc(std::move(Init)) {}

	/**
	 * \brief 创建线程类时应该填一个函数赋值给该成员
	 * 该成员返回的值将move到Workers成员中
	 */
	std::function<T()>InitFunc;

protected:
	//开始启动
	void Run()
	{
		std::cout << "线程类开始运行" << std::endl;
		for (size_t i = 0; i < PoolSize; ++i) {
			Workers.push(InitFunc());
		}
	};

	std::queue<T> Workers;//实际处理逻辑的类型
	std::atomic<bool> bStop{ false };
	size_t PoolSize;
	std::mutex Mutex;
	std::condition_variable Cond;
private:

};

template <typename T>
TThreadWoker<T>::~TThreadWoker()
{
	std::lock_guard<std::mutex> lock(Mutex);
	Close();
	while (!Workers.empty()) {
		Workers.pop();
	}
}

//template <typename T>
//std::unique_ptr<TThreadWoker<T>> CreateWorkThread(std::function<T()> InitFun, int poolSize = 4) {
//	auto pool = std::make_unique<TThreadWoker<T>>(poolSize, std::move(InitFun));
//	pool->Run();
//	return pool;
//}
#endif // THREADWORKERTEMPLATE_H
