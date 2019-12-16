#pragma once

#include <string>
#include <sstream>

using namespace std;

class NetworkManager {
public:
	void load(string str);
	string save();
	void cross(string str1, string str2);
	
	NetworkManager& operator>>(int &i);
	NetworkManager& operator>>(double &d);
	
	NetworkManager& operator<<(const char *str);
	NetworkManager& operator<<(const int &i);
	NetworkManager& operator<<(const double &d);

private:
	string networkStr;
	stringstream stream;
};