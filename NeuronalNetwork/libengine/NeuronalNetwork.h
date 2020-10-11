//
//  NeuronalNetwork.h
//  NeuronalNetwork
//
//  Created by Nicolas Fricker on 04/10/20.
//  Copyright © 2019 Nicolas Fricker. All rights reserved.
//

#ifndef NeuronalNetwork_
#define NeuronalNetwork_

#pragma GCC visibility push(default)

#include "Neuron.h"

#include "ThreadPool.hpp"

#include <iostream>
#include <cmath>
#include <numeric>
#include <vector>
#include <unordered_map>

#include <pthread.h>

#define MAX_THREADS 1

class NeuronalNetwork
{
	class NeuronArg;
	class NeuronThread;

	std::vector<int> layers_sizes = {16,4,1};

	std::vector<std::unordered_map<Neuron*, double*>> network;

	std::vector<Neuron*> layer1;


	// mV
	double voltage_clamp = 0.451;

	double dt = 0.01;

	// µs
	int duration = 10000;


	inline static ThreadPool<NeuronThread, NeuronArg, std::unordered_map<Neuron*, double*>>* threadpool = nullptr;

	inline static pthread_mutex_t s_queue_m = PTHREAD_MUTEX_INITIALIZER;
	inline static pthread_mutex_t s_result_m = PTHREAD_MUTEX_INITIALIZER;

public:
	explicit NeuronalNetwork();
	~NeuronalNetwork();

	void Start() noexcept;
	void Stop() noexcept;
	void Cancel() noexcept;

	static bool Stopped() noexcept;

	static pthread_mutex_t* ResultMutex() noexcept;
	static pthread_mutex_t* QueueMutex() noexcept;

private:
	class NeuronArg
	{
	public:
		Neuron* neuron = nullptr;
		double dt = 0;

	public:
		NeuronArg();
		NeuronArg(Neuron* neuron, double dt);
		NeuronArg(NeuronArg&& other);
		~NeuronArg();

		NeuronArg& operator=(const NeuronArg& other) = default;
		
	};

	class NeuronThread: public ThreadPool<NeuronThread, NeuronArg, void*>::Thread_
	{
		std::vector<NeuronArg>* queue = nullptr;
		void* results = nullptr;
		std::atomic<int>* count = nullptr;

	public:
		explicit NeuronThread(std::vector<NeuronArg>* queue, void* results, std::atomic<int>* count);
		NeuronThread(NeuronThread&& other);
		~NeuronThread();

		void* run() noexcept;
		
	};
	
};

#pragma GCC visibility pop
#endif /* NeuronalNetwork_ */
