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

#define MAX_THREADS 16

class NeuronalNetwork
{
	// forward declaration of argument class
	class NeuronArg;
	// forward delcaration of thread class
	class NeuronThread;
	
	// size of layers
	std::vector<int> layers_sizes;
	
	// array of neurons in entire system
	std::vector<Neuron> neurons;
	
	// static threadpool pointer
	inline static ThreadPool<NeuronThread, NeuronArg, void*>* threadpool = nullptr;
	
	// static mutexes
	inline static pthread_mutex_t s_queue_m = PTHREAD_MUTEX_INITIALIZER;
	inline static pthread_mutex_t s_result_m = PTHREAD_MUTEX_INITIALIZER;
	
	// mV
	inline constexpr static const double voltage_clamp = 0.451;
	// µs time step;
	inline constexpr static const double dt = 0.01;

public:
	NeuronalNetwork();
	NeuronalNetwork(std::vector<int> layers);
	NeuronalNetwork(std::initializer_list<int> layers);
	~NeuronalNetwork();

	std::vector<double> Start() noexcept;
	const void Stop() noexcept;
	const void Cancel() noexcept;

	static const  bool Stopped() noexcept;

	static pthread_mutex_t* ResultMutex() noexcept;
	static pthread_mutex_t* QueueMutex() noexcept;

private:
	const void Initialize(int sum_neurons) noexcept;
	
	class NeuronArg
	{
	public:
		// Neuron pointer
		Neuron* neuron = nullptr;
		// delta time
		double dt = 0;

	public:
		NeuronArg();
		NeuronArg(Neuron* neuron, const double dt);
		NeuronArg(NeuronArg&& other);
		~NeuronArg();

		NeuronArg& operator=(const NeuronArg& other) = default;
		
	};

	class NeuronThread: public ThreadPool<NeuronThread, NeuronArg, void*>::Thread_
	{
		// queue pointer of arguments
		std::vector<NeuronArg>* queue = nullptr;
		// result pointer (unused in this case)
		void* results = nullptr;
		// atomic count pointer
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
