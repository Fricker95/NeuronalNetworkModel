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

#include <pthread.h>

class NeuronalNetwork
{
	// forward declaration of argument class
	class NeuronArg;
	// forward delcaration of thread class
	class NeuronThread;
	
	// size of layers
	std::vector<int> layers_sizes_;
	
	// array of neurons in entire system
	std::vector<Neuron> neurons_;
	
	// static threadpool pointer
	inline static ThreadPool<NeuronThread, NeuronArg, void*>* sp_threadpool_ = nullptr;
	
	// static mutexes
	inline static pthread_mutex_t s_queue_m_ = PTHREAD_MUTEX_INITIALIZER;
	inline static pthread_mutex_t s_result_m_ = PTHREAD_MUTEX_INITIALIZER;
	
	// mV
	inline static double s_Iclamp_ = 0.451;
	// µs time step;
	inline static double s_dt_ = 0.01;
	// number of bins, num_bins * dt = µs
	inline static int s_num_bins_ = 10000;
	// number of neighboring neurons
	inline static int s_max_neighbors_ = 10000;

public:
	NeuronalNetwork();
	NeuronalNetwork(std::vector<int> layers);
	NeuronalNetwork(std::initializer_list<int> layers);
	template <typename... Args>
	NeuronalNetwork(Args... args);
	virtual ~NeuronalNetwork();
	
	virtual void InitializeNetwork();

	const void Start() noexcept;
	const void Stop() noexcept;
	const void Cancel() noexcept;
	
	const void AllocateNeurons(size_t n) noexcept;
	
	std::vector<Neuron>& GetNeurons() noexcept;

	static const bool Stopped() noexcept;

	static const void SetCurrentClamp(const double cc) noexcept;
	static const void SetTimeStep(const double dt) noexcept;
	static const void SetNumBins(const int nb) noexcept;
	static const void SetMaxNeighbors(const int mn) noexcept;

private:
	static pthread_mutex_t* ResultMutex() noexcept;
	static pthread_mutex_t* QueueMutex() noexcept;
	
	struct NeuronArg
	{
	public:
		// Neuron pointer
		Neuron* neuron_ = nullptr;
		// delta time
		double dt_ = 0;

		NeuronArg();
		NeuronArg(Neuron* neuron, const double dt);
		NeuronArg(NeuronArg&& other);
		~NeuronArg();

		NeuronArg& operator=(const NeuronArg& other) = default;
		
	};

	class NeuronThread: public ThreadPool<NeuronThread, NeuronArg, void*>::Thread_
	{
		// queue pointer of arguments
		std::vector<NeuronArg>* queue_ = nullptr;
		// result pointer (unused in this case)
		void* results_ = nullptr;
		// atomic count pointer
		std::atomic<int>* a_count_ = nullptr;

	public:
		explicit NeuronThread(std::vector<NeuronArg>* queue, void* results, std::atomic<int>* count);
		NeuronThread(NeuronThread&& other);
		~NeuronThread();

		void* run() noexcept;
	};
};

#pragma GCC visibility pop
#endif /* NeuronalNetwork_ */
