#pragma once

#include <iostream>
#include <vector>
#include <Eigen/Dense>
#include <mutex>

#include "LSTM/NetworkManager.h"
#include "LSTM/LSTM.h"
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
	bool getTrainStatus() { return bTrain; }
	
	bool predict(string datetime, vector<double> &result);
	double train();
	
	void saveNetwork();
	void loadNetwork();
	
private:
	int fetchTrainData(vector<vector<VectorXd>> &trainDataArr, int &lastDataNumber);

private:
	const int INPUT_NUM = 30, HIDDEN_NUM = 35, OUTPUT_NUM = 5;
	const int INPUT_SIZE = 10, HIDDEN_SIZE = 10, OUTPUT_SIZE = 1;
	
	const double TRAIN_DATA_MAX = 1.01, TRAIN_DATA_MIN=0.99;
	const int TRAIN_DATA_NUM = 200;
	const int NETWORK_SAVE_INTERVAL = 43200; // 12시간
	
	LSTM *network;
	
	bool bLoop, bTrain;
	
	mutex mtx;
};