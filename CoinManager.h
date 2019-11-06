#pragma once

#include <iostream>
#include <vector>
#include <Eigen/Dense>

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
	
	// void predict();
	double train();
	
	void fetchTrainData();
	
	void saveNetwork();
	void loadNetwork();

private:
	LSTM *network;
	int inputNum, hiddenNum, outputNum;
	int inputSize, hiddenSize, outputSize;
	
	vector<vector<VectorXd>> trainDataArr;
	int trainDataNum;
	
	bool bLoop;
};