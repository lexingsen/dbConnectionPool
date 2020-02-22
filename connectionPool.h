#pragma once
#include <string>
#include <queue>
#include <mutex>
#include <atomic>
#include <memory>
#include <functional>
#include <thread>
#include <chrono>
#include <condition_variable>
#include "connection.h"
#include "public.h"
using namespace std;


// 线程安全的单例

class ConnectionPool {
private:
	ConnectionPool(); // 构造函数私有化

	// 运行在独立的线程中 专门负责生产新连接
	void produceConnectionTask();

	bool loadConfigFile();

	void scannerConnectionTask();

	string _ip;                         // mysql ip地址
	unsigned short _port;               // 端口
	string _username;                   // 登录用户名
	string _password;					          // 登录密码
	string _dbname;                     // 数据库名
 
	int _initSize;                      // 连接池的初始连接量
	int _maxSize;                       // 连接池的最大连接量
	int _maxIdleTime;                   // 连接池的最大空闲时间
	int _connectionTimeOut;             // 连接池获取连接的超时时间

	
	queue<Connection*> _connectionQue; 	// 存储连接池的队列
	mutex _queueMutex; 									// 维护连接队列的线程安全的互斥锁
	atomic_int _connectionCnt; 					// 记录连接所创建的Connection连接的总数量
	condition_variable _cv; 							// 条件变量，用于连接生产者和消费者线程之间的通信
public:
	// 获取连接池对象实例
	static ConnectionPool* getConnectionPool();

	shared_ptr<Connection> getConnection();
};
