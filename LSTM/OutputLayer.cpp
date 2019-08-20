#include "OutputLayer.h"

#include <iostream>

OutputLayer::OutputLayer(int _hiddenLayerSize, int _outputLayerSize) {
	hiddenLayerSize = _hiddenLayerSize;
	outputLayerSize = _outputLayerSize;
	
    double weightLimit = sqrt(6.0/(hiddenLayerSize+1)); // He 초기화 사용 변수
	weight.resize(outputLayerSize, hiddenLayerSize+1);
	
	weight.setRandom();
	
	weight *= weightLimit;
}
OutputLayer::OutputLayer(int _hiddenLayerSize, int _outputLayerSize, istream &is) {
	hiddenLayerSize = _hiddenLayerSize;
	outputLayerSize = _outputLayerSize;
	
	weight.resize(outputLayerSize, hiddenLayerSize+1);
	
	// load weight
	for (int i = 0; i < outputLayerSize; i++) {
		for (int j = 0; j <= hiddenLayerSize; j++)
			is >> weight(i, j);
	}
}

VectorXd OutputLayer::forward(VectorXd _h) {
	h = VectorXd::Zero(hiddenLayerSize+1);
	h.segment(0, hiddenLayerSize) = _h;
	h(hiddenLayerSize) = 1; // Bias
	
	VectorXd y = weight * h;
	
	return y;
}
VectorXd OutputLayer::backward(VectorXd _delta) {
	VectorXd dh(hiddenLayerSize);
	for (int i = 0; i < hiddenLayerSize; i++)
		dh(i) = mul(weight.col(i), _delta).sum();
    
	for (int i = 0; i < outputLayerSize; i++)
		weight.row(i) += -ETA * _delta[i] * h;
	
	return dh;
}
