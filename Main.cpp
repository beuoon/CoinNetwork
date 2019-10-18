#include <iostream>
#include <fstream>
#include <cstdio>
#include <fstream>
#include <Eigen/Dense>
#include <ctime>
#include <cmath>
#include <string>
#include <vector>
#include <unistd.h>
#include <mysql.h>
#include <algorithm>

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

int main() {
	setenv("TZ", "GMT-9:00", 1);
	
	MYSQL mysql;
	mysql_init(&mysql);
	mysql_real_connect(&mysql, "localhost", "root", "root", "bitcoin", 0, NULL, 0);
	
	time_t startTime = time(NULL), endTime;
	startTime += -(startTime%60) + 60; // 다음 분부터
	endTime = startTime + 60;
	string startTimeStr = tts(startTime), endTimeStr = tts(endTime);

	vector<Bithumb::Transaction> histories;
	while (true) {
		sleep(1); // 1초마다
		
		vector<Bithumb::Transaction> newHistories = Bithumb::getTransactionHistory(100, "BTC");
		vector<Bithumb::Transaction>::iterator iter;
		
		// 이전 데이터 삭제
		for (iter = newHistories.begin(); iter != newHistories.end() && iter->date < startTimeStr; iter = newHistories.erase(iter));
		
		// 이미 있는 데이터 삭제
		vector<Bithumb::Transaction>::iterator equIter = find(histories.begin(), histories.end(), *iter);
		while (equIter != histories.end()) {
			if (*equIter == *iter)
				iter = newHistories.erase(iter);
			equIter++;
		}
		
		// 데이터 축적
		histories.insert(equIter, iter, newHistories.end());
		
		// 1분이 지나면 DB에 저장
		if (histories.size() == 0 || histories.back().date <= endTimeStr) continue;
		
		// 거래 내역
		double trans_ask_min = 0, trans_ask_avrg = 0, trans_bid_max = 0, trans_bid_avrg = 0;
		double askUnit = 0, bidUnit = 0;
		for (iter = histories.begin(); iter != histories.end(); iter = histories.erase(iter)) {
			if (iter->type == Bithumb::TransactionType::ASK) {
				if (trans_ask_min == 0 || trans_ask_min > iter->price)
					trans_ask_min = iter->price;
				trans_ask_avrg += iter->total;
				askUnit += iter->unit;
			}
			else {
				if (trans_bid_max == 0 || trans_bid_max < iter->price)
					trans_bid_max = iter->price;
				trans_bid_avrg += iter->total;
				bidUnit += iter->unit;
			}
		}
		trans_ask_avrg /= askUnit;
		trans_bid_avrg /= bidUnit;
		
		// 주문 내역
		map<string, vector<Bithumb::Order>> orderBook = Bithumb::getOrderBook(30, "BTC");
		vector<Bithumb::Order> askOrder = orderBook["ask"];
		vector<Bithumb::Order> bidOrder = orderBook["bid"];
		double order_ask_min = 0, order_ask_avrg = 0, order_bid_max, order_bid_avrg = 0;
		askUnit = 0, bidUnit = 0;
		
		for (int i = 0; i < askOrder.size(); i++) {
			if (order_ask_min == 0 || order_ask_min > askOrder[i].price)
				order_ask_min = askOrder[i].price;
			order_ask_avrg += askOrder[i].price * askOrder[i].unit;
			askUnit += askOrder[i].unit;
		}
		order_ask_avrg /= askUnit;
		
		for (int i = 0; i < bidOrder.size(); i++) {
			if (order_bid_max == 0 || order_bid_max < bidOrder[i].price)
				order_bid_max = bidOrder[i].price;
			order_bid_avrg += bidOrder[i].price * bidOrder[i].unit;
			bidUnit += bidOrder[i].unit;
		}
		order_bid_avrg /= bidUnit;
		
		// DB 전송
		cout << startTimeStr << " ~ " << endTimeStr << endl;
		char query[1000];
		sprintf(query, "insert into history_table values('%s', %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf)", startTimeStr.c_str(),
				trans_ask_min, trans_ask_avrg, trans_bid_max, trans_bid_avrg,
				order_ask_min, order_ask_avrg, order_bid_max, order_bid_avrg);
		mysql_query(&mysql, query);
		
		startTimeStr = endTimeStr;
		endTime += 60;
		endTimeStr = tts(endTime);
	}
	
	mysql_close(&mysql);
	
	
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
