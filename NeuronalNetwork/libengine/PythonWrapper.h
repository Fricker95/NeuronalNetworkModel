#include "Neuron.h"
#include "NeuronalNetwork.h"

#include <iostream>
#include <cmath>
#include <vector>
#include <chrono>

extern "C" const double* run(const double x = 0.451, const double dt = 0.01, const int size = 10000, int* layers = nullptr, int n = 0);
