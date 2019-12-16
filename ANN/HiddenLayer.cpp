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
}

NetworkManager& operator<<(NetworkManager& out, const HiddenLayer& layer) {
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