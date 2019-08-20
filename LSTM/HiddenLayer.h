#pragma once

#include <Eigen/Dense>
#include <fstream>
#include "Util.h"

using namespace Eigen;
using namespace std;

class HiddenLayer {
public:
	HiddenLayer(int _inputLayerSize, int _hiddenLayerSize);
	HiddenLayer(int _inputLayerSize, int _hiddenLayerSize, istream &is);
	
	void forward(VectorXd _x, VectorXd &_h_prev, VectorXd &_c_prev);
	void backward(VectorXd _dy, VectorXd &_dh_next, VectorXd &_dc_next);
	
	VectorXd getH() { return h; }
	
private:
	const double ETA = 0.005;
	
	int inputLayerSize, hiddenLayerSize;
	
    VectorXd x, h_prev, c_prev,
			f, i, g, o,
			h, c;
	
	MatrixXd xhWeight, hhWeight;
	VectorXd bias;
};