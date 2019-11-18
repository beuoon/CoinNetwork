/*
	RNN과 LSTM에 대한 이해: https://ratsgo.github.io/natural%20language%20processing/2017/03/09/rnnlstm/
*/
#pragma once

#include <vector>
#include <Eigen/Dense>

#include "HiddenLayer.h"
#include "OutputLayer.h"
#include "NetworkManager.h"

using namespace std;
using namespace Eigen;

class LSTM {
public:
	LSTM(int _inputLayerNum, int _hiddenLayerNum, int _outputLayerNum, 
		 int _inputLayerSize, int _hiddenLayerSize, int _outputLayerSize);
	LSTM(NetworkManager &in);
	~LSTM();
	
	vector<VectorXd> predict(vector<VectorXd> _inputs) { return forward(_inputs); }
	double train(vector<VectorXd> _inputs, vector<VectorXd> _labels);
	
	friend NetworkManager& operator<<(NetworkManager& out, const LSTM &network);
	
private:
	vector<VectorXd> forward(vector<VectorXd> _inputs);
	void backward(vector<VectorXd> _deltas);
	
private:
	int inputLayerNum, hiddenLayerNum, outputLayerNum;
	int inputLayerSize, hiddenLayerSize, outputLayerSize;
	
	vector<HiddenLayer *> hiddenLayers;
	vector<OutputLayer *> outputLayers;
};
