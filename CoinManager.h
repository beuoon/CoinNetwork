#pragma once

#include <iostream>
#include <vector>
#include <Eigen/Dense>
#include <mutex>

#include "ANN/NetworkManager.h"
#include "ANN/ANN.h"
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
	double checkAccuracy(double& accuracy, double& loss);

private:
	const int INPUT_NUM = 10, HIDDEN_NUM = 13;
	const int INPUT_SIZE = 3, HIDDEN_SIZE = 5, OUTPUT_SIZE = 2;
	
	const int TRAIN_DATA_NUM = 300;
	
	ANN *network, *bestNetwork;
	double bestBenefit, bestLoss;
	
	bool bLoop, bTrain, bInvest;
	
	static mutex mtx;
};