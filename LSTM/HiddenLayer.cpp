#include "HiddenLayer.h"

#include <iostream>

HiddenLayer::HiddenLayer(int _inputLayerSize, int _hiddenLayerSize) {
	inputLayerSize = _inputLayerSize;
	hiddenLayerSize = _hiddenLayerSize;
    
    double xhWeightLimit = sqrt(6.0/inputLayerSize);	// He 초기화 사용 변수
    double hhWeightLimit = sqrt(6.0/hiddenLayerSize);	// He 초기화 사용 변수
	double biasLimit = sqrt(6.0/(inputLayerSize + hiddenLayerSize));
	
	xhWeight.resize(hiddenLayerSize, inputLayerSize);
	hhWeight.resize(hiddenLayerSize, hiddenLayerSize);
	bias.resize(hiddenLayerSize);
	
	xhWeight.setRandom();
	hhWeight.setRandom();
	bias.setRandom();
	
	xhWeight *= xhWeightLimit;
	hhWeight *= hhWeightLimit;
	bias *= biasLimit;
}
HiddenLayer::HiddenLayer(int _inputLayerSize, int _hiddenLayerSize, NetworkManager &in) {
	inputLayerSize = _inputLayerSize;
	hiddenLayerSize = _hiddenLayerSize;
    
    xhWeight.resize(hiddenLayerSize, inputLayerSize);
	hhWeight.resize(hiddenLayerSize, hiddenLayerSize);
	bias.resize(hiddenLayerSize);
	
	// load xhWeight
	for (int i = 0; i < inputLayerSize; i++) {
		for (int j = 0; j < hiddenLayerSize; j++)
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

void HiddenLayer::forward(VectorXd _x, VectorXd &_h_prev, VectorXd &_c_prev) {
	x = _x;
	h_prev = _h_prev;
	c_prev = _c_prev;
	
	VectorXd t = xhWeight*x + hhWeight*h_prev + bias;
	
	f = sigmoid(t);
	i = sigmoid(t);
	g = tanh(t);
	o = sigmoid(t);
	
	c = mul(c_prev, f) + mul(i, g);
	h = mul(tanh(c), o);
	
	_h_prev = h;
	_c_prev = c;
}
void HiddenLayer::backward(VectorXd _dy, VectorXd &_dh_next, VectorXd &_dc_next) {
	VectorXd dh = _dy + _dh_next;
	
	VectorXd dc = mul(tanh_diff(c), mul(o, dh)) + _dc_next;
	VectorXd dc_prev = mul(dc, f);
	
	VectorXd df_ = mul(sigmoid_diff(f), mul(dc, c_prev));
	VectorXd di_ = mul(sigmoid_diff(i), mul(dc, g));
	VectorXd dg_ = mul(tanh_diff(g), mul(dc, i));
	VectorXd do_ = mul(sigmoid_diff(o), mul(dh, tanh(c)));
	
	VectorXd dt = df_ + di_ + dg_ + do_;
	
	// VectorXd dh_prev = hhWeight * dt;
	VectorXd dh_prev(hiddenLayerSize);
	for (int i = 0; i < hiddenLayerSize; i++)
		dh_prev(i) = mul(hhWeight.col(i), dt).sum();
	
	// update Weight
	for (int i = 0; i < hiddenLayerSize; i++) {
		xhWeight.row(i) += -ETA * dt[i] * x;
		hhWeight.row(i) += -ETA * dt[i] * h_prev;
	}
	bias += -ETA * dt;
	
	_dh_next = dh_prev;
	_dc_next = dc_prev;
}

NetworkManager& operator<<(NetworkManager& out, const HiddenLayer &layer) {
	// save xhWeight
	for (int i = 0; i < layer.inputLayerSize; i++) {
		for (int j = 0; j < layer.hiddenLayerSize; j++)
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