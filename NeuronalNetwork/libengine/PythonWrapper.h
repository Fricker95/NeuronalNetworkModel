//
//  PythonWrapper.h
//  NeuronalNetwork
//
//  Created by Nicolas Fricker on 04/10/20.
//  Copyright © 2020 Nicolas Fricker. All rights reserved.
//

#ifndef PythonWrapper_
#define PythonWrapper_

#pragma GCC visibility push(default)

#include "NeuronalNetwork.h"

#include <vector>

class MyNN: public NeuronalNetwork
{
	/*
		Child class of Neuronal Network
		Used to overwrite the virtual InitializeNetwork function
	*/
	
	// layers in the network for linear model (in this case)
	std::vector<int> layers = {16, 4, 1};
	// array of entire neurons in the entire network
	std::vector<Neuron>* neurons = nullptr;
	
public:
	MyNN(std::vector<int> layers);
	~MyNN();
	
	void InitializeNetwork();
};

static double* VOLTAGES;

extern "C" const void initialize(int n);
extern "C" const void deinitialize();
extern "C" const double* run(const double x = 0.451, const double dt = 0.01, const int size = 10000, int* layers = nullptr, int n = 0);

#pragma GCC visibility pop
#endif /* PythonWrapper_ */
