#include <memory>
#include <iostream>
#include "connection.h"
#include "connectionPool.h"
#include "public.h"
using namespace std;

const int SQLSIZE=1024;



#define SIGTHREADLEN 1000
#define MULTITHREADLEN 100


#if 1
int main() {
	clock_t begin = clock();
	ConnectionPool *cp = ConnectionPool::getConnectionPool();
	for (int i=0; i<MULTITHREADLEN; ++i) {
		shared_ptr<Connection> sp = cp->getConnection();
		char sql[SQLSIZE] = {0};
		sprintf(sql, "insert into user(id, name, sex) values(%d, '%s', '%s')", 20,  "zhangsan", "male");
		sp->update(sql);
	}
	clock_t end = clock();
	cout << (end - begin) << "ms" << endl;
	delete cp;
	return 0;
}
#endif


#if 0
int main() {
	clock_t begin = clock();
	for (int i=0; i<SIGTHREADLEN; ++i) {
		Connection con;
		char sql[SQLSIZE] = {0};
		sprintf(sql, "insert into user(id, name, sex) values(%d, '%s', '%s')", 20,  "zhangsan", "male");
		con.connect("127.0.0.1", 3306, "root", "111111", "test");
		con.update(sql);
	}
	clock_t end = clock();
	cout << (end - begin) << "ms" << endl;
	return 0;
}

#endif
