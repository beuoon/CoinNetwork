/*
	RNN과 LSTM에 대한 이해: https://ratsgo.github.io/natural%20language%20processing/2017/03/09/rnnlstm/
*/
#pragma once

#include <vector>
#include <Eigen/Dense>

#include "HiddenLayer.h"
#include "OutputLayer.h"

using namespace std;
using namespace Eigen;

class LSTM {
public:
	LSTM(int _inputLayerNum, int _hiddenLayerNum, int _outputLayerNum, 
		 int _inputLayerSize, int _hiddenLayerSize, int _outputLayerSize,
		 bool _bContinuous = false);
	LSTM(istream &in);
	~LSTM();
	
	vector<VectorXd> predict(vector<VectorXd> _inputs) { return forward(_inputs); }
	double train(vector<VectorXd> _inputs, vector<VectorXd> _labels);
	
	void initContinuous() {
		h_prev = VectorXd::Zero(hiddenLayerSize);
		c_prev = VectorXd::Zero(hiddenLayerSize);
	}
	
private:
	vector<VectorXd> forward(vector<VectorXd> _inputs);
	void backward(vector<VectorXd> _deltas);
	
private:
	int inputLayerNum, hiddenLayerNum, outputLayerNum;
	int inputLayerSize, hiddenLayerSize, outputLayerSize;
	
	vector<HiddenLayer *> hiddenLayers;
	vector<OutputLayer *> outputLayers;
	
	bool bContinuous;
	VectorXd h_prev, c_prev;
};