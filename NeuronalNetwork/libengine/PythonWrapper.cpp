//
//  PythonWrapper.cpp
//  NeuronalNetwork
//
//  Created by Nicolas Fricker on 04/10/20.
//  Copyright © 2019 Nicolas Fricker. All rights reserved.
//

#include "PythonWrapper.h"

MyNN::MyNN(std::vector<int> layers): NeuronalNetwork(layers), layers(std::move(layers)), neurons(&GetNeurons()) {}

MyNN::~MyNN() {}

void MyNN::InitializeNetwork()
{
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

const double* run(const double x, const double dt, const int size, int* layers, int n)
{
	NeuronalNetwork::SetCurrentClamp(x);
	NeuronalNetwork::SetTimeStep(dt);
	NeuronalNetwork::SetNumBins(size);
	
	std::chrono::time_point<std::chrono::system_clock> start, end;
	
	start = std::chrono::system_clock::now();
	
//	{256,64,16,4,1}
	MyNN network(std::vector<int>(layers, layers + n));
	
	network.Start();
	
	end = std::chrono::system_clock::now();
	
	std::chrono::duration<double> elapsed_seconds = end - start;
	std::time_t end_time = std::chrono::system_clock::to_time_t(end);
	
	std::cout << "finished computation at " << std::ctime(&end_time)
	<< "elapsed time: " << elapsed_seconds.count() << "s\n";
	
	printf("Done");
	
	std::vector<Neuron>* neurons = &network.GetNeurons();

	double* voltages = new double[neurons->size() * size];
	
	for (int i = 0; i < neurons->size(); i++) {
		for (int j = 0; j < size; j++) {
			voltages[(i * (*neurons)[i].GetHistorySize()) + j] = (*neurons)[i].GetHistory()[j];
		}
	}

	return voltages;
}
