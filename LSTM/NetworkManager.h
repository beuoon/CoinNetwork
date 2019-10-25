#pragma once

#include <string>
#include <stdlib.h>


using namespace std;

class NetworkManager {
public:
	NetworkManager() {
	}
	~NetworkManager() {}
	
	void load(char *str) { loadStr = str; }
	const char *save() { return saveStr.c_str(); }
	
	NetworkManager& operator>>(int &i) {
		i = strtol(loadStr, &loadStr, 10);
		return *this;
	}
	NetworkManager& operator>>(double &d) {
		d = strtod(loadStr, &loadStr);
		return *this;
	}
	
	NetworkManager& operator<<(const char *str) {
		saveStr += str;
		return *this;
	}
	NetworkManager& operator<<(const int &i) {
		saveStr += to_string(i);
		return *this;
	}
	NetworkManager& operator<<(const double &d) {
		saveStr += to_string(d);
		return *this;
	}

private:
	char *loadStr;
	string saveStr;
};