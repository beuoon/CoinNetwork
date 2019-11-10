#pragma once

#include <Eigen/Dense>
#include <cmath>

using namespace Eigen;

VectorXd mul(const VectorXd &_v1, const VectorXd &_v2);
VectorXd sqrt(const VectorXd &_v);
VectorXd reciproc(const VectorXd &_v);
const VectorXd operator+(const VectorXd& _v, const double &d);

double sigmoid(const double &_d);
VectorXd sigmoid(const VectorXd &_v);
VectorXd sigmoid_diff(const VectorXd &_v);

VectorXd tanh(const VectorXd &_v);
VectorXd tanh_diff(const VectorXd &_v);
