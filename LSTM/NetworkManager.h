#pragma once

#include <string>

using namespace std;

class NetworkManager {
public:
	void load(char *str);
	const char *save();
	
	NetworkManager& operator>>(int &i);
	NetworkManager& operator>>(double &d);
	
	NetworkManager& operator<<(const char *str);
	NetworkManager& operator<<(const int &i);
	NetworkManager& operator<<(const double &d);

private:
	char *loadStr;
	string saveStr;
};