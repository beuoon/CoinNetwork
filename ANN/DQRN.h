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

class DQRN {
public:
	DQRN(int _inputLayerNum, int _hiddenLayerNum, int _inputLayerSize, int _hiddenLayerSize, int _outputLayerSize);
	DQRN(NetworkManager &in);
	~DQRN();
	
	VectorXd predict(vector<VectorXd> _inputs) { return forward(_inputs); }
	double train(vector<VectorXd> _inputs, VectorXd _labels);
	double loss(vector<VectorXd> _inputs, VectorXd _labels);
	
	friend NetworkManager& operator<<(NetworkManager& out, const DQRN &network);
	
private:
	VectorXd forward(vector<VectorXd> _inputs);
	void backward(VectorXd _deltas);
	
private:
	int inputLayerNum, hiddenLayerNum;
	int inputLayerSize, hiddenLayerSize, outputLayerSize;
	
	vector<HiddenLayer *> hiddenLayers;
	OutputLayer *outputLayer;
};
