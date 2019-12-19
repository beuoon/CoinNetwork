#include "NetworkManager.h"

void NetworkManager::load(string str) {
	stream = stringstream(str);
}
string NetworkManager::save() { return networkStr; }

NetworkManager& NetworkManager::operator>>(int &i) {
	string token;
	if (stream >> token)
		i = atoi(token.c_str());
	return *this;
}
NetworkManager& NetworkManager::operator>>(double &d) {
	string token;
	if (stream >> token)
		d = atof(token.c_str());
	return *this;
}

NetworkManager& NetworkManager::operator<<(const char *str) {
	networkStr += str;
	return *this;
}
NetworkManager& NetworkManager::operator<<(const int &i) {
	networkStr += to_string(i);
	return *this;
}
NetworkManager& NetworkManager::operator<<(const double &d) {
	networkStr += to_string(d);
	return *this;
}