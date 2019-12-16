#include "CoinManager.h"

#include <unistd.h>
#include <cstdio>
#include <string>

#include <cmath> // DEBUG

CoinManager::CoinManager() {
	network = nullptr;
	loadNetwork();
	if (network == nullptr)
		network = new DQRN(INPUT_NUM, HIDDEN_NUM, INPUT_SIZE, HIDDEN_SIZE, OUTPUT_SIZE);
	
	bLoop = true;
	bTrain = true;
	bInvest = false;
	bPrevNetwork = false;
}
CoinManager::~CoinManager() {
	delete network;
}
	
void CoinManager::loop() {
	time_t prevSaveTime = time(NULL);
	
	while (bLoop) {
		usleep(1000); // 0.001초 지연
		
		// 학습
		if (bTrain) {
			// 백업
			NetworkManager mgr;
			mgr << *network;
			string networkStr = mgr.save();
			
			// 학습
			int trainCount = 0;
			double error = train(trainCount);
			if (error > 0) {
				printf("TC: %d, Accuracy: %.8lf, Error: %.6lf\n", trainCount, 1.-(error/trainCount), error);
				fflush(stdout);
			}
			
			// NaN 발생
			if (isnan(error)) {
				mtx.lock();
				if (!bPrevNetwork) {
					cout << "NaN 발생 - 새로운 신경망 생성" << endl;
					prevNetworkStr = networkStr;
					delete network;
					network = new DQRN(INPUT_NUM, HIDDEN_NUM, INPUT_SIZE, HIDDEN_SIZE, OUTPUT_SIZE);
					bPrevNetwork = true;
				}
				else {
					cout << "NaN 발생 - 신경망 교배" << endl;
					mgr.cross(prevNetworkStr, networkStr);
					delete network;
					network = new DQRN(mgr);
					bPrevNetwork = false;
				}
				mtx.unlock();
			}
			
			// 신경망 저장
			if (time(NULL) - prevSaveTime >= NETWORK_SAVE_INTERVAL) {
				saveNetwork();
				prevSaveTime = time(NULL);
			}
		}
		
		// 투자
		if (bInvest) {
			vector<double> predictResult;
			if (futurePredict(predictResult))
				invest(predictResult);
		}
	}
}

bool CoinManager::futurePredict(vector<double> &result) {
	/*
	//＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊//
	// Input data 가져오기
	MySQL *mysql = MySQL::getInstance();
	MYSQL_ROW row;
	
	char query[1000];
	
	sprintf(query, "select datetime from history order by number desc limit 1");
	mysql->query(query);
	if (mysql->storeResult() == NULL) return false;
	if ((row = mysql->fetchRow()) == NULL) {
		mysql->freeResult();
		return false;
	}
	string dataNumber = atoi(row[0]);
	mysql->freeResult();
	
	sprintf(query, "select * from train_data where number > %d limit %d", dataNumber-INPUT_NUM, INPUT_NUM);
	mysql->query(query);
	if (mysql->storeResult() == NULL) return false;
	if (mysql->getRowNum() != INPUT_NUM) {
		mysql->freeResult();
		return false;
	}
	
	vector<VectorXd> input;
	while ((row = mysql->fetchRow()) != NULL) {
		int number = atoi(row[0]);
		
		if (dataNumber+1 < number) {
			mysql->freeResult();
			return false;
		}
		dataNumber = number;
		
		VectorXd data(INPUT_SIZE);
		for (int i = 0; i < INPUT_SIZE; i++)
			data(i) = atof(row[i+1]);
		input.push_back(data);
	}
	mysql->freeResult();
	
	//＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊//
	// 예측
	mtx.lock();
	VectorXd output = network->predict(input);
	mtx.unlock();
	
	// 데이터 변경
	result.clear();
	for (int i = 0; i < output.size(); i++)
		result.push_back(output[i]);
	*/
	
	return true;
}
void CoinManager::invest(vector<double> futureInfo) {
	// TODO: 자산 현황 확인
	
	
	// KRW가 있는 상태
	// TODO: 미래가 상승한다 - 현재 자산만큼 시장가 매수
	
	// BTC가 있는 상태
	// TODO: 미래가 감소한다 - 현재 자산만큼 시장가 매도
}

bool CoinManager::predict(time_t predictTime, vector<double> &result) {
	char timeStr[250];
	predictTime -= INPUT_NUM*60;
	strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:00", localtime(&predictTime));
	
	//＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊//
	// Input data 가져오기
	MySQL *mysql = MySQL::getInstance();
	MYSQL_ROW row;
	
	char query[1000];
	
	sprintf(query, "select number from history where datetime = '%s'", timeStr);
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
		
		if (dataNumber+1 < number) {
			mysql->freeResult();
			return false;
		}
		dataNumber = number;
		
		VectorXd data(INPUT_SIZE);
		for (int i = 0; i < INPUT_SIZE; i++)
			data(i) = atof(row[i+1]);
		input.push_back(data);
	}
	mysql->freeResult();
	
	//＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊//
	// 예측
	mtx.lock();
	VectorXd output = network->predict(input);
	mtx.unlock();
	
	// 데이터 변경
	result.clear();
	for (int i = 0; i < output.size(); i++)
		result.push_back(output[i]);
	
	return true;
}
double CoinManager::train(int& trainCount) {
	double totalError = 0;
	int lastDataNumber = 1, dataNum;
	vector<vector<VectorXd>> trainDataArr(1);
	
	VectorXd trueLabel(2); trueLabel << 1, 0;
	VectorXd falseLabel(2); falseLabel << 0, 1;
	
	do {
		dataNum = fetchTrainData(trainDataArr, lastDataNumber);
		
		for (int i = 0; i < trainDataArr.size(); i++) {
			if (trainDataArr[i].size() < HIDDEN_NUM) continue;
				
			vector<VectorXd>::iterator inputIter = trainDataArr[i].begin();
			vector<VectorXd>::iterator labelIter = inputIter + INPUT_NUM;
			vector<VectorXd>::iterator prevIter = labelIter-1;
			
			mtx.lock();
			while (labelIter != trainDataArr[i].end()) {
				vector<VectorXd> input(inputIter, labelIter);
				VectorXd label = ((*prevIter)(0)-(*labelIter)(0) > 0) ? trueLabel : falseLabel;
				
				totalError += network->train(input, label);
				trainCount++;
				
				inputIter++;
				labelIter++;
				prevIter++;
			}
			mtx.unlock();
		}
	} while (false); // (dataNum == TRAIN_DATA_NUM);
	
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
	string networkStr = mgr.save();
	
	char timeStr[250];
	time_t t = time(NULL);
	strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", localtime(&t));
	
	// DB 저장
	char *query = new char[1000 + networkStr.size()];
	sprintf(query, "insert into network values(NULL, '%s', '%s')", timeStr, networkStr.c_str());
	MySQL::getInstance()->query(query);
	delete[] query;
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
	mgr.load(string(str));
	
	mtx.lock();
	if (network != nullptr)
		delete network;
	network = new DQRN(mgr);
	mtx.unlock();
	
	mysql->freeResult();
}