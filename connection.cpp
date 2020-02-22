#include "connection.h"
#include "public.h"


// 初始化连接
Connection::Connection() {
	_con = mysql_init(nullptr);
}

// 释放数据库连接资源
Connection::~Connection() {
	if (!_con) mysql_close(_con);
}

// 连接数据库
bool Connection::connect(string ip, unsigned short port, string username, string password, string dbname) {
	MYSQL *p = mysql_real_connect(_con, ip.c_str(), username.c_str(), password.c_str(), dbname.c_str(), port, nullptr, 0);
	return p != nullptr;
}

// 更新
bool Connection::update(string sql) {
	if (mysql_query(_con, sql.c_str())) {
  	LOG("update error:" + sql);
		return false;
	} 
	return true;
}

// 查找
MYSQL_RES* Connection::query(string sql) {
	if (mysql_query(_con, sql.c_str())) {
		LOG("query error:" + sql);
		return nullptr;
	} else {
		return mysql_use_result(_con);
	}
}
