#include <iostream>
#include <cstdio>
#include <cstring>
#include <thread>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "DataSaver.h"
#include "CoinManager.h"

using namespace std;

class Server {
public:
	int open() {
		server_socket = socket(PF_INET, SOCK_STREAM, 0);
		if (-1 == server_socket) {
			printf("server socket 생성 실패");
			return 1;
		}
		
		struct sockaddr_in server_addr;
		memset(&server_addr, 0, sizeof(server_addr));
		server_addr.sin_family     = AF_INET;
		server_addr.sin_port       = htons(PORT);
		server_addr.sin_addr.s_addr= htonl(INADDR_ANY);
		
		if (-1 == bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr))) {
			printf("bind() 실행 에러\n");
			return 2;
		}
		
		if (-1 == listen(server_socket, CLIENT_MAX_NUM)) {
			printf( "대기상태 모드 설정 실패n");
			return 3;
		}
		
		return 0;
	}
	void close() {
		::close(server_socket);
	}
	
	void loop() {
		struct sockaddr_in client_addr;
		socklen_t client_addr_size = sizeof(client_addr);
		int client_socket;
		char sendBuff[BUFF_SIZE], recvBuff[BUFF_SIZE];
		
		// Thread Start
		DataSaver dataSaver;
		CoinManager coinMgr;
		
		thread tDataSaver(&DataSaver::loop, &dataSaver);
		thread tNetwork(&CoinManager::loop, &coinMgr);
		
		// Process Message
		printf("서버 시작\n");
		do {
			client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_size);
			if (-1 == client_socket) {
				printf("클라이언트 연결 수락 실패\n");
				continue;
			}
			printf("클라이언트 연결\n");
			
			memset(recvBuff, 0, sizeof(recvBuff));
			int len = read(client_socket, recvBuff, BUFF_SIZE);
			printf("수신: %s\n", recvBuff);
			
			strcpy(sendBuff, "ok");
			write(client_socket, sendBuff, strlen(sendBuff)+1);
			usleep(1000);
			
			::close(client_socket);
			printf("클라이언트 연결 종료\n");
		} while(strcmp(recvBuff, "end"));
		
		printf("서버 종료\n");
	
		// Thread Finish
		dataSaver.switchLoop(false);
		coinMgr.switchLoop(false);
		
		tDataSaver.join();
		tNetwork.join();
	}

private:
	int server_socket;
	
	const int PORT = 4000;
	const int CLIENT_MAX_NUM = 1;
	const int BUFF_SIZE = 500;
};


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
