//
//  NeuronalNetwork.cpp
//  NeuronalNetwork
//
//  Created by Nicolas Fricker on 04/10/20.
//  Copyright © 2019 Nicolas Fricker. All rights reserved.
//

#include "NeuronalNetwork.h"

NeuronalNetwork::NeuronalNetwork() 
{
	int sum_neurons = std::accumulate(layers_sizes.begin(), layers_sizes.end(), 0);
	for (int i = 0; i < sum_neurons; i++) {
		neurons.emplace_back(Neuron());
	}
	
	int count = 0;
	
	for (int i = 0; i < layers_sizes.size(); i++) {
		for (int j = count; j < layers_sizes[i]; j++) {
			for (int m = count; m < layers_sizes[i]; m++) {
				if (j != m) {
					neurons[j].AddNeighbor(&neurons[m]);
				}
			}
			count++;
		}
	}
	
	count = 0;
	
	for (int i = 0; i < layers_sizes.size() - 1; i++) {
		count += layers_sizes[i];
		int branchless = ((i == 0) ? 0 : (i == layers_sizes.size() - 1) ? sum_neurons - layers_sizes.back() : layers_sizes[i - 1]);
		for (int j = branchless; j < layers_sizes[i] + branchless; j++) {
			neurons[j].AddPostsynapticNeuron(&neurons[count + j % layers_sizes[i + 1]]);
		}
	}
	
	threadpool = new ThreadPool<NeuronThread, NeuronArg, std::unordered_map<Neuron*, double*>>(MAX_THREADS, sum_neurons);
}

NeuronalNetwork::~NeuronalNetwork() 
{
	if (!threadpool->stopped() && threadpool->started()) {
		threadpool->stop();
		threadpool->join();
		threadpool->clear();
		threadpool->results()->clear();
	}

	if (threadpool) {
		delete threadpool;
	}
}

void NeuronalNetwork::Start() noexcept
{
	int count = 0;
	int sum_neurons = std::accumulate(layers_sizes.begin(), layers_sizes.end(), 0);
	for (int t = 0; t < duration; ++t) {
		for (int i = 0; i < layers_sizes.size(); i++) {
			count += layers_sizes[i];
			int branchless = ((i == 0) ? 0 : (i == layers_sizes.size() - 1) ? sum_neurons - layers_sizes.back() : layers_sizes[i - 1]);
			for (int j = branchless; j < layers_sizes[i] + branchless; j++) {
				if (i == 0) {
					neurons[j].InjectCurrent(voltage_clamp);
				}
				threadpool->set_task<Neuron*, double>(&neurons[j], dt);
			}
			threadpool->start();
			threadpool->join();
		}
	}

	double* voltages;
	
	voltages = neurons[neurons.size() - 1].GetHistory();
	
	for (int i = 0; i < duration; ++i) {
		printf("%f\n", voltages[i]);
	}
}

void NeuronalNetwork::Stop() noexcept {
	threadpool->stop();
	threadpool->clear();
}

void NeuronalNetwork::Cancel() noexcept {
	threadpool->cancel();
	Stop();
}

bool NeuronalNetwork::Stopped() noexcept {
	return threadpool->stopped();
}

pthread_mutex_t* NeuronalNetwork::ResultMutex() noexcept {
	return &s_result_m;
}

pthread_mutex_t* NeuronalNetwork::QueueMutex() noexcept {
	return &s_queue_m;
}

NeuronalNetwork::NeuronArg::NeuronArg() {}

NeuronalNetwork::NeuronArg::NeuronArg(Neuron* neuron, double dt)
{
	this->neuron = neuron;
	this->dt = dt;
}

NeuronalNetwork::NeuronArg::NeuronArg(NeuronArg&& other): neuron(std::move(other.neuron)), dt(std::move(other.dt)) {}

NeuronalNetwork::NeuronArg::~NeuronArg() {}


NeuronalNetwork::NeuronThread::NeuronThread(std::vector<NeuronArg>* queue, void* results, std::atomic<int>* count) 
{
	this->queue = queue;
	this->results = results;
	this->count = count;
	
}

NeuronalNetwork::NeuronThread::NeuronThread(NeuronThread&& other): queue(std::move(other.queue)), results(std::move(other.results)), count(std::move(other.count)) {}

NeuronalNetwork::NeuronThread::~NeuronThread() {}

void* NeuronalNetwork::NeuronThread::run() noexcept 
{	
	NeuronArg arg;

	while (true) {
		pthread_mutex_lock(&s_queue_m);
		if (queue->empty() || stopped()) {
			pthread_mutex_unlock(&s_queue_m);
			break;
		}
		
		arg = std::move(queue->back());
		queue->pop_back();
		pthread_mutex_unlock(&s_queue_m);
		
		if (arg.neuron) {
			arg.neuron->Process(arg.dt);
		}
		
		(*count)++;
	}
	
	return NULL;
}






