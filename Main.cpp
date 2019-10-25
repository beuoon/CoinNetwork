#include <iostream>
#include <fstream>
#include <cstdio>
#include <ctime>
#include <string>
#include <vector>
#include <unistd.h>
#include <algorithm>
#include <thread>
#include <mutex>

#include <Eigen/Dense>
#include <mysql.h>
#include "LSTM/LSTM.h"
#include "Bithumb.h"
#include "xcoin_api.h"

using namespace Eigen;
using namespace std;

string tts(time_t t) { // Time To String
	char str[250];
	strftime(str, sizeof(str), "%Y-%m-%d %H:%M:%S", localtime(&t));
	return str;
}

class MySQL {
public:
	MySQL() {
		mysql_init(&mysql);
		mysql_real_connect(&mysql, "localhost", "root", "root", "bitcoin", 0, NULL, 0);
	}
	~MySQL() {
		mysql_close(&mysql);
	}
	
	int query(const char *str) {
		lock_guard<mutex> lg(mtx);
		return mysql_query(&mysql, str);
	}
	MYSQL_RES *storeResult() {
		mtx.lock();
		res = mysql_store_result(&mysql);
		return res;
	}
	void freeResult() {
		mysql_free_result(res);
		mtx.unlock();
	}
	

private:
	MYSQL mysql;
	MYSQL_ROW row;
	MYSQL_RES *res;
	
	static mutex mtx;
};
mutex MySQL::mtx;
MySQL mysql;

void saveData(bool *bPower) {
	setenv("TZ", "GMT-9:00", 1);
	
	time_t startTime = time(NULL), endTime;
	startTime += -(startTime%60) + 60; // 다음 분부터
	endTime = startTime + 60;
	string startTimeStr = tts(startTime), endTimeStr = tts(endTime);
	
	//＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊//
	int firstNumber, count = 0;
	
	MYSQL_RES *res;
	MYSQL_ROW row;
	
	
	mysql.query("select number from history order by datetime desc limit 1");
	if ((res = mysql.storeResult()) == NULL) return ;
	if ((row = mysql_fetch_row(res)) == NULL)	firstNumber = 1;
	else										firstNumber = atoi(row[0]) + 1;
	mysql.freeResult();

	//＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊//
	double prev_trans_ask_min = 0, prev_trans_ask_avrg = 0, prev_trans_ask_unit = 0;
	double prev_trans_bid_max = 0, prev_trans_bid_avrg = 0, prev_trans_bid_unit = 0;
	double prev_order_ask_min = 0, prev_order_ask_avrg = 0, prev_order_bid_max = 0, prev_order_bid_avrg = 0;
	
	vector<Bithumb::Transaction> histories;
	while (true) {
		if (!*bPower) break;
		
		sleep(1); // 1초마다
		time_t fetchTime = time(NULL);
		
		vector<Bithumb::Transaction> newHistories;
		Bithumb::getTransactionHistory(100, "BTC", newHistories);
		
		vector<Bithumb::Transaction>::iterator iter;
		
		// 이전 데이터 삭제
		for (iter = newHistories.begin(); iter != newHistories.end() && iter->date < startTimeStr; iter = newHistories.erase(iter));
		
		if (iter != newHistories.end()) {
			// 이미 있는 데이터 삭제
			vector<Bithumb::Transaction>::iterator equIter = find(histories.begin(), histories.end(), *iter);
			while (equIter != histories.end()) {
				if (*equIter == *iter)
					iter = newHistories.erase(iter);
				equIter++;
			}
		
			// 데이터 축적
			for (int i = 0; i < newHistories.size(); i++) {
				if (newHistories[i].date >= endTimeStr) break;
				
				histories.push_back(newHistories[i]);
			}
		}
		
		if (endTime > fetchTime) continue;
		
		//＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊//
		// 데이터 전처리
		double trans_ask_min = 0, trans_ask_avrg = 0, trans_ask_unit = 0;
		double trans_bid_max = 0, trans_bid_avrg = 0, trans_bid_unit = 0;
		
		// 거래 내역
		for (iter = histories.begin(); iter != histories.end(); iter = histories.erase(iter)) {
			if (iter->type == Bithumb::TransactionType::ASK) {
				if (trans_ask_min == 0 || trans_ask_min > iter->price)
					trans_ask_min = iter->price;
				trans_ask_avrg += iter->total;
				trans_ask_unit += iter->unit;
			}
			else {
				if (trans_bid_max == 0 || trans_bid_max < iter->price)
					trans_bid_max = iter->price;
				trans_bid_avrg += iter->total;
				trans_bid_unit += iter->unit;
			}
		}
		if (trans_ask_unit != 0) trans_ask_avrg /= trans_ask_unit;
		if (trans_bid_unit != 0) trans_bid_avrg /= trans_bid_unit;
		
		// 주문 내역
		map<string, vector<Bithumb::Order>> orderBook;
		vector<Bithumb::Order> askOrder;
		vector<Bithumb::Order> bidOrder;
		if (Bithumb::getOrderBook(30, "BTC", orderBook)) {
			askOrder = orderBook["ask"];
			bidOrder = orderBook["bid"];
		}
		
		double order_ask_min = 0, order_ask_avrg = 0, order_ask_unit = 0;
		double order_bid_max = 0, order_bid_avrg = 0, order_bid_unit = 0;
		
		for (int i = 0; i < askOrder.size(); i++) {
			if (order_ask_min == 0 || order_ask_min > askOrder[i].price)
				order_ask_min = askOrder[i].price;
			order_ask_avrg += askOrder[i].price * askOrder[i].unit;
			order_ask_unit += askOrder[i].unit;
		}
		if (order_ask_unit != 0) order_ask_avrg /= order_ask_unit;
		
		for (int i = 0; i < bidOrder.size(); i++) {
			if (order_bid_max == 0 || order_bid_max < bidOrder[i].price)
				order_bid_max = bidOrder[i].price;
			order_bid_avrg += bidOrder[i].price * bidOrder[i].unit;
			order_bid_unit += bidOrder[i].unit;
		}
		if (order_bid_unit != 0) order_bid_avrg /= order_bid_unit;
		
		// 예외처리
		if (count == 0 && (trans_ask_unit == 0 || trans_bid_unit == 0 || order_ask_unit == 0 || order_bid_unit == 0)) {
			startTimeStr = endTimeStr;
			endTime += 60;
			endTimeStr = tts(endTime);
			
			continue;
		}
		
		// 변화량 측정
		double trans_ask_min_rate = 1, trans_ask_avrg_rate = 1;
		double trans_bid_max_rate = 1, trans_bid_avrg_rate = 1;
		double order_ask_min_rate = 1, order_ask_avrg_rate = 1;
		double order_bid_max_rate = 1, order_bid_avrg_rate = 1;
		
		if (count > 0) {
			if (trans_ask_unit != 0) {
				trans_ask_min_rate = trans_ask_min/prev_trans_ask_min;
				trans_ask_avrg_rate = trans_ask_avrg/prev_trans_ask_avrg;
			}
			if (trans_bid_unit != 0) {
				trans_bid_max_rate = trans_bid_max/prev_trans_bid_max;
				trans_bid_avrg_rate = trans_bid_avrg/prev_trans_bid_avrg;
			}
			if (order_ask_unit != 0) {
				order_ask_min_rate = order_ask_min/prev_order_ask_min;
				order_ask_avrg_rate = order_ask_avrg/prev_order_ask_avrg;
			}
			if (order_bid_unit != 0) {
				order_bid_max_rate = order_bid_max/prev_order_bid_max;
				order_bid_avrg_rate = order_bid_avrg/prev_order_bid_avrg;
			}
		}
		
		//＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊//
		// DB 전송
		cout << startTimeStr << " ~ " << endTimeStr << endl;
		
		char query[1000];
		sprintf(query, "insert into history values(NULL, '%s', %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf)", startTimeStr.c_str(),
				trans_ask_min, trans_ask_avrg, trans_ask_unit,
				trans_bid_max, trans_bid_avrg, trans_bid_unit,
				order_ask_min, order_ask_avrg, order_bid_max, order_bid_avrg);
		mysql.query(query);
		
		// 학습 데이터
		if (count > 0) {
			sprintf(query, "insert into train_data values(%d, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf)", firstNumber + count,
					trans_ask_min_rate, trans_ask_avrg_rate, trans_ask_unit,
					trans_bid_max_rate, trans_bid_avrg_rate, trans_bid_unit,
					order_ask_min_rate, order_ask_avrg_rate, order_bid_max_rate, order_bid_avrg_rate);
			mysql.query(query);
		}
		
		//＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊//
		// 데이터 저장
		if (trans_ask_unit != 0) {
			prev_trans_ask_min = trans_ask_min;
			prev_trans_ask_avrg = trans_ask_avrg;
			prev_trans_ask_unit = trans_ask_unit;
		}		
		if (trans_bid_unit != 0) {
			prev_trans_bid_max = trans_bid_max;
			prev_trans_bid_avrg = trans_bid_avrg;
			prev_trans_bid_unit = trans_bid_unit;
		}
		if (order_ask_unit != 0) {
			prev_order_ask_min = order_ask_min;
			prev_order_ask_avrg = order_ask_avrg;
		}
		if (order_bid_unit != 0) {
			prev_order_bid_max = order_bid_max;
			prev_order_bid_avrg = order_bid_avrg;
		}
		
		startTimeStr = endTimeStr;
		endTime += 60;
		endTimeStr = tts(endTime);
		
		count++;
	}
}

class CoinNetwork {
public:
	CoinNetwork() {
		inputNum = 60, outputNum = 30;
		hiddenNum = inputNum + outputNum;
 		inputSize = 10, hiddenSize = 10, outputSize = 1;
		
		network = new LSTM(inputNum, hiddenNum, outputNum, inputSize, hiddenSize, outputSize); // 60분의 데이터를 주면 이후 30분의 데이터를 추측
		trainDataNum = 1000;
		
		bLoop = true;
	}
	~CoinNetwork() {
		delete network;
	}
	
	void loop() {
		loadNetwork();
		cout << "network loop start" << endl;
		time_t fetchTime = time(NULL);
		fetchTrainData();
		
		while (bLoop) {
			double error = train();
			cout << "Error: " << error << endl;
			sleep(1);
			
			if (time(NULL) - fetchTime >= 1800) { // 30분마다 학습 데이터 변경
				fetchTrainData();
				fetchTime = time(NULL);
			}
		}
		
		cout << "network loop end" << endl;
		saveNetwork();
	}
	void switchLoop(bool _bLoop) { bLoop = _bLoop; }
	
	// void predict();
	double train() {
		double totalError = 0;
		int cnt = 0;
		
		for (int i = 0; i < trainDataArr.size(); i++) {
			if (trainDataArr[i].size() < inputNum+outputNum) continue;
			
			vector<VectorXd>::iterator firstIter = trainDataArr[i].begin();
			vector<VectorXd>::iterator middleIter = firstIter + inputNum;
			vector<VectorXd>::iterator lastIter = middleIter + outputNum;
			
			while (lastIter != trainDataArr[i].end()) {
				vector<VectorXd> input(firstIter, middleIter);
				vector<VectorXd> label;
				for (auto iter = middleIter; iter != lastIter; iter++) {
					VectorXd data(1); data << (*iter)(0); // trans_ask_min_rate
					label.push_back(data);
				}
				
				totalError += network->train(input, label);
				cnt++;
				
				firstIter++;
				middleIter++;
				lastIter++;
			}
		}
		
		return totalError;
	}
	
	void fetchTrainData() {
		MYSQL_RES *res;
		MYSQL_ROW row;
		
		mysql.query("select number from train_data order by number desc limit 1");
		if ((res = mysql.storeResult()) == NULL) return;
		if ((row = mysql_fetch_row(res)) == NULL) return;
		int number = atoi(row[0]);
		
		mysql.freeResult();
		
		number = rand() % (number - trainDataNum);
		
		char query[1000];
		sprintf(query, "select * from train_data where number >= %d limit %d", number, trainDataNum);
		mysql.query(query);
		
		if ((res = mysql.storeResult()) == NULL) return;
		
		trainDataArr.clear();
		trainDataArr.push_back(vector<VectorXd>());
		
		int arrCnt = 0;
		int lastTrainDataNumber = 1;
		
		while ((row = mysql_fetch_row(res)) != NULL) {
			int number = atoi(row[0]);
			if (lastTrainDataNumber+1 < number) {
				trainDataArr.push_back(vector<VectorXd>());
				arrCnt++;
			}
			lastTrainDataNumber = number;
			
			VectorXd data(inputSize);
			for (int i = 0; i < inputSize; i++)
				data(i) = atof(row[i+2]);
			trainDataArr[arrCnt].push_back(data);
		}
	
		mysql.freeResult();
	}
	
	void saveNetwork() {
		NetworkManager mgr;
		mgr << *network;
		const char *str = mgr.save();
		
		// DB 저장
		char query[200000];
		sprintf(query, "insert into network values(NULL, '%s', '%s')", tts(time(NULL)).c_str(), str);
		mysql.query(query);
	}
	void loadNetwork() {
		// DB 가져오기
		MYSQL_RES *res;
		MYSQL_ROW row;
		mysql.query("select data from network order by id desc limit 1");
		
		if ((res = mysql.storeResult()) == NULL) return;
		if ((row = mysql_fetch_row(res)) == NULL) return;
		char *str = row[0];
		
		NetworkManager mgr;
		mgr.load(str);
		
		if (network != nullptr)
			delete network;
		network = new LSTM(mgr);
		
		mysql.freeResult();
	}

private:
	LSTM *network;
	int inputNum, hiddenNum, outputNum;
	int inputSize, hiddenSize, outputSize;
	
	vector<vector<VectorXd>> trainDataArr;
	int trainDataNum;
	
	bool bLoop;
};


int main() {
	cout << "시작" << endl;
	srand((unsigned)time(NULL));
	
	CoinNetwork network;
	bool bPower_saveData = true;
	
	thread tSaveData(saveData, &bPower_saveData);
	thread tNetwork(&CoinNetwork::loop, &network);
	
	// 파일의 유무로 종료하는 루틴
	while (access("./stop", 0) == -1) sleep(1);
	
	bPower_saveData = false;
	network.switchLoop(false);
	tSaveData.join();
	tNetwork.join();
	
	cout << "종료" << endl;
	
	/*
	
	int inputSize = 1, outputSize = 1;
	int inputNum = 3, outputNum = 1;
	
	double inputArr[] = {0.01, 0.02, 0.03, 0.04, 0.05, 0.06};
	double labelArr[] = {0.01, 0.07, 0.08, 0.07};
	int dataNum = 4;
	vector<VectorXd> inputs, labels;
	for (int i = 0; i < 6; i++) {
		VectorXd input(inputSize); input << inputArr[i];
		inputs.push_back(input);
	}
	for (int i = 0; i < 4; i++) {
		VectorXd label(outputSize); label << labelArr[i];
		labels.push_back(label);
	}
	
	LSTM network(inputNum, 10, outputNum, inputSize, 10, outputSize);
	
	cout << "Before: " << endl;
	for (int j = 0; j < dataNum; j++) {
		vector<VectorXd> input(inputs.begin()+j, inputs.begin()+j+inputNum);
		cout << network.predict(input)[0] << endl;
	}
	
	for (int i = 0; i < 1000; i++) {
		for (int j = 0; j < dataNum; j++) {
			vector<VectorXd> input(inputs.begin()+j, inputs.begin()+j+inputNum);
			vector<VectorXd> label(labels.begin()+j, labels.begin()+j+outputNum);
			network.train(input, label);
		}
	}
	
	cout << "After: " << endl;
	for (int j = 0; j < dataNum; j++) {
		vector<VectorXd> input(inputs.begin()+j, inputs.begin()+j+inputNum);
		cout << network.predict(input)[0] << endl;
	}
	*/
	
	/*
	ifstream is("network");
	LSTM network(is);
	is.close();
	
	VectorXd inputData(6); inputData << 0.1, 0.2, 0.3, 0.4, 0.5, 0.6;
	VectorXd labelData(6); labelData << 0.1, 0.2, 0.3, 0.4, 0.5, 0.6;
	vector<VectorXd> input, label;
	input.push_back(inputData);
	label.push_back(labelData);
	
	cout << "Before" << endl;
	cout << network.predict(input)[0].transpose() << endl;
	
	for (int i = 0; i < 10000; i++)
		network.train(input, label);
	
	cout << "After" << endl;
	cout << network.predict(input)[0].transpose() << endl;
	*/
	
	return 0;
}
