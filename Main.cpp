#include <iostream>
#include <fstream>
#include <cstdio>
#include <ctime>
#include <string>
#include <vector>
#include <unistd.h>
#include <algorithm>
#include <thread>

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

const char *dbHost = "localhost";
const char *dbUser = "root";
const char *dbPW = "root";
const char *dbDB = "bitcoin";

void saveData(bool *bPower) {
	setenv("TZ", "GMT-9:00", 1);
	
	time_t startTime = time(NULL), endTime;
	startTime += -(startTime%60) + 60; // 다음 분부터
	endTime = startTime + 60;
	string startTimeStr = tts(startTime), endTimeStr = tts(endTime);
	
	//＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊//
	int firstNumber, lastNumber, count = 0;
	string firstDatetime, lastDatetime;
	
	MYSQL mysql;
	MYSQL_RES *res;
	MYSQL_ROW row;
	
	mysql_init(&mysql);
	mysql_real_connect(&mysql, dbHost, dbUser, dbPW, dbDB, 0, NULL, 0);
	
	mysql_query(&mysql, "select number from history order by datetime desc limit 1");
	res = mysql_store_result(&mysql);
	row = mysql_fetch_row(res);
	
	if (row == NULL)	firstNumber = 1;
	else				firstNumber = atoi(row[0]) + 1;
	cout << "firstNumber: " << firstNumber << endl;
	firstDatetime = startTimeStr;

	//＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊//
	double prev_trans_ask_min = 0, prev_trans_ask_avrg = 0, prev_trans_ask_unit = 0;
	double prev_trans_bid_max = 0, prev_trans_bid_avrg = 0, prev_trans_bid_unit = 0;
	double prev_order_ask_min = 0, prev_order_ask_avrg = 0, prev_order_bid_max = 0, prev_order_bid_avrg = 0;
	
	vector<Bithumb::Transaction> histories;
	while (true) {
		if (!*bPower) break;
		
		sleep(1); // 1초마다
		time_t fetchTime = time(NULL);
		
		vector<Bithumb::Transaction> newHistories = Bithumb::getTransactionHistory(100, "BTC");
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
		map<string, vector<Bithumb::Order>> orderBook = Bithumb::getOrderBook(30, "BTC");
		vector<Bithumb::Order> askOrder = orderBook["ask"];
		vector<Bithumb::Order> bidOrder = orderBook["bid"];
		
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
			
			firstDatetime = startTimeStr;
			
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
		mysql_query(&mysql, query);
		
		// 학습 데이터
		if (count > 0) {
			sprintf(query, "insert into train_data values(%d, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf)", firstNumber + count,
					trans_ask_min_rate, trans_ask_avrg_rate, trans_ask_unit,
					trans_bid_max_rate, trans_bid_avrg_rate, trans_bid_unit,
					order_ask_min_rate, order_ask_avrg_rate, order_bid_max_rate, order_bid_avrg_rate);
			mysql_query(&mysql, query);
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
	
	// 데이터 간격 전송
	lastNumber = firstNumber + count - 1;
	lastDatetime = tts(endTime - 120);
	
	cout << "lastNumber: " << lastNumber << endl;
	
	if (firstNumber <= lastNumber) {
		char query[1000];
		sprintf(query, "insert into history_info values(%d, %d, '%s', '%s')",
					firstNumber, lastNumber,
					firstDatetime.c_str(), lastDatetime.c_str());
		mysql_query(&mysql, query);
	}
	
	mysql_close(&mysql);
}

void _setTrainData() { // 간격 구분 안되어있음
	MYSQL mysql;
	MYSQL_RES *res;
	MYSQL_ROW row;
	
	mysql_init(&mysql);
	mysql_real_connect(&mysql, dbHost, dbUser, dbPW, dbDB, 0, NULL, 0);
	
	mysql_query(&mysql, "select * from history");
	res = mysql_store_result(&mysql);
	
	double prev_trans_ask_min = 0, prev_trans_ask_avrg = 0, prev_trans_ask_unit = 0;
	double prev_trans_bid_max = 0, prev_trans_bid_avrg = 0, prev_trans_bid_unit = 0;
	double prev_order_ask_min = 0, prev_order_ask_avrg = 0, prev_order_bid_max = 0, prev_order_bid_avrg = 0;
	
	double trans_ask_min = 0, trans_ask_avrg = 0, trans_ask_unit = 0;
	double trans_bid_max = 0, trans_bid_avrg = 0, trans_bid_unit = 0;
	double order_ask_min = 0, order_ask_avrg = 0;
	double order_bid_max = 0, order_bid_avrg = 0;
	
	double trans_ask_min_rate = 1, trans_ask_avrg_rate = 1;
	double trans_bid_max_rate = 1, trans_bid_avrg_rate = 1;
	double order_ask_min_rate = 1, order_ask_avrg_rate = 1;
	double order_bid_max_rate = 1, order_bid_avrg_rate = 1;
	
	row = mysql_fetch_row(res);
	prev_trans_ask_min = atof(row[2]);
	prev_trans_ask_avrg = atof(row[3]);
	prev_trans_ask_unit = atof(row[4]);
	prev_trans_bid_max = atof(row[5]);
	prev_trans_bid_avrg = atof(row[6]);
	prev_trans_bid_unit = atof(row[7]);
	prev_order_ask_min = atof(row[8]);
	prev_order_ask_avrg = atof(row[9]);
	prev_order_bid_max = atof(row[10]);
	prev_order_bid_avrg = atof(row[11]);
	
	int cnt = 1;
	while ((row = mysql_fetch_row(res)) != NULL) {
		trans_ask_min = atof(row[2]);
		trans_ask_avrg = atof(row[3]);
		trans_ask_unit = atof(row[4]);
		trans_bid_max = atof(row[5]);
		trans_bid_avrg = atof(row[6]);
		trans_bid_unit = atof(row[7]);
		order_ask_min = atof(row[8]);
		order_ask_avrg = atof(row[9]);
		order_bid_max = atof(row[10]);
		order_bid_avrg = atof(row[11]);
		
		if (trans_ask_unit != 0) {
			trans_ask_min_rate = trans_ask_min/prev_trans_ask_min;
			trans_ask_avrg_rate = trans_ask_avrg/prev_trans_ask_avrg;
			
			prev_trans_ask_min = trans_ask_min;
			prev_trans_ask_avrg = trans_ask_avrg;
			prev_trans_ask_unit = trans_ask_unit;
		}
		if (trans_bid_unit != 0) {
			trans_bid_max_rate = trans_bid_max/prev_trans_bid_max;
			trans_bid_avrg_rate = trans_bid_avrg/prev_trans_bid_avrg;
			
			prev_trans_bid_max = trans_bid_max;
			prev_trans_bid_avrg = trans_bid_avrg;
			prev_trans_bid_unit = trans_bid_unit;
		}
		
		order_ask_min_rate = order_ask_min/prev_order_ask_min;
		order_ask_avrg_rate = order_ask_avrg/prev_order_ask_avrg;
			
		prev_order_ask_min = order_ask_min;
		prev_order_ask_avrg = order_ask_avrg;
		
		order_bid_max_rate = order_bid_max/prev_order_bid_max;
		order_bid_avrg_rate = order_bid_avrg/prev_order_bid_avrg;
		
		prev_order_bid_max = order_bid_max;
		prev_order_bid_avrg = order_bid_avrg;
	
		char query[1000];
		sprintf(query, "insert into train_data values(%d, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf)", ++cnt,
				trans_ask_min_rate, trans_ask_avrg_rate, trans_ask_unit,
				trans_bid_max_rate, trans_bid_avrg_rate, trans_bid_unit,
				order_ask_min_rate, order_ask_avrg_rate, order_bid_max_rate, order_bid_avrg_rate);
		mysql_query(&mysql, query);
	}
	
	mysql_close(&mysql);
}


class CoinNetwork {
public:
	CoinNetwork() {
		inputNum = 60, outputNum = 30;
		hiddenNum = inputNum + outputNum;
 		inputSize = 10, hiddenSize = 10, outputSize = 1;
		
		network = new LSTM(inputNum, hiddenNum, outputNum, inputSize, hiddenSize, outputSize); // 60분의 데이터를 주면 이후 30분의 데이터를 추측
		lastTrainDataNumber = 1;
		fetchTrainData();
	}
	~CoinNetwork() {
		delete network;
	}
	
	void train() {
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
				
				network->train(input, label);
				
				firstIter++;
				middleIter++;
				lastIter++;
			}
		}
	}
	// void predict();
	
	void fetchTrainData() {
		MYSQL mysql;
		MYSQL_RES *res;
		MYSQL_ROW row;
		
		mysql_init(&mysql);
		mysql_real_connect(&mysql, dbHost, dbUser, dbPW, dbDB, 0, NULL, 0);
		
		char query[1000];
		sprintf(query, "select * from train_data where number > %d", lastTrainDataNumber);
		mysql_query(&mysql, query);
		
		res = mysql_store_result(&mysql);
		
		if (lastTrainDataNumber == 1)
			trainDataArr.push_back(vector<VectorXd>());
		int arrCnt = trainDataArr.size()-1;
		
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
	
		mysql_close(&mysql);
		
		cout << "trainData: " << trainDataArr.size() << endl;
		for (int i = 0; i < trainDataArr.size(); i++)
			cout << " - [" << i << "]: " << trainDataArr[i].size() << endl;
	}

private:
	LSTM *network;
	int inputNum, hiddenNum, outputNum;
	int inputSize, hiddenSize, outputSize;
	
	vector<vector<VectorXd>> trainDataArr;
	int lastTrainDataNumber;
};

int main() {
	CoinNetwork network;
	
	network.train();
	
	cout << "종료" << endl;
	
	/*
	bool bPower_saveData = true;
	
	thread tSaveData(saveData, &bPower_saveData);
	
	// 파일의 유무로 종료하는 루틴
	while (access("./stop", 0) == -1) sleep(1);
	bPower_saveData = false;
	tSaveData.join();
	*/
	
	/*
	srand((unsigned)time(NULL));
	
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
