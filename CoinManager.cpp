#include "CoinManager.h"

#include <unistd.h>
#include <cstdio>
#include <string>

#include <cmath> // DEBUG

CoinManager::CoinManager() {
	network = new LSTM(INPUT_NUM, HIDDEN_NUM, OUTPUT_NUM, INPUT_SIZE, HIDDEN_SIZE, OUTPUT_SIZE);
	
	bLoop = true;
	bTrain = true;
}
CoinManager::~CoinManager() {
	delete network;
}
	
void CoinManager::loop() {
	loadNetwork();
	cout << "network loop start" << endl;
	time_t prevSaveTime = time(NULL);
	
	while (bLoop) {
		sleep(1); // 1초 지연
		
		if (bTrain) {
			double error = train();
			cout << "Error: " << error << endl;
			
			// 신경망 저장
			if (time(NULL) - prevSaveTime >= NETWORK_SAVE_INTERVAL) {
				saveNetwork();
				prevSaveTime = time(NULL);
			}
		}
	}
	
	cout << "network loop end" << endl;
}

bool CoinManager::predict(string datetime, vector<double> &result) {
	//＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊//
	// Input data 가져오기
	MySQL *mysql = MySQL::getInstance();
	MYSQL_ROW row;
	
	char query[1000];
	
	sprintf(query, "select number from history where datetime = '%s'", datetime.c_str());
	mysql->query(query);
	if (mysql->storeResult() == NULL) return false;
	if ((row = mysql->fetchRow()) == NULL) {
		mysql->freeResult();
		return false;
	}
	int dataNumber = atoi(row[0]);
	mysql->freeResult();
	
	sprintf(query, "select * from train_data where number >= %d limit %d", dataNumber, INPUT_NUM);
	mysql->query(query);
	if (mysql->storeResult() == NULL) return false;
	if (mysql->getRowNum() != INPUT_NUM) {
		mysql->freeResult();
		return false;
	}
	
	vector<VectorXd> input;
	while ((row = mysql->fetchRow()) != NULL) {
		int number = atoi(row[0]);
		
		if (dataNumber+1 < number) return false;
		dataNumber = number;
		
		VectorXd data(INPUT_SIZE);
		for (int i = 0; i < INPUT_SIZE; i++)
			data(i) = atof(row[i+1]);
		input.push_back(data);
	}
	mysql->freeResult();
	
	//＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊//
	// 학습
	mtx.lock();
	vector<VectorXd> output = network->predict(input);
	mtx.unlock();
	
	// 데이터 변경
	result.clear();
	for (int i = 0; i < output.size(); i++) {
		double value = output[i](0)*(TRAIN_DATA_MAX-TRAIN_DATA_MIN) + TRAIN_DATA_MIN;
		result.push_back(value);
	}
	
	return true;
}
double CoinManager::train() {
	double totalError = 0;
	int lastDataNumber = 1, dataNum;
	vector<vector<VectorXd>> trainDataArr(1);
	
	do {
		dataNum = fetchTrainData(trainDataArr, lastDataNumber);
		
		for (int i = 0; i < trainDataArr.size(); i++) {
			if (trainDataArr[i].size() < HIDDEN_NUM) continue;
				
			vector<VectorXd>::iterator firstIter = trainDataArr[i].begin();
			vector<VectorXd>::iterator middleIter = firstIter + INPUT_NUM;
			vector<VectorXd>::iterator lastIter = middleIter + OUTPUT_NUM;
			
			mtx.lock();
			while (lastIter != trainDataArr[i].end()) {
				vector<VectorXd> input(firstIter, middleIter);
				vector<VectorXd> label;
				for (auto iter = middleIter; iter != lastIter; iter++) {
					VectorXd data(1); data << (*iter)(0); // trans_ask_min_rate
					label.push_back(data);
				}
				
				totalError += network->train(input, label);
				
				// DEBUG
				if (isnan(totalError)) {
					cout << "NaN 발생" << endl;
					exit(1);
				}
				
				firstIter++;
				middleIter++;
				lastIter++;
			}
			mtx.unlock();
		}
	} while (dataNum == TRAIN_DATA_NUM);
	
	return totalError;
}
	
int CoinManager::fetchTrainData(vector<vector<VectorXd>> &trainDataArr, int &lastDataNumber) {
	// 이전 데이터 중 마지막 배열의 HIDDEN_NUM - 1개를 제외하고 삭제
	if (!trainDataArr.empty()) {
		vector<VectorXd> temp = trainDataArr.back();
		
		if (temp.size() >= HIDDEN_NUM) {
			int eraseNum = temp.size()-HIDDEN_NUM+1;
			temp.erase(temp.begin(), temp.begin()+eraseNum);
		}
		
		trainDataArr.clear();
		trainDataArr.push_back(temp);
	}
	
	// 데이터 가져오기
	MySQL *mysql = MySQL::getInstance();
	MYSQL_ROW row;
	
	char query[1000];
	sprintf(query, "select * from train_data where number > %d limit %d", lastDataNumber, TRAIN_DATA_NUM);
	mysql->query(query);
	
	if (mysql->storeResult() == NULL) return 0;
	
	int arrIndex = 0, dataCount = 0;
	
	while ((row = mysql->fetchRow()) != NULL) {
		int number = atoi(row[0]);
		if (lastDataNumber+1 < number) {
			trainDataArr.push_back(vector<VectorXd>());
			arrIndex++;
		}
		lastDataNumber = number;
		
		VectorXd data(INPUT_SIZE);
		for (int i = 0; i < INPUT_SIZE; i++)
			data(i) = atof(row[i+1]);
		trainDataArr[arrIndex].push_back(data);
		
		dataCount++;
	}
	mysql->freeResult();
	
	return dataCount;
}
	
void CoinManager::saveNetwork() {
	NetworkManager mgr;
	mgr << *network;
	const char *networkStr = mgr.save();
	
	char timeStr[250];
	time_t t = time(NULL);
	strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", localtime(&t));
	
	// DB 저장
	char query[200000];
	sprintf(query, "insert into network values(NULL, '%s', '%s')", timeStr, networkStr);
	MySQL::getInstance()->query(query);
}
void CoinManager::loadNetwork() {
	// DB 가져오기
	MySQL *mysql = MySQL::getInstance();
	MYSQL_ROW row;
	mysql->query("select data from network order by id desc limit 1");

	if (mysql->storeResult() == NULL) return;
	if ((row = mysql->fetchRow()) == NULL) {
		mysql->freeResult();
		return;
	}
	char *str = row[0];
	
	NetworkManager mgr;
	mgr.load(str);
	
	if (network != nullptr)
		delete network;
	network = new LSTM(mgr);
	
	mysql->freeResult();
}