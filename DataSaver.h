#pragma once

#include <ctime>
#include <vector>
#include <string>

#include "Bithumb.h"
#include "MySQL.h"

using namespace std;

class DataSaver {
public:
	DataSaver();
	
	void loop();
	
	void switchLoop(bool _bLoop) { bLoop = _bLoop; }
	
private:
	void setting();
	
	void fetchTransactionData();
	
	void processTransactionData();
	void processOrderData();
	
	void sendDB();
	
	void savePrevData();
	
private:
	bool bLoop;
	
	time_t startTime, endTime;
	string startTimeStr, endTimeStr;
	int timeInterval;
	
	int firstNumber, count;
	
	vector<Bithumb::Transaction> histories;
	
	double trans_ask_min, trans_ask_avrg, trans_ask_unit;
	double trans_bid_max, trans_bid_avrg, trans_bid_unit;
	double order_ask_min, order_ask_avrg, order_ask_unit;
	double order_bid_max, order_bid_avrg, order_bid_unit;
	
	double prev_trans_ask_min, prev_trans_ask_avrg, prev_trans_ask_unit;
	double prev_trans_bid_max, prev_trans_bid_avrg, prev_trans_bid_unit;
	double prev_order_ask_min, prev_order_ask_avrg, prev_order_bid_max, prev_order_bid_avrg;	
};