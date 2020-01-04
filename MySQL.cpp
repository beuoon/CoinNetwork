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
bool MySQL::query_result(const char *str) {
	mtx.lock();
	mysql_query(&mysql, str);
	if ((res = mysql_store_result(&mysql)) == NULL) {
		mtx.unlock();
		return false;
	}
	return true;
}
MYSQL_ROW MySQL::fetchRow() {
	return mysql_fetch_row(res);
}
my_ulonglong MySQL::getRowNum() {
	return mysql_num_rows(res);
}
void MySQL::freeResult() {
	mysql_free_result(res);
	mtx.unlock();
}