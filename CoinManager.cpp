#include "CoinManager.h"

#include <unistd.h>
#include <cstdio>
#include <string>

#include <cmath> // DEBUG

mutex CoinManager::mtx;

CoinManager::CoinManager() {
	network = new DQRN(INPUT_NUM, HIDDEN_NUM, INPUT_SIZE, HIDDEN_SIZE, OUTPUT_SIZE);
	
	bestNetwork = nullptr;
	bestBenefit = 0;
	loadNetwork();
	
	bLoop = true;
	bTrain = true;
	bInvest = false;
}
CoinManager::~CoinManager() {
	delete network;
}
	
void CoinManager::loop() {
	int totalTrainCount = 0;
	
	while (bLoop) {
		usleep(1000); // 0.001초 지연
		
		// 학습
		if (bTrain) {
			// 학습
			int trainCount = 0;
			double trainError = train(trainCount);
			totalTrainCount += trainCount;
			
			// 검토
			double accuracy, loss;
			int benefit;
			benefit = checkAccuracy(accuracy, loss);
			
			// 갱신
			if (benefit > bestBenefit) {
				bestBenefit = benefit;
				cout << "Best Benefit: " << benefit << endl;
				
				mtx.lock();
				if (bestNetwork != nullptr)
					delete bestNetwork;
				NetworkManager mgr; mgr << *network;
				bestNetwork = new DQRN(mgr);
				mtx.unlock();
				
				saveNetwork();
			}
			
			printf("TC: %d, B: %d, A: %.8lf, L: %.6lf, BB: %d\n", totalTrainCount, benefit, accuracy, loss, bestBenefit);
			fflush(stdout);
			
			// NaN 발생
			if (isnan(loss) || isinf(loss)) {
				cout << "문제가 발생하여 새로운 신경망을 생성했습니다." << endl;
				delete network;
				network = new DQRN(INPUT_NUM, HIDDEN_NUM, INPUT_SIZE, HIDDEN_SIZE, OUTPUT_SIZE);
				
				totalTrainCount = 0;
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
	if (bestNetwork == nullptr)
		return false;
	
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
	VectorXd output = bestNetwork->predict(input);
	mtx.unlock();
	
	// 데이터 변경
	result.clear();
	for (int i = 0; i < output.size(); i++)
		result.push_back(output[i]);
	
	return true;
}
double CoinManager::train(int& trainCount) {
	double totalError = 0;
	static vector<vector<VectorXd>> trainDataArr(1);
	static int lastDataNumber = 1;
	int dataNum;
	double money = 1;
	
	dataNum = fetchTrainData(trainDataArr, lastDataNumber);
		
	for (int i = 0; i < trainDataArr.size(); i++) {
		if (trainDataArr[i].size() < HIDDEN_NUM) continue;
		
		vector<VectorXd>::iterator inputIter = trainDataArr[i].begin();
		vector<VectorXd>::iterator middleIter = inputIter + INPUT_NUM - 1;
		vector<VectorXd>::iterator labelIter = middleIter;
		
		while (labelIter != trainDataArr[i].end()) {
			vector<VectorXd> input(inputIter, middleIter); // add money
			double rate = (*labelIter)(0); // ask_min_rate
			VectorXd label = (rate >= 0.4) ? bombLabel : ((rate > 0.0) ? increaseLabel : decreaseLabel);
			
			result = network->predict(input);
			if (result == 1, 0) {
				money *= rate;
				reward = 1.0-rate;
			}
			else {
				reward = rate-1.0;
			}
			
			
			
			totalError += network->train(input, label);
			trainCount++;
			
			inputIter++;
			middleIter++;
			labelIter++;
		}
	}
	
	if (dataNum != TRAIN_DATA_NUM) {
		lastDataNumber = 1;
		trainDataArr.clear();
		trainDataArr.push_back(vector<VectorXd>());
	}
	
	return totalError;
}
int CoinManager::checkAccuracy(double& accuracy, double& loss) {
	vector<vector<VectorXd>> trainDataArr(1);
	int lastDataNumber = 1, dataNum, count = 0;
	
	accuracy = 0, loss = 0;
	
	VectorXd bombLabel(OUTPUT_SIZE);	bombLabel << 1, 0, 0;
	VectorXd increaseLabel(OUTPUT_SIZE);increaseLabel << 0, 1, 0;
	VectorXd decreaseLabel(OUTPUT_SIZE);decreaseLabel << 0, 0, 1;
	
	// TEST
	int BS = 0, BF = 0, IS = 0, IF = 0, DS = 0, DF = 0;
	
	do {
		dataNum = fetchTrainData(trainDataArr, lastDataNumber);
			
		for (int i = 0; i < trainDataArr.size(); i++) {
			if (trainDataArr[i].size() < HIDDEN_NUM) continue;
			
			vector<VectorXd>::iterator inputIter = trainDataArr[i].begin();
			vector<VectorXd>::iterator middleIter = inputIter + INPUT_NUM;
			vector<VectorXd>::iterator labelIter = middleIter;
			
			while (labelIter != trainDataArr[i].end()) {
				vector<VectorXd> input(inputIter, middleIter);				
				double rate = (*labelIter)(0); // ask_min_rate
				VectorXd label = (rate >= 0.4) ? bombLabel : ((rate > 0.0) ? increaseLabel : decreaseLabel);
				// cout << (*labelIter)(0) << " -> " << ((*labelIter)(0)*0.1225 + 0.5) * 0.02 + 0.99 << endl;
				
				// Error
				double error = 0;
				VectorXd output = network->predict(input);
				
				VectorXd delta(OUTPUT_SIZE);
				for (int i = 0; i < OUTPUT_SIZE; i++) {
					delta[i] = output[i] - label[i];
					error += -label[i]*log(output[i]); // cross-entropy error
				}
				error /= OUTPUT_SIZE;
				
				loss += error;
				
				// Accuracy
				if (label == bombLabel && output[0] > output[1] && output[0] > output[2] ||
					label == increaseLabel && output[1] > output[0] && output[1] > output[2] ||
					label == decreaseLabel && output[2] > output[0] && output[2] > output[1])
					accuracy += 1;
					
				// TEST
				if (label == bombLabel) {
					if (output[0] > output[1] && output[0] > output[2]) BS++;
					else 					   							BF++;
				}
				else if (label == increaseLabel) {
					if (output[1] > output[0] && output[1] > output[2]) IS++;
					else 					   							IF++;
				}
				else {
					if (output[2] > output[0] && output[2] > output[1]) DS++;
					else												DF++;
				}
				
				count++;
				
				inputIter++;
				middleIter++;
				labelIter++;
			}
		}
	} while (dataNum == TRAIN_DATA_NUM);
	
	printf("C: %d, B:%d/%d, I:%d/%d, D:%d/%d\n", count, BS, BF, IS, IF, DS, DF);
	fflush(stdout);
	
	loss /= count;
	accuracy /= count;
	
	return BS-DF;
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
	NetworkManager mgr; mgr << *bestNetwork;
	string networkStr = mgr.save();
	
	// DB 비우기
	MySQL::getInstance()->query("delete from network");
	
	// DB 저장
	char timeStr[250];
	time_t t = time(NULL);
	strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", localtime(&t));
	
	char *query = new char[1000 + networkStr.size()];
	sprintf(query, "insert into network values(NULL, '%s', %d, '%s')", timeStr, bestBenefit, networkStr.c_str());
	MySQL::getInstance()->query(query);
	delete[] query;
}
void CoinManager::loadNetwork() {
	// DB 가져오기
	MySQL *mysql = MySQL::getInstance();
	MYSQL_ROW row;
	mysql->query("select benefit, data from network order by id desc limit 1");

	if (mysql->storeResult() == NULL) return;
	if ((row = mysql->fetchRow()) == NULL) {
		mysql->freeResult();
		return;
	}
	bestBenefit = atof(row[0]);
	char *str = row[1];
	
	NetworkManager mgr;
	mgr.load(string(str));
	
	mtx.lock();
	if (bestNetwork != nullptr)
		delete bestNetwork;
	bestNetwork = new DQRN(mgr);
	mtx.unlock();
	
	mysql->freeResult();
}