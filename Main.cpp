#include <iostream>
#include <cstdio>
#include <fstream>
#include <Eigen/Dense>
#include <ctime>
#include <cmath>
#include <string>
#include <vector>
#include <unistd.h>

#include "LSTM/LSTM.h"
#include "Bithumb.h"
#include "xcoin_api.h"

using namespace Eigen;
using namespace std;

int main() {
	// cout << api_request((char*)"/public/orderbook/BTC",(char*)"count=30") << endl;
	
	setenv("TZ", "GMT-9:00", 1);
	
	vector<Bithumb::Transaction> minuteHistories;
	
	time_t startTime = time(NULL), endTime;
	time_t prevTime = startTime;
	
	startTime += -(startTime%60) + 60; // 다음 분부터
	endTime = startTime + 60;
	
	char timeStr[250];
	strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", localtime(&startTime));
	cout << timeStr << endl;
	
	while (true) {
		if (time(NULL) - prevTime < 10) { sleep(1); continue; }
		prevTime = time(NULL);
		
		vector<Bithumb::Transaction> histories = Bithumb::getTransactionHistory(100, "BTC");
		if (histories.size() > 0) {
			vector<Bithumb::Transaction>::iterator minIter = minuteHistories.find(histories[0]);
			
			for (minIter )
			if (minIter == minuteHistories.end()) {
				
			}
			
		}
		
		
		if (prevTime >= endTime) {
			startTime = endTime;
			endTime += 60;
		}
		
		break;
	}
	
	
	/*
	setenv("TZ", "GMT-9:00", 1);
	char oneTimeStr[250], twoTimeStr[250];
	time_t t = time(NULL);
	t -= t % 60;
	strftime(oneTimeStr, sizeof(oneTimeStr), "%Y-%m-%d %H:%M:%S", localtime(&t));
	t -= 60;
	strftime(twoTimeStr, sizeof(twoTimeStr), "%Y-%m-%d %H:%M:%S", localtime(&t));
	
	cout << twoTimeStr << " <= ~ < " << oneTimeStr << endl;
	
	
	
	auto histories = Bithumb::getTransactionHistory(100, "BTC");
	
	double bidTotal = 0, askTotal = 0, bidUnit = 0, askUnit = 0;
	int minPriceIndex = -1, maxPriceIndex = -1;
	for (int i = 0; i < histories.size(); i++) {
		if (!(twoTimeStr <= histories[i].date && histories[i].date < oneTimeStr)) continue;
		
		if (histories[i].type == Bithumb::TransactionType::BID) {
			bidUnit += histories[i].unit;
			bidTotal += histories[i].total;
			
			if (maxPriceIndex == -1) maxPriceIndex = i;
			if (histories[i].price > histories[maxPriceIndex].price)
				maxPriceIndex = i;
		}
		else {
			askUnit += histories[i].unit;
			askTotal += histories[i].total;
			
			if (minPriceIndex == -1) minPriceIndex = i;
			if (histories[i].price < histories[minPriceIndex].price)
				minPriceIndex = i;
		}
	}
	int bidAvrg = bidTotal / bidUnit;
	int askAvrg = askTotal / askUnit;
	
	int bidMaxPrice = (maxPriceIndex != -1) ? histories[maxPriceIndex].price : 0;
	int askMinPrice = (minPriceIndex != -1) ? histories[minPriceIndex].price : 0;
	
	printf("구입 유닛: %lf\n", bidUnit);
	printf("구입 평균가: %d\n", bidAvrg);
	printf("구입 최고가: %d\n", bidMaxPrice);
	
	printf("판매 유닛: %lf\n", askUnit);
	printf("판매 평균가: %d\n", askAvrg);
	printf("판매 최저가: %d\n", askMinPrice);
	
	
	
	
	
	
	
	std::map<string, vector<Bithumb::Order>> book = Bithumb::getOrderBook(30, "BTC");
	
	vector<Bithumb::Order> bidBook = book["bid"];
	double bidOrderTotal = 0, bidOrderUnit = 0;
	for (int i = 0; i < bidBook.size(); i++) {
		bidOrderTotal += bidBook[i].unit * bidBook[i].price;
		bidOrderUnit += bidBook[i].unit;
		
		if (bidBook[i].price > bidBook[maxPriceIndex].price)
			maxPriceIndex = i;
	}
	int bidOrderAvrg = bidOrderTotal / bidOrderUnit;
	int bidOrderMaxPrice = bidBook[maxPriceIndex].price;
	printf("구입 대기 평균가: %d\n", bidOrderAvrg);
	printf("구입 대기 최고가: %d\n", bidOrderMaxPrice);
	
	vector<Bithumb::Order> askBook = book["ask"];
	double askOrderTotal = 0, askOrderUnit = 0;
	for (int i = 0; i < askBook.size(); i++) {
		askOrderTotal += askBook[i].unit * askBook[i].price;
		askOrderUnit += askBook[i].unit;
		
		if (askBook[i].price < askBook[minPriceIndex].price)
			minPriceIndex = i;
	}
	int askOrderAvrg = askOrderTotal / askOrderUnit;
	int askOrderMinPrice = askBook[minPriceIndex].price;
	printf("판매 대기 평균가: %d\n", askOrderAvrg);
	printf("판매 대기 최저가: %d\n", askOrderMinPrice);
	
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
