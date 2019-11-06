#include "CoinManager.h"

#include <unistd.h>
#include <cstdio>
#include <string>

CoinManager::CoinManager() {
	inputNum = 60, outputNum = 30;
	hiddenNum = inputNum + outputNum;
	inputSize = 10, hiddenSize = 10, outputSize = 1;
	
	network = new LSTM(inputNum, hiddenNum, outputNum, inputSize, hiddenSize, outputSize); // 60분의 데이터를 주면 이후 30분의 데이터를 추측
	trainDataNum = 1000;
	
	bLoop = true;
}
CoinManager::~CoinManager() {
	delete network;
}
	
void CoinManager::loop() {
	loadNetwork();
	cout << "network loop start" << endl;
	time_t fetchTime = time(NULL), saveTime = time(NULL);
	fetchTrainData();
	
	while (bLoop) {
		sleep(1); // 1초 지연
		
		double error = train();
		cout << "Error: " << error << endl;
		
		if (time(NULL) - fetchTime >= 1800) { // 30분마다 학습 데이터 변경
			fetchTrainData();
			fetchTime = time(NULL);
		}
		if (time(NULL) - saveTime >= 86400) { // 하루마다 신경망 저장
			saveNetwork();
			saveTime = time(NULL);
		}
	}
	
	cout << "network loop end" << endl;
	// saveNetwork();
}

// void CoinManager::predict();
double CoinManager::train() {
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
	
void CoinManager::fetchTrainData() {
	MySQL *mysql = MySQL::getInstance();
	MYSQL_RES *res;
	MYSQL_ROW row;
	
	mysql->query("select number from train_data order by number desc limit 1");
	if ((res = mysql->storeResult()) == NULL) return;
	if ((row = mysql_fetch_row(res)) == NULL) return;
	int totalDataNum = atoi(row[0]);
	
	mysql->freeResult();
	
	int dataNumber = rand() % (totalDataNum - trainDataNum);
	
	char query[1000];
	sprintf(query, "select * from train_data where number >= %d limit %d", dataNumber, trainDataNum);
	mysql->query(query);
	
	if ((res = mysql->storeResult()) == NULL) return;
	
	trainDataArr.clear();
	trainDataArr.push_back(vector<VectorXd>());
	
	int arrCnt = 0;
	int lastTrainDataNumber = dataNumber;
		
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
	mysql->freeResult();
		
	cout << "TrainDataNumber: " << dataNumber << endl;
	for (int i = 0; i < trainDataArr.size(); i++)
		printf("[%d]: %lu\n", i, trainDataArr[i].size());
	cout << endl;
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
	MYSQL_RES *res;
	MYSQL_ROW row;
	MySQL::getInstance()->query("select data from network order by id desc limit 1");

	if ((res = MySQL::getInstance()->storeResult()) == NULL) return;
	if ((row = mysql_fetch_row(res)) == NULL) return;
	char *str = row[0];
	
	NetworkManager mgr;
	mgr.load(str);
	
	if (network != nullptr)
		delete network;
	network = new LSTM(mgr);
	
	MySQL::getInstance()->freeResult();
}