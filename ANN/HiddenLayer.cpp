#include "HiddenLayer.h"

#include <cstdio>
#include <iostream>

HiddenLayer::HiddenLayer(int _inputLayerSize, int _hiddenLayerSize) {
	inputLayerSize = _inputLayerSize;
	hiddenLayerSize = _hiddenLayerSize;
    
    double xhWeightLimit = sqrt(6.0/inputLayerSize);	// He 초기화 사용 변수
    double hhWeightLimit = sqrt(6.0/hiddenLayerSize);	// He 초기화 사용 변수
	
	xhWeight = MatrixXd::Random(hiddenLayerSize, inputLayerSize) * xhWeightLimit;
	hhWeight = MatrixXd::Random(hiddenLayerSize, hiddenLayerSize) * hhWeightLimit;
	bias = VectorXd::Zero(hiddenLayerSize);
}
HiddenLayer::HiddenLayer(int _inputLayerSize, int _hiddenLayerSize, NetworkManager &in) {
	inputLayerSize = _inputLayerSize;
	hiddenLayerSize = _hiddenLayerSize;
    
    xhWeight.resize(hiddenLayerSize, inputLayerSize);
	hhWeight.resize(hiddenLayerSize, hiddenLayerSize);
	bias.resize(hiddenLayerSize);
	
	// load xhWeight
	for (int i = 0; i < hiddenLayerSize; i++) {
		for (int j = 0; j < inputLayerSize; j++)
			in >> xhWeight(i, j);
	}
	
	// load hhWeight
	for (int i = 0; i < hiddenLayerSize; i++) {
		for (int j = 0; j < hiddenLayerSize; j++)
			in >> hhWeight(i, j);
	}
	
	// load bias
	for (int i = 0; i < hiddenLayerSize; i++)
		in >> bias(i);
}

void HiddenLayer::forward(VectorXd _x, VectorXd& _h_prev) {
	x = _x;
	h_prev = _h_prev;
	
	VectorXd t = xhWeight*x + hhWeight*h_prev + bias;
	
	h = leakyReLU(t);
	
	_h_prev = h;
}
void HiddenLayer::backward(VectorXd _dy, VectorXd& _dh_next) {
	VectorXd dh = _dy + _dh_next;
	
	VectorXd dt = leakyReLU_diff(dh);
	
	VectorXd dh_prev(hiddenLayerSize);
	for (int i = 0; i < hiddenLayerSize; i++)
		dh_prev(i) = mul(hhWeight.col(i), dt).sum();
	_dh_next = dh_prev;
	
	// update weight
	for (int i = 0; i < hiddenLayerSize; i++) {
		xhWeight.row(i) += -ETA * dt[i] * x;
		hhWeight.row(i) += -ETA * dt[i] * h_prev;
	}
	bias += -ETA * dt;
}

NetworkManager& operator<<(NetworkManager& out, const HiddenLayer& layer) {
	// save xhWeight
	for (int i = 0; i < layer.hiddenLayerSize; i++) {
		for (int j = 0; j < layer.inputLayerSize; j++)
			out << layer.xhWeight(i, j) << " ";
	}
	
	// save hhWeight
	for (int i = 0; i < layer.hiddenLayerSize; i++) {
		for (int j = 0; j < layer.hiddenLayerSize; j++)
			out << layer.hhWeight(i, j) << " ";
	}
	
	// save bias
	for (int i = 0; i < layer.hiddenLayerSize; i++)
		out << layer.bias(i) << " ";
	
	return out;
}