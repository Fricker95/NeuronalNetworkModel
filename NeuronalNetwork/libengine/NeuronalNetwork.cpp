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
	// initialize threadpool
	threadpool = new ThreadPool<NeuronThread, NeuronArg, void*>(MAX_THREADS);
}

NeuronalNetwork::NeuronalNetwork(std::vector<int> layers)
{
	/*
		Constructor initializes neurons and estabilshed neighbors and postsynaptic shcematics of the network
	*/
	layers_sizes = std::move(layers);
	int sum_neurons = std::accumulate(layers_sizes.begin(), layers_sizes.end(), 0);
	
	Initialize(sum_neurons);
	
	// initialize threadpool
	threadpool = new ThreadPool<NeuronThread, NeuronArg, void*>(MAX_THREADS, sum_neurons);
}

NeuronalNetwork::NeuronalNetwork(std::initializer_list<int> layers)
{
	/*
	 Constructor initializes neurons and estabilshed neighbors and postsynaptic shcematics of the network
	 */
	layers_sizes = std::move(layers);
	int sum_neurons = std::accumulate(layers_sizes.begin(), layers_sizes.end(), 0);
	
	Initialize(sum_neurons);
	
	// initialize threadpool
	threadpool = new ThreadPool<NeuronThread, NeuronArg, void*>(MAX_THREADS, sum_neurons);
}

NeuronalNetwork::~NeuronalNetwork() 
{
	/*
		deconstructor
	*/
	if (threadpool) {
		delete threadpool;
	}
}

std::vector<double> NeuronalNetwork::Start() noexcept
{
	/*
		Performs Voltage clamp on layer 1
		resues the threadpool to compute each layer sequentially
	*/
	// start and stop variables to store time stamps
	std::chrono::time_point<std::chrono::system_clock> start, end;
	
	
	int count = 0;
	// sum of neurons in all the layers
	int sum_neurons = std::accumulate(layers_sizes.begin(), layers_sizes.end(), 0);
	// iterates over the number of bins
	for (int t = 0; t < constants::num_bins; ++t) {
		// start time stamp for each bin
		start = std::chrono::system_clock::now();
		// iterates over the number of layers
		for (int i = 0; i < layers_sizes.size(); i++) {
			count += layers_sizes[i];
			// branchless calculation of initial index of each layer i the array of total neurons in the system
			int branchless = ((i == 0) ? 0 : (i == layers_sizes.size() - 1) ? sum_neurons - layers_sizes.back() : layers_sizes[i - 1]);
			// iterate over an individual layer
			for (int j = branchless; j < layers_sizes[i] + branchless; j++) {
				// if iterating over the first layer
				if (i == 0) {
					// voltage clamp neuron in first layer
					neurons[j].InjectCurrent(voltage_clamp);
				}
				// add tasks to threadpool
				threadpool->set_task<Neuron*, double>(&neurons[j], dt);
			}
			// start thread pool
			threadpool->start();
			// wait until threads have joined
			threadpool->join();
		}
		
		// every 10 bins
		if (t % 10 == 0) {
			// calculate end time
			end = std::chrono::system_clock::now();
			
			// calculate time difference
			std::chrono::duration<double> elapsed_seconds = end - start;
			// print time step percentage and time difference
			std::cout << "time step: " << (((double)t)/((double)constants::num_bins)) * 100 << "%, elapsed time: " << elapsed_seconds.count() << "s\n";
		}
	}
	
	// moves the history log of the last neuron in the last layer
	// used to graph the membrane potential
	return std::move(neurons[neurons.size() - 1].GetHistory());
}

const void NeuronalNetwork::Stop() noexcept
{
	// asks theadpool to stop execution and clears the queue
	threadpool->stop();
	threadpool->clear();
}

const void NeuronalNetwork::Cancel() noexcept
{
	// forces theadpool to cancel execution
	// does not clear allocated objects
	threadpool->cancel();
	Stop();
}

const bool NeuronalNetwork::Stopped() noexcept
{
	// checks if threadpool has stopped
	return threadpool->stopped();
}

pthread_mutex_t* NeuronalNetwork::ResultMutex() noexcept
{
	// returns result mutex
	return &s_result_m;
}

pthread_mutex_t* NeuronalNetwork::QueueMutex() noexcept
{
	// returns queue mutex
	return &s_queue_m;
}

const void NeuronalNetwork::Initialize(int sum_neurons) noexcept
{
	for (int i = 0; i < sum_neurons; i++) {
		// initialize Neuron with random output current and neighboring current
		neurons.emplace_back(Neuron(Rand(0.01, 0.05), Rand(0.001, 0.005)));
	}
	
	int count = 0;
	
	for (int i = 0; i < layers_sizes.size(); i++) {
		for (int j = count; j < layers_sizes[i]; j++) {
			for (int m = count; m < layers_sizes[i]; m++) {
				if (j != m) {
					// adds all neighboring neurons in the layer
					neurons[j].AddNeighbor(&neurons[m]);
				}
			}
			count++;
		}
	}
	
	count = 0;
	
	for (int i = 0; i < layers_sizes.size() - 1; i++) {
		count += layers_sizes[i];
		// branchless initial index of layer
		int branchless = ((i == 0) ? 0 : (i == layers_sizes.size() - 1) ? sum_neurons - layers_sizes.back() : layers_sizes[i - 1]);
		for (int j = branchless; j < layers_sizes[i] + branchless; j++) {
			// assigns postsynaptic neuron
			// using a modulo
			neurons[j].AddPostsynapticNeuron(&neurons[count + j % layers_sizes[i + 1]]);
		}
	}
}

NeuronalNetwork::NeuronArg::NeuronArg() {}

NeuronalNetwork::NeuronArg::NeuronArg(Neuron* neuron, double dt)
{
	this->neuron = neuron;
	this->dt = dt;
}

// move constructor
NeuronalNetwork::NeuronArg::NeuronArg(NeuronArg&& other): neuron(std::move(other.neuron)), dt(std::move(other.dt)) {}

NeuronalNetwork::NeuronArg::~NeuronArg() {}

NeuronalNetwork::NeuronThread::NeuronThread(std::vector<NeuronArg>* queue, void* results, std::atomic<int>* count)
{
	this->queue = queue;
	this->results = results;
	this->count = count;
	
}

// move construtor
NeuronalNetwork::NeuronThread::NeuronThread(NeuronThread&& other): queue(std::move(other.queue)), results(std::move(other.results)), count(std::move(other.count)) {}

NeuronalNetwork::NeuronThread::~NeuronThread() {}

void* NeuronalNetwork::NeuronThread::run() noexcept 
{
	/*
		overwritten virtual run function
		pulls neuron argument ( neuron pointer and delta t) from queue
		performs calculation of membrane potential
	*/
	NeuronArg arg;

	while (true) {
		// lock queue mutex
		pthread_mutex_lock(&s_queue_m);
		// breaks while loop if queue is empty or stopped flag is triggered
		if (queue->empty() || stopped()) {
			// unlock queue mutex
			pthread_mutex_unlock(&s_queue_m);
			break;
		}
		// moves neuron argument from back of queue to local variable
		arg = std::move(queue->back());
		// pop last element in the queue
		queue->pop_back();
		// unlock queue mutex
		pthread_mutex_unlock(&s_queue_m);
		
		if (arg.neuron) {
			// update membrane potential
			arg.neuron->Process(arg.dt);
		}
		// increments count
		(*count)++;
	}
	
	return NULL;
}






