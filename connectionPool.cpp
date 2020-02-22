#include "connectionPool.h"



ConnectionPool* ConnectionPool::getConnectionPool() {
	static ConnectionPool pool;
	return &pool;
}


bool ConnectionPool::loadConfigFile() {
	FILE *pf = fopen("mysql.conf", "r");
	if (!pf) {
		LOG("mysql.conf file is not exist!\n");
		return false;
	}

	while (!feof(pf)) {
		char line[1024] = {0};
		fgets(line, 1024, pf);
		string str = line;

		int idx = str.find('=', 0);

		// 无效的配置项
		if (idx == -1) continue;
		int endidx = str.find('\n', idx);

		string key = str.substr(0, idx);
		string val = str.substr(idx+1, endidx-idx-1);
		if (key == "ip") {
			_ip = val;
		} else if (key == "port") {
			_port = atoi(val.c_str());
		} else if (key == "username") {
			_username = val;
		} else if (key == "password") {
			_password = val;
		} else if (key == "initSize") {
			_initSize = atoi(val.c_str());
		} else if (key == "maxSize") {
			_maxSize = atoi(val.c_str());
		} else if (key == "maxIdleTime") {
			_maxIdleTime = atoi(val.c_str());
		} else if (key == "connectionTimeOut") {
			_connectionTimeOut = atoi(val.c_str());
		} else if (key == "dbname") {
			_dbname = val;
		}
	}
	return true;
}



ConnectionPool::ConnectionPool() {
	// 加载配置项
	if (!loadConfigFile()) {
		LOG("load cnfigueration error!\n");
		return;
	}

	for (int i=0; i<_initSize; ++i) {
		Connection* p = new Connection();
		p->connect(_ip, _port, _username, _password, _dbname);
		p->start();
		_connectionQue.push(p);
		_connectionCnt ++;
	}

	// 启动一个新的线程  作为连接的生产者
	thread produce(bind(&ConnectionPool::produceConnectionTask, this));
	produce.detach();


	// 启动一个新的定时线程  扫描超过maxIdleTime时间的空闲连接，并对其回收
	thread scanner(bind(&ConnectionPool::scannerConnectionTask, this));
	scanner.detach();
}

void ConnectionPool::produceConnectionTask() {
	for(;;) {
		unique_lock<mutex> lock(_queueMutex);
		while (!_connectionQue.empty()) {
			_cv.wait(lock);
		}

 		// 连接数量没有达到上限 继续创建
		if (_connectionCnt < _maxSize) {
			Connection* p = new Connection();
			p->connect(_ip, _port, _username, _password, _dbname);
			p->start();
			_connectionQue.push(p);
			_connectionCnt ++;	
		}
		
		// 通知消费者线程  可以消费连接了
		_cv.notify_all();
	}
}


// 消费者线程
shared_ptr<Connection> ConnectionPool::getConnection() {
	unique_lock<mutex> lock(_queueMutex);
	while (_connectionQue.empty()) {
		if (cv_status::timeout == _cv.wait_for(lock, chrono::milliseconds(_connectionTimeOut))) {
			if (_connectionQue.empty()) {
				LOG("获取空闲连接超时，获取连接失败!\n");
				return nullptr; 
			}
		}
	}
/*
 *智能指针析构时，会直接把资源delete
 这里需要自定义shared_ptr释放资源的方式， 把connection归还到连接池中
 * */

	shared_ptr<Connection> sp(_connectionQue.front(),
			[&] (Connection *con) {
				unique_lock<mutex> lock(_queueMutex);
				con->start();
				_connectionQue.push(con);
			});

	_connectionQue.pop();
	if (_connectionQue.empty()) {
		// 谁消费了队列中的最后一个连接，同时生产者生产线程
		_cv.notify_all();	
	}
	return sp;
}


void ConnectionPool::scannerConnectionTask() {
	for(;;) {
		// 通过sleep模拟定时效果
		this_thread::sleep_for(chrono::seconds(_maxIdleTime));
		// 扫描队列，释放多余的连接

		unique_lock<mutex> lock(_queueMutex);

		while (_connectionCnt > _initSize) {
			Connection *p = _connectionQue.front();
			if (p->end() > (_maxIdleTime*1000)) {
				_connectionQue.pop();
				_connectionCnt --;
				delete p;
			} else {
				break; //队头连接没有超时 其他连接肯定没有超时
			}
		}
	}
}
