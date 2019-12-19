#include "Server.h"

int main() {
	srand((unsigned)time(NULL));
	setenv("TZ", "GMT-9:00", 1);
	
	cout << "시작" << endl;
	
	Server server;
	if (!server.open()) {
		server.loop();
		server.close();
	}
	
	cout << "종료" << endl;
	
	return 0;
}
