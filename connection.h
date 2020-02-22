#pragma once
#include <mysql/mysql.h>
#include <string>
#include <ctime>
using namespace std;

// 处理mysql增删改查的类


class Connection {
private:
	MYSQL *_con; // mysql数据库连接句柄

	clock_t _alivetime; // 进入空闲状态之后的存活时间
public:
	Connection();
	~Connection();

	bool connect(string ip, unsigned short port, string username, string password, string dbname);
	bool update(string sql);
	MYSQL_RES* query(string sql);


	void start() {
			_alivetime = clock();
	}

	// 返回存活的时间
	clock_t end() const {
		return clock() - _alivetime;	
	}
};
