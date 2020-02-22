main:main.o connection.o connectionPool.o
	g++ main.o connection.o connectionPool.o -o main -g -L /usr/lib/mysql -lmysqlclient -std=c++11 -lpthread
main.o:main.cpp
	g++ main.cpp -c -g
connection.o:connection.cpp
	g++ connection.cpp -c -g
connectionPool.o:connectionPool.cpp
	g++ connectionPool.cpp -c -g
clean:
	rm *.o -rf
	rm main
