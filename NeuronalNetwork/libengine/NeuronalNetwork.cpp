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
	for (int i = 0; i < std::accumulate(layers_sizes.begin(), layers_sizes.end(), 0); ++i) {
		neurons.emplace_back(new Neuron);
	}

	threadpool = new ThreadPool<NeuronThread, NeuronArg, std::unordered_map<Neuron*, double*>>(MAX_THREADS, (int)std::accumulate(layers_sizes.begin(), layers_sizes.end(), 0));
}

NeuronalNetwork::~NeuronalNetwork() 
{
	for (int m = 0; m < neurons.size(); ++m) {
		if (neurons[m]) {
			delete neurons[m];
		}
	}
	
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
	for (int t = 0; t < duration; ++t) {
		for (int n = 0; n < layers_sizes.size(); ++n) {
			for (int i = ((n == 0) ? 0 : layers_sizes[n-1]); i < layers_sizes[n]; ++i) {
				if (neurons[i]) {
					if (n == 0) {
						neurons[i]->InjectCurrent(voltage_clamp);
					}
					threadpool->set_task<Neuron*, double>(neurons[i], dt);
				}
			}
			threadpool->start();
			threadpool->join();
		}
	}

	double* voltages;
	
	voltages = neurons[neurons.size()-1]->GetHistory();
	
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

NeuronalNetwork::NeuronArg::NeuronArg(NeuronArg&& other): neuron(other.neuron), dt(other.dt) {}

NeuronalNetwork::NeuronArg::~NeuronArg() {}


NeuronalNetwork::NeuronThread::NeuronThread(std::vector<NeuronArg>* queue, void* results, std::atomic<int>* count) 
{
	this->queue = queue;
	this->results = results;
	this->count = count;
}

NeuronalNetwork::NeuronThread::NeuronThread(NeuronThread&& other): queue(other.queue), results(other.results), count(other.count) {}

NeuronalNetwork::NeuronThread::~NeuronThread() {}

void* NeuronalNetwork::NeuronThread::run() noexcept 
{	
	NeuronArg arg;

	while (true) {
		if (queue->empty()) {
			break;
		}
		
		pthread_mutex_lock(&s_queue_m);
		arg = queue->back();
		queue->pop_back();
		pthread_mutex_unlock(&s_queue_m);
		
		arg.neuron->Process(arg.dt);
	}
	
	return NULL;
}






