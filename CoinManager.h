#pragma once

#include <iostream>
#include <vector>
#include <Eigen/Dense>
#include <mutex>

#include "ANN/NetworkManager.h"
#include "ANN/DQRN.h"
#include "MySQL.h"

using namespace Eigen;
using namespace std;

class CoinManager {
public:
	CoinManager();
	~CoinManager();
	
	void loop();
	void switchLoop(bool _bLoop) { bLoop = _bLoop; }
	void switchTrain(bool _bTrain) { bTrain = _bTrain; }
	void switchInvest(bool _bInvest) { bInvest = _bInvest; }
	bool getTrainStatus() { return bTrain; }
	bool getInvestStatus() { return bInvest; }
	
	bool futurePredict(vector<double> &result);
	void invest(vector<double> futureInfo);
	
	bool predict(time_t predictTime, vector<double> &result);
	double train(int& trainCount);
	
	void saveNetwork();
	void loadNetwork();
	
private:
	int fetchTrainData(vector<vector<VectorXd>> &trainDataArr, int &lastDataNumber);

private:
	const int INPUT_NUM = 10, HIDDEN_NUM = 20;
	const int INPUT_SIZE = 10, HIDDEN_SIZE = 10, OUTPUT_SIZE = 2;
	
	const double TRAIN_DATA_MAX = 1.01, TRAIN_DATA_MIN=0.99;
	const int TRAIN_DATA_NUM = 200;
	const int NETWORK_SAVE_INTERVAL = 19600; // 6시간
	
	DQRN *network;
	string prevNetworkStr;
	bool bPrevNetwork;
	
	bool bLoop, bTrain, bInvest;
	
	mutex mtx;
};