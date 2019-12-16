#include "DataSaver.h"

#include <unistd.h>
#include <algorithm>

string tts(time_t t) { // Time To String
	char str[250];
	strftime(str, sizeof(str), "%Y-%m-%d %H:%M:%S", localtime(&t));
	return str;
}

DataSaver::DataSaver() {
	bLoop = true;
	timeInterval = 60;
}
	
void DataSaver::loop() {
	setting();
		
	while (bLoop) {
		sleep(1); // 1초 지연
		
		time_t fetchTime = time(NULL);
		fetchTransactionData(); // 거래 정보 축적
		if (fetchTime < endTime) continue;
		
		//＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊//
		processTransactionData();
		processOrderData();
		
		// 예외처리 (첫 시도인데 데이터가 없을 때)
		if (count == 0 && (trans_ask_unit == 0 || trans_bid_unit == 0 || order_ask_unit == 0 || order_bid_unit == 0)) {
			startTimeStr = endTimeStr;
			endTime += timeInterval;
			endTimeStr = tts(endTime);
			
			continue;
		}
		
		// sendDB();
		
		savePrevData();
		
		//＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊//
		startTimeStr = endTimeStr;
		endTime += timeInterval;
		endTimeStr = tts(endTime);
		
		count++;
	}
}
void DataSaver::setting() {
	// 시간 설정
	startTime = time(NULL);
	startTime += -(startTime%timeInterval) + timeInterval; // 다음 분부터
	endTime = startTime + timeInterval;
	startTimeStr = tts(startTime), endTimeStr = tts(endTime);
	
	// firstNumber 설정
	MySQL *mysql = MySQL::getInstance();
	MYSQL_ROW row;
	
	mysql->query("select number from history order by datetime desc limit 1");
	if (mysql->storeResult() == NULL) 			firstNumber = 1;
	else {
		if ((row = mysql->fetchRow()) == NULL)	firstNumber = 1;
		else									firstNumber = atoi(row[0]) + 1;
		mysql->freeResult();
	}
	
	// 기타
	count = 0;
	histories.clear();
	
	prev_trans_ask_min = 0, prev_trans_ask_avrg = 0, prev_trans_ask_unit = 0;
	prev_trans_bid_max = 0, prev_trans_bid_avrg = 0, prev_trans_bid_unit = 0;
	prev_order_ask_min = 0, prev_order_ask_avrg = 0, prev_order_bid_max = 0, prev_order_bid_avrg = 0;
}

void DataSaver::fetchTransactionData() {
	vector<Bithumb::Transaction> newHistories;
	if (!Bithumb::getTransactionHistory(100, newHistories)) return ;
	
	vector<Bithumb::Transaction>::iterator iter;

	// 이전 분 데이터 삭제
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
			if (newHistories[i].date >= endTimeStr) break; // 다음 분 데이터 넘기기
			
			histories.push_back(newHistories[i]);
		}
	}
}
	
void DataSaver::processTransactionData() {
	trans_ask_min = 0, trans_ask_avrg = 0, trans_ask_unit = 0;
	trans_bid_max = 0, trans_bid_avrg = 0, trans_bid_unit = 0;

	for (auto iter = histories.begin(); iter != histories.end(); iter = histories.erase(iter)) {
		if (iter->type == Bithumb::TransactionType::ASK) { // 매도
			if (trans_ask_min == 0 || trans_ask_min > iter->price)
				trans_ask_min = iter->price;
			trans_ask_avrg += iter->total;
			trans_ask_unit += iter->unit;
		}
		else { // 매수
			if (trans_bid_max == 0 || trans_bid_max < iter->price)
				trans_bid_max = iter->price;
			trans_bid_avrg += iter->total;
			trans_bid_unit += iter->unit;
		}
	}
	
	if (trans_ask_unit != 0) trans_ask_avrg /= trans_ask_unit;
	if (trans_bid_unit != 0) trans_bid_avrg /= trans_bid_unit;
}
void DataSaver::processOrderData() {
	order_ask_min = 0, order_ask_avrg = 0, order_ask_unit = 0;
	order_bid_max = 0, order_bid_avrg = 0, order_bid_unit = 0;
	
	// 주문 정보 가져오기
	map<string, vector<Bithumb::Order>> orderBook;
	if (!Bithumb::getOrderBook(30, orderBook)) return ;
	
	vector<Bithumb::Order> askOrder = orderBook["ask"];
	vector<Bithumb::Order> bidOrder = orderBook["bid"];
	
	// 매도
	for (int i = 0; i < askOrder.size(); i++) {
		if (order_ask_min == 0 || order_ask_min > askOrder[i].price)
			order_ask_min = askOrder[i].price;
		order_ask_avrg += askOrder[i].price * askOrder[i].unit;
		order_ask_unit += askOrder[i].unit;
	}
	if (order_ask_unit != 0) order_ask_avrg /= order_ask_unit;
	
	// 매수
	for (int i = 0; i < bidOrder.size(); i++) {
		if (order_bid_max == 0 || order_bid_max < bidOrder[i].price)
			order_bid_max = bidOrder[i].price;
		order_bid_avrg += bidOrder[i].price * bidOrder[i].unit;
		order_bid_unit += bidOrder[i].unit;
	}
	if (order_bid_unit != 0) order_bid_avrg /= order_bid_unit;
}

void DataSaver::sendDB() {
	// 변화량 측정
	double trans_ask_min_rate = 0, trans_ask_avrg_rate = 0;
	double trans_bid_max_rate = 0, trans_bid_avrg_rate = 0;
	double order_ask_min_rate = 0, order_ask_avrg_rate = 0;
	double order_bid_max_rate = 0, order_bid_avrg_rate = 0;
		
	if (count > 0) {
		if (trans_ask_unit != 0) {
			trans_ask_min_rate = ((trans_ask_min/prev_trans_ask_min - TRAIN_DATA_MIN)/(TRAIN_DATA_MAX-TRAIN_DATA_MIN) - 0.5)/0.1225;
			trans_ask_avrg_rate = ((trans_ask_avrg/prev_trans_ask_avrg - TRAIN_DATA_MIN)/(TRAIN_DATA_MAX-TRAIN_DATA_MIN) - 0.5)/0.1225;
		}
		if (trans_bid_unit != 0) {
			trans_bid_max_rate = ((trans_bid_max/prev_trans_bid_max - TRAIN_DATA_MIN)/(TRAIN_DATA_MAX-TRAIN_DATA_MIN) - 0.5)/0.1225;
			trans_bid_avrg_rate = ((trans_bid_avrg/prev_trans_bid_avrg - TRAIN_DATA_MIN)/(TRAIN_DATA_MAX-TRAIN_DATA_MIN) - 0.5)/0.1225;
		}
		if (order_ask_unit != 0) {
			order_ask_min_rate = ((order_ask_min/prev_order_ask_min - TRAIN_DATA_MIN)/(TRAIN_DATA_MAX-TRAIN_DATA_MIN) - 0.5)/0.02;
			order_ask_avrg_rate = ((order_ask_avrg/prev_order_ask_avrg - TRAIN_DATA_MIN)/(TRAIN_DATA_MAX-TRAIN_DATA_MIN) - 0.5)/0.02;
		}
		if (order_bid_unit != 0) {
			order_bid_max_rate = ((order_bid_max/prev_order_bid_max - TRAIN_DATA_MIN)/(TRAIN_DATA_MAX-TRAIN_DATA_MIN) - 0.5)/0.02;
			order_bid_avrg_rate = ((order_bid_avrg/prev_order_bid_avrg - TRAIN_DATA_MIN)/(TRAIN_DATA_MAX-TRAIN_DATA_MIN) - 0.5)/0.02;
		}
	}
		
	//＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊//
	// DB 전송
	cout << startTimeStr << " ~ " << endTimeStr << endl;
	
	MySQL *mysql = MySQL::getInstance();
	char query[1000];
	sprintf(query, "insert into history values(NULL, '%s', %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf)", startTimeStr.c_str(),
			trans_ask_min, trans_ask_avrg, trans_ask_unit,
			trans_bid_max, trans_bid_avrg, trans_bid_unit,
			order_ask_min, order_ask_avrg, order_bid_max, order_bid_avrg);
	mysql->query(query);
	
	// 학습 데이터
	if (count > 0) {
		sprintf(query, "insert into train_data values(%d, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf)", firstNumber + count,
				trans_ask_min_rate, trans_ask_avrg_rate, trans_ask_unit/100,
				trans_bid_max_rate, trans_bid_avrg_rate, trans_bid_unit/100,
				order_ask_min_rate, order_ask_avrg_rate, order_bid_max_rate, order_bid_avrg_rate);
		mysql->query(query);
	}
}

void DataSaver::savePrevData() {
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
}