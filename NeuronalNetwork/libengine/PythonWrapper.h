//
//  PythonWrapper.h
//  NeuronalNetwork
//
//  Created by Nicolas Fricker on 04/10/20.
//  Copyright © 2019 Nicolas Fricker. All rights reserved.
//

#ifndef PythonWrapper_
#define PythonWrapper_

#pragma GCC visibility push(default)

#include "NeuronalNetwork.h"

#include <iostream>
#include <cmath>
#include <vector>
#include <chrono>

extern "C" const double* run(const double x = 0.451, const double dt = 0.01, const int size = 10000, int* layers = nullptr, int n = 0);

#pragma GCC visibility pop
#endif /* PythonWrapper_ */
