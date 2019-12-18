#include "DQRN.h"

#include <iostream>

DQRN::DQRN(int _inputLayerNum, int _hiddenLayerNum, int _inputLayerSize, int _hiddenLayerSize, int _outputLayerSize) {
	inputLayerNum = _inputLayerNum;
	hiddenLayerNum = _hiddenLayerNum;
	inputLayerSize = _inputLayerSize;
	hiddenLayerSize = _hiddenLayerSize;
	outputLayerSize = _outputLayerSize;
	
	for (int i = 0; i < hiddenLayerNum; i++)
		hiddenLayers.push_back(new HiddenLayer(inputLayerSize, hiddenLayerSize));
	outputLayer = new OutputLayer(hiddenLayerSize, outputLayerSize);
}
DQRN::DQRN(NetworkManager &in) {
	in >> inputLayerNum >> hiddenLayerNum;
	in >> inputLayerSize >> hiddenLayerSize >> outputLayerSize;
	
	for (int i = 0; i < hiddenLayerNum; i++)
		hiddenLayers.push_back(new HiddenLayer(inputLayerSize, hiddenLayerSize, in));
	outputLayer = new OutputLayer(hiddenLayerSize, outputLayerSize, in);
}
DQRN::~DQRN() {
	for (int i = 0; i < hiddenLayerNum; i++)
		delete hiddenLayers[i];
	delete outputLayer;
}
	
double DQRN::train(vector<VectorXd> _inputs, VectorXd _label) {
	double error = 0;
	VectorXd output = forward(_inputs);
	
	VectorXd delta(outputLayerSize);
	for (int i = 0; i < outputLayerSize; i++) {
		delta[i] = output[i] - _label[i];
		error += -_label[i]*log(output[i]); // cross-entropy loss
	}
	error /= outputLayerSize;
	
	backward(delta);
	
	return error;
}
double DQRN::loss(vector<VectorXd> _inputs, VectorXd _label) {
	double error = 0;
	VectorXd output = forward(_inputs);
	
	VectorXd delta(outputLayerSize);
	for (int i = 0; i < outputLayerSize; i++) {
		delta[i] = output[i] - _label[i];
		error += -_label[i]*log(output[i]); // cross-entropy loss
	}
	error /= outputLayerSize;
	
	return error;
}
	
VectorXd DQRN::forward(vector<VectorXd> _inputs) {
	VectorXd h_prev = VectorXd::Zero(hiddenLayerSize);
	
	// Hidden Layer
	for (int i = 0; i < hiddenLayerNum; i++) {
		VectorXd x = (i < inputLayerNum) ? _inputs[i] : VectorXd::Zero(inputLayerSize);
		
		hiddenLayers[i]->forward(x, h_prev);
	}
	
	// Output Layer
	VectorXd h = hiddenLayers[hiddenLayerNum-1]->getH();
	VectorXd output = outputLayer->forward(h);

	return output;
}
void DQRN::backward(VectorXd _delta) {
	VectorXd dh_next = VectorXd::Zero(hiddenLayerSize);
	
	// OutputLayer
	VectorXd dy = outputLayer->backward(_delta);
	hiddenLayers[hiddenLayerNum-1]->backward(dy, dh_next);
	
	// HiddenLayer
	dy = VectorXd::Zero(hiddenLayerSize);
	for (int i = hiddenLayerNum-2; i >= 0; i--)
		hiddenLayers[i]->backward(dy, dh_next);
}

NetworkManager& operator<<(NetworkManager& out, const DQRN &network) {
	out << network.inputLayerNum << " " << network.hiddenLayerNum << "\n";
	out << network.inputLayerSize << " " << network.hiddenLayerSize << " " << network.outputLayerSize << "\n";
	
	for (int i = 0; i < network.hiddenLayerNum; i++)
		out << *network.hiddenLayers[i] << "\n";
	out << *network.outputLayer << "\n";
	
	return out;
}