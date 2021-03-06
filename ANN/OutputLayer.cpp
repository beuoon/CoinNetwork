#include "OutputLayer.h"

#include <iostream>

OutputLayer::OutputLayer(int _hiddenLayerSize, int _outputLayerSize) {
	hiddenLayerSize = _hiddenLayerSize;
	outputLayerSize = _outputLayerSize;
	
    double weightLimit = sqrt(6.0/(hiddenLayerSize+1)); // He 초기화 사용 변수
	weight = MatrixXd::Random(outputLayerSize, hiddenLayerSize+1) * weightLimit;
	weight.col(hiddenLayerSize) = VectorXd::Zero(outputLayerSize); // bias
}
OutputLayer::OutputLayer(int _hiddenLayerSize, int _outputLayerSize, NetworkManager &in) {
	hiddenLayerSize = _hiddenLayerSize;
	outputLayerSize = _outputLayerSize;
	
	weight.resize(outputLayerSize, hiddenLayerSize+1);
	
	// load weight
	for (int i = 0; i < outputLayerSize; i++) {
		for (int j = 0; j <= hiddenLayerSize; j++)
			in >> weight(i, j);
	}
}

VectorXd OutputLayer::forward(VectorXd _h) {
	h = VectorXd::Zero(hiddenLayerSize+1);
	h.segment(0, hiddenLayerSize) = _h;
	h(hiddenLayerSize) = 1; // Bias
	
	VectorXd y = weight * h;
	y = softmax(y);
	
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

NetworkManager& operator<<(NetworkManager& out, const OutputLayer &layer) {// load weight
	for (int i = 0; i < layer.outputLayerSize; i++) {
		for (int j = 0; j <= layer.hiddenLayerSize; j++)
			out << layer.weight(i, j) << " ";
	}
	return out;
}