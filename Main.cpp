#include <iostream>
#include <fstream>
#include <Eigen/Dense>
#include <ctime>
#include <cmath>

#include "LSTM/LSTM.h"

using namespace Eigen;
using namespace std;

int main() {
	/*
	srand((unsigned)time(NULL));
	
	int inputSize = 1, outputSize = 1;
	int inputNum = 3, outputNum = 1;
	
	double inputArr[] = {0.01, 0.02, 0.03, 0.04, 0.05, 0.06};
	double labelArr[] = {0.01, 0.07, 0.08, 0.07};
	int dataNum = 4;
	vector<VectorXd> inputs, labels;
	for (int i = 0; i < 6; i++) {
		VectorXd input(inputSize); input << inputArr[i];
		inputs.push_back(input);
	}
	for (int i = 0; i < 4; i++) {
		VectorXd label(outputSize); label << labelArr[i];
		labels.push_back(label);
	}
	
	LSTM network(inputNum, 10, outputNum, inputSize, 10, outputSize);
	
	cout << "Before: " << endl;
	for (int j = 0; j < dataNum; j++) {
		vector<VectorXd> input(inputs.begin()+j, inputs.begin()+j+inputNum);
		cout << network.predict(input)[0] << endl;
	}
	
	for (int i = 0; i < 1000; i++) {
		for (int j = 0; j < dataNum; j++) {
			vector<VectorXd> input(inputs.begin()+j, inputs.begin()+j+inputNum);
			vector<VectorXd> label(labels.begin()+j, labels.begin()+j+outputNum);
			network.train(input, label);
		}
	}
	
	cout << "After: " << endl;
	for (int j = 0; j < dataNum; j++) {
		vector<VectorXd> input(inputs.begin()+j, inputs.begin()+j+inputNum);
		cout << network.predict(input)[0] << endl;
	}
	*/
	
	
	ifstream is("network");
	LSTM network(is);
	is.close();
	
	VectorXd inputData(6); inputData << 0.1, 0.2, 0.3, 0.4, 0.5, 0.6;
	VectorXd labelData(6); labelData << 0.1, 0.2, 0.3, 0.4, 0.5, 0.6;
	vector<VectorXd> input, label;
	input.push_back(inputData);
	label.push_back(labelData);
	
	cout << "Before" << endl;
	cout << network.predict(input)[0].transpose() << endl;
	
	for (int i = 0; i < 10000; i++)
		network.train(input, label);
	
	cout << "After" << endl;
	cout << network.predict(input)[0].transpose() << endl;
	
	
	return 0;
}
