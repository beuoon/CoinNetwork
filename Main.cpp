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
		char recvBuff[BUFF_SIZE];
		
		// Thread Start
		dataSaver.switchLoop(true);
		coinMgr.switchLoop(true);
		
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
			
			
			//＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊//
			// 수신
			memset(recvBuff, 0, sizeof(recvBuff));
			int len = read(client_socket, recvBuff, BUFF_SIZE);
			// printf("수신: %s\n", recvBuff);
			
			Document document;
			document.Parse(recvBuff);
			
			string result;
			
			if (!document.IsObject()) // json이 아님
				result = error(ErrorType::INVALID_STRING);
			else if (!document.HasMember("method") || !document["method"].IsString()) // method가 없음
				result = error(ErrorType::NO_METHOD);
			else {
				const char *method = document["method"].GetString();
				if (!strcmp(method, "control")) // 기능 조작
					result = control(document);
				else if (!strcmp(method, "status")) // 작동 상태
					result = status(document);
				else if (!strcmp(method, "predict")) // 예측
					result = predict(document);
				else // 잘못된 method
					result = error(ErrorType::INVALID_METHOD);
			}
			
			//＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊//
			// 송신
			const char *sendStr = result.c_str();
			char *sendBuff = new char[strlen(sendStr)+1];
			strcpy(sendBuff, sendStr);
			
			// cout << "송신(" << strlen(sendBuff) << "): " << sendBuff << endl;
			write(client_socket, sendBuff, strlen(sendBuff)+1);
			
			delete[] sendBuff;
			
			//＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊//
			usleep(1000);
			
			::close(client_socket);
			printf("클라이언트 연결 종료\n");
		} while(true);
		
		printf("서버 종료\n");
	
		// Thread Finish
		dataSaver.switchLoop(false);
		coinMgr.switchLoop(false);
		
		tDataSaver.join();
		tNetwork.join();
	}
private:
	enum class ErrorType {
		INVALID_STRING,
		NO_METHOD,
		INVALID_METHOD,
		INVALID_DATA,
		INSUFFICIENT_HISTORY
	};
	string error(ErrorType et) {
		//＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊//
		// 송신 메세지
		Document sd;
		sd.SetObject();

		Document::AllocatorType& allocator = sd.GetAllocator();
		
		Value val(kObjectType);
		
		string status = "failed";
		val.SetString(status.c_str(), static_cast<SizeType>(status.length()), allocator);
		sd.AddMember("status", val, allocator);
		
		string data;
		switch (et) {
		case ErrorType::INVALID_STRING:
			data = "invalid string: it's not json object";
			break;
		case ErrorType::NO_METHOD:
			data = "no method: it needs method";
			break;
		case ErrorType::INVALID_METHOD:
			data = "invalid method: this method isn't supported";
			break;
		case ErrorType::INVALID_DATA:
			data = "invalid data";
			break;
		case ErrorType::INSUFFICIENT_HISTORY:
			data = "insufficient history";
			break;
		}
		
		val.SetString(data.c_str(), static_cast<SizeType>(data.length()), allocator);
		sd.AddMember("data", val, allocator);

		// Convert JSON document to string
		rapidjson::StringBuffer strbuf;
		rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(strbuf);
		sd.Accept(writer);
		
		string str(strbuf.GetString());
		return str;
	}
	string control(Document& rd) {
		//＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊//
		// 수신 메세지
		Value rData = rd["data"].GetObject();
		
		const char* func = rData["func"].GetString();
		const char* power = rData["power"].GetString();
		
		if (!strcmp(func, "train")) { // 학습
			if (!strcmp(power, "on")) {
				coinMgr.switchTrain(true);
				cout << "학습 재개" << endl;
			}
			else if (!strcmp(power, "off")) {
				coinMgr.switchTrain(false);
				cout << "학습 중지" << endl;
			}
		}
		else
			return error(ErrorType::INVALID_DATA);
		
		//＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊//
		// 송신 메세지
		Document sd;
		sd.SetObject();

		Document::AllocatorType& allocator = sd.GetAllocator();

		Value val(kObjectType);
		
		string status = "succeed";
		val.SetString(status.c_str(), static_cast<SizeType>(status.length()), allocator);
		sd.AddMember("status", val, allocator);

		// Convert JSON document to string
		rapidjson::StringBuffer strbuf;
		rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(strbuf);
		sd.Accept(writer);
		
		string str(strbuf.GetString());
		return str;
	}
	string status(Document &rd) {
		//＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊//
		// 송신 메세지
		Document sd;
		sd.SetObject();

		Document::AllocatorType& allocator = sd.GetAllocator();

		Value obj(kObjectType);
		Value val(kObjectType);
		
		string status = "succeed";
		val.SetString(status.c_str(), static_cast<SizeType>(status.length()), allocator);
		sd.AddMember("status", val, allocator);
		
		val.SetBool(coinMgr.getTrainStatus());
		obj.AddMember("train", val, allocator);
		
		sd.AddMember("data", obj, allocator);

		// Convert JSON document to string
		rapidjson::StringBuffer strbuf;
		rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(strbuf);
		sd.Accept(writer);
		
		string str(strbuf.GetString());
		return str;
	}
	string predict(Document &rd) {
		//＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊//
		// 수신 메세지
		Value rData = rd["data"].GetObject();
		
		const char* datetime = rData["datetime"].GetString(); // datetime부터 30분간의 데이터를 예측 요청
		
		struct tm tm;
		time_t t;
		strptime(datetime, "%Y-%m-%d %H:%M:%S", &tm);
		t = mktime(&tm);
		
		t -= (t%60); // 초 단위 없애기
		t -= 3600; // 한시간전
		string startTime = tts(t);
		
		vector<double> pr;
		if (!coinMgr.predict(startTime, pr))
			return error(ErrorType::INSUFFICIENT_HISTORY);
		
		//＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊//
		// 송신 메세지
		Document sd;
		sd.SetObject();

		Document::AllocatorType& allocator = sd.GetAllocator();
		
		Value data(kArrayType);
		Value val(kObjectType);
		
		string status = "succeed";
		val.SetString(status.c_str(), static_cast<SizeType>(status.length()), allocator);
		sd.AddMember("status", val, allocator);
		
		for (int i = 0; i < pr.size(); i++) {
			Value obj(kObjectType);
			obj.AddMember("rate", pr[i], allocator);
			data.PushBack(obj, allocator);
		}
		sd.AddMember("data", data, allocator);

		// Convert JSON document to string
		rapidjson::StringBuffer strbuf;
		rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(strbuf);
		sd.Accept(writer);
		
		string str(strbuf.GetString());
		return str;
	}

private:
	int server_socket;
	
	const int PORT = 4000;
	const int CLIENT_MAX_NUM = 5;
	const int BUFF_SIZE = 500;
	
	DataSaver dataSaver;
	CoinManager coinMgr;
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
