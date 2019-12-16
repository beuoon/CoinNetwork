#include "HiddenLayer.h"

#include <cstdio>
#include <iostream>
#include <cstring>

HiddenLayer::HiddenLayer(int _inputLayerSize, int _hiddenLayerSize) {
	inputLayerSize = _inputLayerSize;
	hiddenLayerSize = _hiddenLayerSize;
    
    double xhWeightLimit = sqrt(6.0/inputLayerSize);	// He 초기화 사용 변수
    double hhWeightLimit = sqrt(6.0/hiddenLayerSize);	// He 초기화 사용 변수
	
	xhWeight = MatrixXd::Random(hiddenLayerSize, inputLayerSize) * xhWeightLimit;
	hhWeight = MatrixXd::Random(hiddenLayerSize, hiddenLayerSize) * hhWeightLimit;
	bias = VectorXd::Zero(hiddenLayerSize);
	
	// Adam parameter
	xhM = MatrixXd::Zero(hiddenLayerSize, inputLayerSize);
	xhV = MatrixXd::Zero(hiddenLayerSize, inputLayerSize);
	hhM = MatrixXd::Zero(hiddenLayerSize, hiddenLayerSize);
	hhV = MatrixXd::Zero(hiddenLayerSize, hiddenLayerSize);
	bM = VectorXd::Zero(hiddenLayerSize);
	bV = VectorXd::Zero(hiddenLayerSize);
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
	
	// Adam parameter
	xhM = MatrixXd::Zero(hiddenLayerSize, inputLayerSize);
	xhV = MatrixXd::Zero(hiddenLayerSize, inputLayerSize);
	hhM = MatrixXd::Zero(hiddenLayerSize, hiddenLayerSize);
	hhV = MatrixXd::Zero(hiddenLayerSize, hiddenLayerSize);
	bM = VectorXd::Zero(hiddenLayerSize);
	bV = VectorXd::Zero(hiddenLayerSize);
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
	
	VectorXd dh_prev(hiddenLayerSize);
	for (int i = 0; i < hiddenLayerSize; i++)
		dh_prev(i) = mul(hhWeight.col(i), dt).sum();
	
	// update Weight
	for (int i = 0; i < hiddenLayerSize; i++) {
		// printf("dt: %lf\n", dt[i]);
		// cout << "x: " << x.transpose() << endl;
		// cout << "h_prev: " << h_prev.transpose() << endl;
		xhWeight.row(i) += -ETA * dt[i] * x;
		hhWeight.row(i) += -ETA * dt[i] * h_prev;
	}
	bias += -ETA * dt;
	
	//＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊//
	// Adam
	VectorXd g, mt, vt;
	for (int i = 0; i < hiddenLayerSize; i++) {
		g = dt[i] * x;
		xhM.row(i) = B1*(VectorXd)xhM.row(i) + (1-B1)*g;
		xhV.row(i) = B2*(VectorXd)xhV.row(i) + (1-B2)*mul(g, g);
			
		mt = xhM.row(i)/(1-B1);
		vt = xhV.row(i)/(1-B2);
		xhWeight.row(i) += -ETA * mul(mt, reciproc(sqrt(vt + EPSILON)));
		
		g = dt[i] * h_prev;
		hhM.row(i) = B1*(VectorXd)hhM.row(i) + (1-B1)*g;
		hhV.row(i) = B2*(VectorXd)hhV.row(i) + (1-B2)*mul(g, g);
		
		mt = hhM.row(i)/(1-B1);
		vt = hhV.row(i)/(1-B2);
		hhWeight.row(i) += -ETA * mul(mt, reciproc(sqrt(vt + EPSILON)));
	}
	
	g = dt;
	bM = B1*bM + (1-B1)*g;
	bV = B2*bV + (1-B2)*mul(g, g);
	
	mt = (1/(1-B1)) * bM;
	vt = (1/(1-B2)) * bV;
	bias += -ETA * mul(mt, reciproc(sqrt(vt + EPSILON)));
	
	//＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊//
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