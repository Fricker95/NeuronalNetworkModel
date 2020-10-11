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
	for (int i = 0; i < layers_sizes.size(); ++i) {
		for (int j = 0; j < layers_sizes[i]; ++j) {
			if (i == 0) {
				layer1.emplace_back(new Neuron);
			}
		}
	}

	threadpool = new ThreadPool<NeuronThread, NeuronArg, std::unordered_map<Neuron*, double*>>(MAX_THREADS, (int)std::accumulate(layers_sizes.begin(), layers_sizes.end(), 0));
}

NeuronalNetwork::~NeuronalNetwork() 
{
	for (int m = 0; m < layer1.size(); ++m) {
		if (layer1[m]) {
			delete layer1[m];
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
		for (int i = 0; i < layer1.size(); ++i) {
			// printf("0x%llx\n", (uint64_t)layer1[i]);
			if (layer1[i]) {
				layer1[i]->InjectCurrent(voltage_clamp);
				threadpool->set_task<Neuron*, double>(layer1[i], dt);
			}
		}
		threadpool->start();
		if (!threadpool->stopped() && threadpool->started()) {
			threadpool->join();
			threadpool->clear();
		}
		printf("time step %i\n", t);
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
			// printf("Break\n");
			break;
		}
		
		pthread_mutex_lock(&s_queue_m);
		// printf("Queue size = %lu, %i\n", queue->size(), queue->empty());
		arg = queue->back();
		queue->pop_back();
		pthread_mutex_unlock(&s_queue_m);
		
		// arg.neuron->Process(arg.dt);

		// printf("0x%llx Done\n", (uint64_t)arg.neuron);
		
		// pthread_mutex_lock(&s_result_m);
		// (*arg.results)[arg.neuron] = arg.neuron->GetHistory();
		// pthread_mutex_unlock(&s_result_m);

	}
	
	pthread_exit(NULL);
}






