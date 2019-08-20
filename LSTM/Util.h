#pragma once

#include <Eigen/Dense>
#include <cmath>

using namespace Eigen;

VectorXd mul(const VectorXd &_v1, const VectorXd &_v2);

double sigmoid(const double &_d);
VectorXd sigmoid(const VectorXd &_v);
VectorXd sigmoid_diff(const VectorXd &_v);

VectorXd tanh(const VectorXd &_v);
VectorXd tanh_diff(const VectorXd &_v);
