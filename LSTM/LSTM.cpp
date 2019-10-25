#include "LSTM.h"

#include <iostream>

LSTM::LSTM(int _inputLayerNum, int _hiddenLayerNum, int _outputLayerNum, 
		   int _inputLayerSize, int _hiddenLayerSize, int _outputLayerSize) {
	inputLayerNum = _inputLayerNum;
	hiddenLayerNum = _hiddenLayerNum;
	outputLayerNum = _outputLayerNum;
	inputLayerSize = _inputLayerSize;
	hiddenLayerSize = _hiddenLayerSize;
	outputLayerSize = _outputLayerSize;
	
	for (int i = 0; i < hiddenLayerNum; i++)
		hiddenLayers.push_back(new HiddenLayer(inputLayerSize, hiddenLayerSize));
	for (int i = 0; i < outputLayerNum; i++)
		outputLayers.push_back(new OutputLayer(hiddenLayerSize, outputLayerSize));
}
LSTM::LSTM(NetworkManager &in) {
	in >> inputLayerNum >> hiddenLayerNum >> outputLayerNum;
	in >> inputLayerSize >> hiddenLayerSize >> outputLayerSize;
	
	for (int i = 0; i < hiddenLayerNum; i++)
		hiddenLayers.push_back(new HiddenLayer(inputLayerSize, hiddenLayerSize, in));
	for (int i = 0; i < outputLayerNum; i++)
		outputLayers.push_back(new OutputLayer(hiddenLayerSize, outputLayerSize, in));
}
LSTM::~LSTM() {
	for (int i = 0; i < hiddenLayerNum; i++)
		delete hiddenLayers[i];
	for (int i = 0; i < outputLayerNum; i++)
		delete outputLayers[i];
}
	
double LSTM::train(vector<VectorXd> _inputs, vector<VectorXd> _labels) {
	double error = 0;
	vector<VectorXd> outputs = forward(_inputs);
	
	vector<VectorXd> deltas;
	for (int i = 0; i < outputLayerNum; i++) {
		VectorXd delta(outputLayerSize);
		
		for (int j = 0; j < outputLayerSize; j++) {
			delta[j] = outputs[i][j] - _labels[i][j];
			error += 0.5*delta[j]*delta[j];
		}
		deltas.push_back(delta);
	}
	
	backward(deltas);
	
	return error;
}
	
vector<VectorXd> LSTM::forward(vector<VectorXd> _inputs) {
	VectorXd h_prev(hiddenLayerSize);
	VectorXd c_prev(hiddenLayerSize);
	
	// Hidden Layer
	for (int i = 0; i < hiddenLayerNum; i++) {
		VectorXd x = (i < inputLayerNum) ? _inputs[i] : VectorXd::Zero(inputLayerSize);
		
		hiddenLayers[i]->forward(x, h_prev, c_prev);
		
		if (i == 0) {
			this->h_prev = h_prev;
			this->c_prev = c_prev;
		}
	}
	
	// Output Layer
	vector<VectorXd> outputs;
	for (int i = 0, j = hiddenLayerNum - outputLayerNum; i < outputLayerNum; i++, j++) {
		VectorXd h = hiddenLayers[j]->getH();
		
		outputs.push_back(outputLayers[i]->forward(h));
	}

	return outputs;
}
void LSTM::backward(vector<VectorXd> _deltas) {
	VectorXd dh_next(hiddenLayerSize);
	VectorXd dc_next(hiddenLayerSize);
	
	for (int i = hiddenLayerNum-1, j = outputLayerNum-1; i >= 0; i--, j--) {
		VectorXd dy = (j >= 0) ? outputLayers[j]->backward(_deltas[j]) : VectorXd::Zero(hiddenLayerSize);
		
		hiddenLayers[i]->backward(dy, dh_next, dc_next);
	}
}

NetworkManager& operator<<(NetworkManager& out, const LSTM &network) {
	out << network.inputLayerNum << " " << network.hiddenLayerNum << " " << network.outputLayerNum << "\n";
	out << network.inputLayerSize << " " << network.hiddenLayerSize << " " << network.outputLayerSize << "\n";
	
	for (int i = 0; i < network.hiddenLayerNum; i++)
		out << *network.hiddenLayers[i] << "\n";
	for (int i = 0; i < network.outputLayerNum; i++)
		out << *network.outputLayers[i] << "\n";
	
	return out;
}