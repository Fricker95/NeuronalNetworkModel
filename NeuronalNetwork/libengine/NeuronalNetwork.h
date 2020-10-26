//
//  NeuronalNetwork.h
//  NeuronalNetwork
//
//  Created by Nicolas Fricker on 04/10/20.
//  Copyright © 2019 Nicolas Fricker. All rights reserved.
//

#ifndef NeuronalNetwork_
#define NeuronalNetwork_

#pragma GCC visibility push(hidden)

#include "Neuron.h"
#include "ThreadPool.hpp"

#include <iostream>
#include <cmath>
#include <numeric>
#include <vector>
#include <unordered_map>

#include <pthread.h>

#define MAX_THREADS 16

const double Rand(const double min, const double max);

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
	inline static double voltage_clamp = 0.451;
	// µs time step;
	inline static double dt = 0.01;
	// number of bins, num_bins * dt = µs
	inline static int num_bins = 10000;
	// number of neighboring neurons
	inline static int max_neighbors = 10000;

public:
	NeuronalNetwork();
	NeuronalNetwork(std::vector<int> layers);
	NeuronalNetwork(std::initializer_list<int> layers);
	~NeuronalNetwork();

	const void Start() noexcept;
	const void Stop() noexcept;
	const void Cancel() noexcept;
	
	std::vector<Neuron>& GetNeurons() noexcept;

	static const bool Stopped() noexcept;

	static const void SetVoltageClamp(const double vc) noexcept;
	static const void SetDeltaTime(const double dt) noexcept;
	static const void SetNumBins(const int nb) noexcept;
	static const void SetMaxNeighbors(const int mn) noexcept;
	static pthread_mutex_t* ResultMutex() noexcept;
	static pthread_mutex_t* QueueMutex() noexcept;

private:
	const void Initialize(int sum_neurons) noexcept;
	
	struct NeuronArg
	{
	public:
		// Neuron pointer
		Neuron* neuron = nullptr;
		// delta time
		double dt = 0;

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

inline const double Rand(const double min, const double max)
{
	/*
	 Random function
	 return a double within the min and max
	 */
	const double f = (double)rand() / RAND_MAX;
	return min + f * (max - min);
}

#pragma GCC visibility pop
#endif /* NeuronalNetwork_ */
