#pragma once

#include <mysql.h>
#include <mutex>

using namespace std;

class MySQL {
private:
	MySQL();
	~MySQL();
	
public:
	static MySQL *getInstance();

	int query(const char *str);
	MYSQL_RES *storeResult();
	MYSQL_ROW fetchRow();
	my_ulonglong getRowNum();
	void freeResult();

private:
	MYSQL mysql;
	MYSQL_ROW row;
	MYSQL_RES *res;
	
	mutex mtx;
};