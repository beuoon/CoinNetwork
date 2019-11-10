#include "Util.h"

VectorXd mul(const VectorXd &_v1, const VectorXd &_v2) {
	int size = (_v1.size() < _v2.size()) ? _v1.size() : _v2.size();
	
	VectorXd v(size);
	for (int i = 0; i < size; i++)
		v[i] = _v1[i] * _v2[i];
	
	return v;
}
VectorXd sqrt(const VectorXd &_v) {
	int size = _v.size();
	
	VectorXd v(size);
	for (int i = 0; i < size; i++)
		v[i] = sqrt(_v[i]);
	
	return v;
}
VectorXd reciproc(const VectorXd &_v) {
	int size = _v.size();
	
	VectorXd v(size);
	for (int i = 0; i < size; i++)
		v[i] = 1/_v[i];
	
	return v;
}
const VectorXd operator+(const VectorXd& _v, const double &d) {
	int size = _v.size();
	
	VectorXd v(size);
	for (int i = 0; i < size; i++)
		v[i] = _v[i] + d;
	
	return v;
}

double sigmoid(const double &_d) {
	return 1 / (1 + exp(-_d));
}
VectorXd sigmoid(const VectorXd &_v) {
	int size = _v.size();
	
	VectorXd v(size);
	for (int i = 0; i < size; i++)
		v[i] = sigmoid(_v[i]);
	
	return v;
}
VectorXd sigmoid_diff(const VectorXd &_v) {
	int size = _v.size();
	
	VectorXd v(size);
	for (int i = 0; i < size; i++) {
		double fx = sigmoid(_v[i]);
		v[i] = fx * (1 - fx);
	}
	
	return v;
}

VectorXd tanh(const VectorXd &_v) {
	int size = _v.size();
	
	VectorXd v(size);
	for (int i = 0; i < size; i++)
		v[i] = tanh(_v[i]);
	
	return v;
}
VectorXd tanh_diff(const VectorXd &_v) {
	int size = _v.size();
	
	VectorXd v(size);
	for (int i = 0; i < size; i++)
		v[i] = 1 - pow(tanh(_v[i]), 2);
	
	return v;
}
