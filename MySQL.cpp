#include "MySQL.h"

MySQL::MySQL() {
	mysql_init(&mysql);
	mysql_real_connect(&mysql, "localhost", "root", "root", "bitcoin", 0, NULL, 0);
}
MySQL::~MySQL() {
	mysql_close(&mysql);
}

MySQL *MySQL::getInstance() {
	static MySQL *sql = new MySQL();
	return sql;
}

int MySQL::query(const char *str) {
	lock_guard<mutex> lg(mtx);
	return mysql_query(&mysql, str);
}
MYSQL_RES *MySQL::storeResult() {
	mtx.lock();
	res = mysql_store_result(&mysql);
	return res;
}
void MySQL::freeResult() {
	mysql_free_result(res);
	mtx.unlock();
}