#include <iostream>
#include <cstdio>
#include <vector>
#include <cmath>
#include "../MySQL.h"

using namespace std;

time_t stt(const char *str) {
	struct tm ts;
	
	sscanf(str, "%d-%d-%d %d:%d:%d", &ts.tm_year, &ts.tm_mon, &ts.tm_mday, &ts.tm_hour, &ts.tm_min, &ts.tm_sec);
	
	return mktime(&ts);
}

int getPrice(vector<string>& data) {
	double trans_ask_min = atof(data[2].c_str());
	double trans_bid_max = atof(data[5].c_str());
	double price = 0;
	
	if (trans_ask_min != 0 && trans_bid_max != 0)
		price = (trans_ask_min + trans_bid_max) / 2;
	else if (trans_ask_min != 0)
		price = trans_ask_min;
	else if (trans_bid_max != 0)
		price = trans_bid_max;
	else
		price = 0;
	
	return price;
}

int main() {
	const int DATA_LIMIT = 300;
	char query[1000];
		
	vector<vector<vector<string>>> dataArrList;
	int lastDataNumber = 0;
	time_t prevT = 0;
	int dataCount = 0;
	
	MySQL *mysql = MySQL::getInstance();
	MYSQL_ROW row;
	
	mysql->query("delete from train_data");
	
	do {
		if (!dataArrList.empty()) {
			vector<vector<string>> lastDataArr = dataArrList.back();
			
			vector<string> lastData = lastDataArr.back();
			lastDataArr.clear();
			lastDataArr.push_back(lastData);
			lastDataNumber = atoi(lastData[0].c_str());
						
			dataArrList.clear();
			dataArrList.push_back(lastDataArr);
		}
		
		// 데이터 가져오기
		
		char query[1000];
		sprintf(query, "select * from history where number > %d limit %d", lastDataNumber, DATA_LIMIT);
		
		
		if (!mysql->query_result(query)) return 0;
		
		int arrIndex = (dataArrList.size() == 0) ? -1 : 0; dataCount = 0;
		
		while ((row = mysql->fetchRow()) != NULL) {
			time_t t = stt(row[1]);
			if (t - prevT != 60) {
				dataArrList.push_back(vector<vector<string>>());
				arrIndex++;
			}
			prevT = t;
			
			vector<string> data;
			for (int i = 0; i < 12; i++)
				data.push_back(row[i]);
			dataArrList[arrIndex].push_back(data);
			
			dataCount++;
		}
		mysql->freeResult();
		
		// 데이터 넣기
		for (int i = 0; i < dataArrList.size(); i++) {
			int startIndex = 0;
			double prevPrice = 0;
			do {
				vector<string>& data = dataArrList[i][startIndex++];
				prevPrice = getPrice(data);
			} while (startIndex < dataArrList[i].size() && prevPrice == 0);
			
			for (int j = startIndex; j < dataArrList[i].size(); j++) {
				vector<string>& data = dataArrList[i][j];
				int number;
				double trans_price_rate, trans_ask_unit, trans_bid_unit;
				
				number = atoi(data[0].c_str());
				double trans_price = getPrice(data);
				if (trans_price == 0)
					trans_price_rate = 0;
				else {
					trans_price_rate = (trans_price/prevPrice - 1) / 0.1225;
					prevPrice = trans_price;
				}
				trans_ask_unit = atof(data[4].c_str()) / 100;
				trans_bid_unit = atof(data[7].c_str()) / 100;
				
				cout << number << " " << trans_price_rate << endl;
				
				sprintf(query, "insert into train_data values(%d, %lf, %lf, %lf)", number,
						trans_price_rate, trans_ask_unit, trans_bid_unit);
				mysql->query(query);
			}
			if (i != dataArrList.size()-1)
				cout << "-----------------------------------------" << endl;
		}
	} while (dataCount == DATA_LIMIT);
	
	return 1;
}