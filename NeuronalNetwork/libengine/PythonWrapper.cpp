//
//  PythonWrapper.cpp
//  NeuronalNetwork
//
//  Created by Nicolas Fricker on 04/10/20.
//  Copyright © 2019 Nicolas Fricker. All rights reserved.
//

#include "PythonWrapper.h"

const double* run(const double x, const double dt, const int size, int* layers, int n)
{
	NeuronalNetwork::SetVoltageClamp(x);
	NeuronalNetwork::SetDeltaTime(dt);
	NeuronalNetwork::SetNumBins(size);
	
	std::chrono::time_point<std::chrono::system_clock> start, end;
	
	start = std::chrono::system_clock::now();
	
//	{256,64,16,4,1}
	NeuronalNetwork network(std::vector<int>(layers, layers + n));
	
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
