//
//  PythonWrapper.cpp
//  NeuronalNetwork
//
//  Created by Nicolas Fricker on 04/10/20.
//  Copyright © 2020 Nicolas Fricker. All rights reserved.
//

#include "PythonWrapper.h"

#include <iostream>
#include <cmath>
#include <chrono>

MyNN::MyNN(std::vector<int> layers): NeuronalNetwork(layers), layers(std::move(layers)), neurons(&GetNeurons()) {}

MyNN::~MyNN() {}

void MyNN::InitializeNetwork()
{
	/*
		Overwritten method to initialize network (in this case it is a linear model)
	*/
	int count = 0;
	
	for (int i = 0; i < layers.size(); i++) {
		for (int j = count; j < layers[i]; j++) {
			for (int m = count; m < layers[i]; m++) {
				if (j != m) {
					// adds all neighboring neurons in the layer
					(*neurons)[j].AddNeighbor(&(*neurons)[m]);
				}
			}
			count++;
		}
	}
	
	count = 0;
	
	for (int i = 0; i < layers.size() - 1; i++) {
		count += layers[i];
		// branchless initial index of layer
		int branchless = ((i == 0) ? 0 : (i == layers.size() - 1) ? (int)neurons->size() - layers.back() : layers[i - 1]);
		for (int j = branchless; j < layers[i] + branchless; j++) {
			// assigns postsynaptic neuron using a modulo
			(*neurons)[j].AddPostsynapticNeuron(&(*neurons)[count + j % layers[i + 1]]);
		}
	}
}

const void initialize(int n)
{
	if (VOLTAGES) {
		deinitialize();
	}
	VOLTAGES = new double[n];
}

const void deinitialize()
{
	if (VOLTAGES) {
		delete[] VOLTAGES;
	}
}

const double* run(const double x, const double dt, const int size, int* layers, int n)
{
	// set static Voltage clamp current [µA]
	NeuronalNetwork::SetCurrentClamp(x);
	// set static time step duration in [ms]
	NeuronalNetwork::SetTimeStep(dt);
	// set static number of iterations
	NeuronalNetwork::SetNumBins(size);
	
	// simulation time = num_bins * ∆t
	
	// declaration of variables for execution time
	std::chrono::time_point<std::chrono::system_clock> start, end;
	// stores start time
	start = std::chrono::system_clock::now();
	
	// {256,64,16,4,1}
	// network initialization
	MyNN network(std::vector<int>(layers, layers + n));
	
	// start iterating through the bins and injecting current into clamped neurons
	network.Start();
	
	// stores end time
	end = std::chrono::system_clock::now();
	// ∆t for execution time
	std::chrono::duration<double> elapsed_seconds = end - start;
	
	// outputs execution time
	std::cout << "elapsed time: " << elapsed_seconds.count() << "s\n";
	
	// outputs termination of program
	printf("Done\n");
	
	// retrieves vector reference of all neurons in the network
	std::vector<Neuron>* neurons = &network.GetNeurons();

	initialize((int)(neurons->size() * size));
	
	for (int i = 0; i < neurons->size(); i++) {
		for (int j = 0; j < size; j++) {
			VOLTAGES[(i * (*neurons)[i].GetHistorySize()) + j] = (*neurons)[i].GetHistory()[j];
		}
	}

	return VOLTAGES;
}
