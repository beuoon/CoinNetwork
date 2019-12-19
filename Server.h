#include <iostream>
#include <cstdio>
#include <cstring>
#include <thread>

#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>

#include "DataSaver.h"
#include "CoinManager.h"

using namespace rapidjson;
using namespace std;

class Server {
public:
	int open();
	void close();
	
	void loop();
private:
	enum class ErrorType {
		INVALID_STRING,
		NO_METHOD,
		INVALID_METHOD,
		INVALID_DATA,
		INSUFFICIENT_HISTORY
	};
	string error(ErrorType et);
	string control(Document& rd);
	string status(Document &rd);
	string predict(Document &rd);

private:
	int server_socket;
	
	const int PORT = 4000;
	const int CLIENT_MAX_NUM = 50;
	const int BUFF_SIZE = 500;
	
	DataSaver dataSaver;
	CoinManager coinMgr;
};
