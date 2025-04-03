#include <iostream>
#include <queue>
#include<sw/redis++/redis++.h>
#include "RedisMgr.h"
#include "LlfRedisMgr.h"
#include "MysqlDAO.h"


// 并发测试参数配置
constexpr int THREAD_NUM = 64;     // 并发线程数
constexpr int OPS_PER_THREAD = 256;// 每个线程操作次数
constexpr int KEY_POOL_SIZE = 1000;// 键空间分散度

// 原子计数器用于统计
std::atomic<int> total_success(0);
std::atomic<int> total_failure(0);

void ConcurrentTestLlfWorker(int thread_id) {
    // 生成线程专属键名前缀避免冲突
    const std::string prefix = "concurrent_test_" + std::to_string(thread_id) + "_";

    try {
        for (int i = 0; i < OPS_PER_THREAD; ++i) {
            // 分散键名空间防止热点
            const std::string key = prefix + std::to_string(i % KEY_POOL_SIZE);
            const std::string value = "val_" + std::to_string(i);

            // 混合操作序列
            bool success = true;
            std::string result;

            // 基础键值操作测试
            success &= LlfRedisMgr::GetInstance().Set(key, value);
            success &= LlfRedisMgr::GetInstance().Get(key, result);
            success &= (result == value);
            // 哈希表操作测试
            success &= LlfRedisMgr::GetInstance().HSet(key + "_hash", "field", value);
            success &= (LlfRedisMgr::GetInstance().HGet(key + "_hash", "field") == value);

            // 列表操作测试
            success &= LlfRedisMgr::GetInstance().LPush(key + "_list", value);
            success &= LlfRedisMgr::GetInstance().RPop(key + "_list", result);
            success &= (result == value);

            // 原子计数器更新
            success ? ++total_success : ++total_failure;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Thread " << thread_id << " failed: " << e.what() << std::endl;
        ++total_failure;
    }
}

void ConcurrentTestWorker(int thread_id) {
    // 生成线程专属键名前缀避免冲突
    const std::string prefix = "concurrent_test_" + std::to_string(thread_id) + "_";

    try {
        for (int i = 0; i < OPS_PER_THREAD; ++i) {
            // 分散键名空间防止热点
            const std::string key = prefix + std::to_string(i % KEY_POOL_SIZE);
            const std::string value = "val_" + std::to_string(i);

            // 混合操作序列
            bool success = true;
            std::string result;

            // 基础键值操作测试
            success &= SRedisMgr::GetInstance().Set(key, value);
            success &= SRedisMgr::GetInstance().Get(key, result);
            success &= (result == value);

            // 哈希表操作测试
            success &= SRedisMgr::GetInstance().HSet(key + "_hash", "field", value);
            success &= (SRedisMgr::GetInstance().HGet(key + "_hash", "field") == value);

            // 列表操作测试
            success &= SRedisMgr::GetInstance().LPush(key + "_list", value);
            success &= SRedisMgr::GetInstance().RPop(key + "_list", result);
            success &= (result == value);

            // 原子计数器更新
            success ? ++total_success : ++total_failure;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Thread " << thread_id << " failed: " << e.what() << std::endl;
        ++total_failure;
    }
}

void RunConcurrentTest() {
    using namespace std::chrono;
    auto start = high_resolution_clock::now();

    std::vector<std::thread> threads;
    threads.reserve(THREAD_NUM);

    // 启动并发线程
    for (int i = 0; i < THREAD_NUM; ++i) {
        threads.emplace_back(ConcurrentTestWorker, i);
    }

    // 等待所有线程完成
    for (auto& t : threads) {
        t.join();
    }

    // 计算性能指标
    auto duration = duration_cast<milliseconds>(high_resolution_clock::now() - start);
    int total_ops = THREAD_NUM * OPS_PER_THREAD;
    double qps = total_ops / (duration.count() / 1000.0);

    // 输出测试报告
    std::cout << "\n======= 并发测试报告 =======\n"
        << "线程数量: " << THREAD_NUM << "\n"
        << "总操作量: " << total_ops << "\n"
        << "成功次数: " << total_success << "\n"
        << "失败次数: " << total_failure << "\n"
        << "耗时: " << duration.count() << "ms\n"
        << "QPS: " << qps << "/s\n"
        << "==========================\n";

    // 最终断言验证
    assert(total_failure == 0);
    assert(total_success == total_ops);
}

void TestRedisMgr() {

	assert(SRedisMgr::GetInstance().Set("blogwebsite", "llfc.club"));
	std::string value = "";
	assert(SRedisMgr::GetInstance().Get("blogwebsite", value));
	assert(SRedisMgr::GetInstance().Get("nonekey", value) == false);
	assert(SRedisMgr::GetInstance().HSet("bloginfo", "blogwebsite", "llfc.club"));
	assert(SRedisMgr::GetInstance().HGet("bloginfo", "blogwebsite") != "");
	assert(SRedisMgr::GetInstance().ExistsKey("bloginfo"));
	assert(SRedisMgr::GetInstance().Del("bloginfo"));
	assert(SRedisMgr::GetInstance().Del("bloginfo"));
	assert(SRedisMgr::GetInstance().ExistsKey("bloginfo") == false);
	assert(SRedisMgr::GetInstance().LPush("lpushkey1", "lpushvalue1"));
	assert(SRedisMgr::GetInstance().LPush("lpushkey1", "lpushvalue2"));
	assert(SRedisMgr::GetInstance().LPush("lpushkey1", "lpushvalue3"));
	assert(SRedisMgr::GetInstance().RPop("lpushkey1", value));
	assert(SRedisMgr::GetInstance().RPop("lpushkey1", value));
	assert(SRedisMgr::GetInstance().LPop("lpushkey1", value));
	assert(SRedisMgr::GetInstance().LPop("lpushkey2", value) == false);
}

void SqlTest()
{
    try {
        // 获取 MySQL 单例
        auto& mysqlDao = SMysqlDao::GetInstance();

        /***************************************
         * 测试连接池和基础功能
         ​***************************************/
         // 1. 获取连接池对象

        // 2. 创建临时测试库（避免污染正式数据）
        {
            auto con = mysqlDao.SqlPool->GetWorker();
            std::unique_ptr<sql::Statement> stmt(con->createStatement());
            stmt->execute("CREATE DATABASE IF NOT EXISTS test_temp");
            con->setSchema("test_temp"); // 切换至测试库
            mysqlDao.SqlPool->ReturnWorker(std::move(con));
            std::cout << "测试库创建成功" << std::endl;
        }

        // 3. 创建测试表
        {
            auto con = mysqlDao.SqlPool->GetWorker();
            std::unique_ptr<sql::Statement> stmt(con->createStatement());
            stmt->execute(
                "CREATE TABLE IF NOT EXISTS test_table ("
                "id INT AUTO_INCREMENT PRIMARY KEY,"
                "name VARCHAR(50) NOT NULL,"
                "value INT DEFAULT 0)"
            );
            std::cout << "测试表创建成功" << std::endl;
            mysqlDao.SqlPool->ReturnWorker(std::move(con));
        }

        /***************************************
         * CRUD 功能验证
         ​***************************************/
         // 4. 插入数据测试
        int insertId = -1;
        {
            auto con = mysqlDao.SqlPool->GetWorker();
            std::unique_ptr<sql::PreparedStatement> pstmt(
                con->prepareStatement("INSERT INTO test_table (name, value) VALUES (?, ?)")
            );
            pstmt->setString(1, "test_item");
            pstmt->setInt(2, 100);
            pstmt->execute();

            // 获取自增ID验证插入成功
            std::unique_ptr<sql::Statement> stmt(con->createStatement());
            std::unique_ptr<sql::ResultSet> res(stmt->executeQuery("SELECT LAST_INSERT_ID() AS id"));
            if (res->next()) {
                insertId = res->getInt("id");
                std::cout << "插入成功，ID=" << insertId << std::endl;
            }
            mysqlDao.SqlPool->ReturnWorker(std::move(con));
        }

        // 5. 查询数据测试
        {
            auto con = mysqlDao.SqlPool->GetWorker();
            std::unique_ptr<sql::PreparedStatement> pstmt(
                con->prepareStatement("SELECT * FROM test_table WHERE id = ?")
            );
            pstmt->setInt(1, insertId);
            std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

            if (res->next()) {
                std::cout << "查询结果：name=" << res->getString("name")
                    << ", value=" << res->getInt("value") << std::endl;
            }
            else {
                throw std::runtime_error("数据查询失败");
            }
            mysqlDao.SqlPool->ReturnWorker(std::move(con));
        }

        // 6. 更新数据测试
        {
            auto con = mysqlDao.SqlPool->GetWorker();
            std::unique_ptr<sql::PreparedStatement> pstmt(
                con->prepareStatement("UPDATE test_table SET value = ? WHERE id = ?")
            );
            pstmt->setInt(1, 200);
            pstmt->setInt(2, insertId);
            int affectedRows = pstmt->executeUpdate();
            std::cout << "更新影响行数：" << affectedRows << std::endl;
            mysqlDao.SqlPool->ReturnWorker(std::move(con));
        }

        // 7. 删除数据测试
        {
            auto con = mysqlDao.SqlPool->GetWorker();
            std::unique_ptr<sql::PreparedStatement> pstmt(
                con->prepareStatement("DELETE FROM test_table WHERE id = ?")
            );
            pstmt->setInt(1, insertId);
            int affectedRows = pstmt->executeUpdate();
            std::cout << "删除影响行数：" << affectedRows << std::endl;
            mysqlDao.SqlPool->ReturnWorker(std::move(con));
        }

        /***************************************
         * 清理测试环境
         ​***************************************/
        {
            auto con = mysqlDao.SqlPool->GetWorker();
            std::unique_ptr<sql::Statement> stmt(con->createStatement());
            stmt->execute("DROP DATABASE IF EXISTS test_temp");
            std::cout << "测试库清理完成" << std::endl;
            mysqlDao.SqlPool->ReturnWorker(std::move(con));
        }

    }
    catch (const sql::SQLException& e) {
        std::cerr << "MySQL 异常: [" << e.getErrorCode()
            << "] " << e.what() << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "系统异常: " << e.what() << std::endl;
    }
}

int main()
{
	//TestRedisMgr();
    //RunConcurrentTest();
    SqlTest();
	system("pause");
	return 0;
}