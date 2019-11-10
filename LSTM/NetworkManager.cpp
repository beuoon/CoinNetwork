#include "NetworkManager.h"

#include <stdlib.h>

void NetworkManager::load(char *str) { loadStr = str; }
const char *NetworkManager::save() { return saveStr.c_str(); }
	
NetworkManager& NetworkManager::operator>>(int &i) {
	i = strtol(loadStr, &loadStr, 10);
	return *this;
}
NetworkManager& NetworkManager::operator>>(double &d) {
	d = strtod(loadStr, &loadStr);
	return *this;
}

NetworkManager& NetworkManager::operator<<(const char *str) {
	saveStr += str;
	return *this;
}
NetworkManager& NetworkManager::operator<<(const int &i) {
	saveStr += to_string(i);
	return *this;
}
NetworkManager& NetworkManager::operator<<(const double &d) {
	saveStr += to_string(d);
	return *this;
}
	