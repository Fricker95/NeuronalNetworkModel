//
//  NeuronalNetwork.cpp
//  NeuronalNetwork
//
//  Created by Nicolas Fricker on 04/10/20.
//  Copyright © 2019 Nicolas Fricker. All rights reserved.
//

#include "NeuronalNetwork.h"

__attribute__((visibility("default"))) NeuronalNetwork::NeuronalNetwork()
{
	// initialize threadpool
	sp_threadpool_ = new ThreadPool<NeuronThread, NeuronArg, void*>();
}

__attribute__((visibility("default"))) NeuronalNetwork::NeuronalNetwork(std::vector<int> layers)
{
	/*
		Constructor initializes neurons and estabilshed neighbors and postsynaptic shcematics of the network
	*/
	layers_sizes_ = std::move(layers);
	int sum_neurons = std::accumulate(layers_sizes_.begin(), layers_sizes_.end(), 0);
	
	AllocateNeurons(sum_neurons);
	
	// initialize threadpool
	sp_threadpool_ = new ThreadPool<NeuronThread, NeuronArg, void*>(sum_neurons);
}

__attribute__((visibility("default"))) NeuronalNetwork::NeuronalNetwork(std::initializer_list<int> layers)
{
	/*
	 Constructor initializes neurons and estabilshed neighbors and postsynaptic shcematics of the network
	 */
	layers_sizes_ = std::move(layers);
	int sum_neurons = std::accumulate(layers_sizes_.begin(), layers_sizes_.end(), 0);
	
	AllocateNeurons(sum_neurons);
	
	// initialize threadpool
	sp_threadpool_ = new ThreadPool<NeuronThread, NeuronArg, void*>(sum_neurons);
}

__attribute__((visibility("default"))) NeuronalNetwork::~NeuronalNetwork()
{
	/*
		Deconstructor
	*/
	if (sp_threadpool_) {
		delete sp_threadpool_;
	}
}

__attribute__((visibility("default"))) void NeuronalNetwork::InitializeNetwork()
{
	/*
		Virtual function that establishes network:
		adds references to neighboring neurons
		adds references to postsynaptic neurons
	 */
	int count = 0;
	
	for (int i = 0; i < layers_sizes_.size(); i++) {
		for (int j = count; j < layers_sizes_[i]; j++) {
			for (int m = count; m < layers_sizes_[i]; m++) {
				if (j != m) {
					// adds all neighboring neurons in the layer
					neurons_[j].AddNeighbor(&neurons_[m]);
				}
			}
			count++;
		}
	}
	
	count = 0;
	
	for (int i = 0; i < layers_sizes_.size() - 1; i++) {
		count += layers_sizes_[i];
		// branchless initial index of layer
		int branchless = ((i == 0) ? 0 : (i == layers_sizes_.size() - 1) ? (int)neurons_.size() - layers_sizes_.back() : layers_sizes_[i - 1]);
		for (int j = branchless; j < layers_sizes_[i] + branchless; j++) {
			// assigns postsynaptic neuron using a modulo
			neurons_[j].AddPostsynapticNeuron(&neurons_[count + j % layers_sizes_[i + 1]]);
		}
	}
}

__attribute__((visibility("default"))) const void NeuronalNetwork::Start() noexcept
{
	/*
		Performs Voltage clamp on layer 1
		resues the threadpool to compute each layer sequentially
	*/
	InitializeNetwork();
	
	// start and stop variables to store time stamps
	std::chrono::time_point<std::chrono::system_clock> start, end;
	
	int count = 0;
	// sum of neurons in all the layers
	const int sum_neurons = std::accumulate(layers_sizes_.begin(), layers_sizes_.end(), 0);
	// iterates over the number of bins
	for (int t = 0; t < NeuronalNetwork::s_num_bins_; t++) {
		// start time stamp for each bin
		start = std::chrono::system_clock::now();
		// iterates over the number of layers
		for (int i = 0; i < layers_sizes_.size(); i++) {
			count += layers_sizes_[i];
			// branchless calculation of initial index of each layer i the array of total neurons in the system
			int branchless = ((i == 0) ? 0 : (i == layers_sizes_.size() - 1) ? sum_neurons - layers_sizes_.back() : layers_sizes_[i - 1]);
			// iterate over an individual layer
			for (int j = branchless; j < layers_sizes_[i] + branchless; j++) {
				// if iterating over the first layer
				if (i == 0) { //  && (t < 5000 || t > 15000)
					// voltage clamp neuron in first layer
					neurons_[j].InjectCurrent(s_Iclamp_);
				}
				// add tasks to threadpool
				sp_threadpool_->set_task<Neuron*, double>(&neurons_[j], s_dt_);
			}
			// start thread pool
			sp_threadpool_->start();
			// wait until threads have joined
			sp_threadpool_->join();
		}
		
		// every 10 bins
		if (t % 10 == 0) {
			// calculate end time
			end = std::chrono::system_clock::now();
			
			// calculate time difference
			std::chrono::duration<double> elapsed_seconds = end - start;
			// print time step percentage and time difference
			std::cout << "time step: " << (((double)t)/((double)NeuronalNetwork::s_num_bins_)) * 100 << "%, elapsed time: " << elapsed_seconds.count() << "s\n";
		}
	}
}

__attribute__((visibility("default"))) const void NeuronalNetwork::Stop() noexcept
{
	/*
		asks theadpool to stop execution and clears the queue
	*/
	sp_threadpool_->stop();
	sp_threadpool_->clear();
}

__attribute__((visibility("default"))) const void NeuronalNetwork::Cancel() noexcept
{
	/*
		forces theadpool to cancel execution
		does not clear allocated objects
	*/
	sp_threadpool_->cancel();
	Stop();
}

__attribute__((visibility("default"))) const void NeuronalNetwork::AllocateNeurons(size_t n) noexcept
{
	/*
		Stores n neurons into the neurons_ vector;
	*/
	for (int i = 0; i < n; i++) {
		// initialize Neuron with random output current and neighboring current
		neurons_.emplace_back(i, NeuronalNetwork::s_num_bins_, NeuronalNetwork::s_max_neighbors_);
	}
}

__attribute__((visibility("default"))) std::vector<Neuron>& NeuronalNetwork::GetNeurons() noexcept
{
	/*
		return neuron vector reference
	*/
	return neurons_;
}

__attribute__((visibility("default"))) const bool NeuronalNetwork::Stopped() noexcept
{
	/*
		checks if threadpool has stopped
	*/
	return sp_threadpool_->stopped();
}

__attribute__((visibility("default"))) const void NeuronalNetwork::SetCurrentClamp(const double cc) noexcept
{
	/*
		sets static voltage_clamp
	*/
	NeuronalNetwork::s_Iclamp_ = cc;
}

__attribute__((visibility("default"))) const void NeuronalNetwork::SetTimeStep(const double dt) noexcept
{
	/*
		sets static delta t, bin size
	*/
	NeuronalNetwork::s_dt_ = dt;
}

__attribute__((visibility("default"))) const void NeuronalNetwork::SetNumBins(const int nb) noexcept
{
	/*
		sets static number of bins, iterations
	*/
	NeuronalNetwork::s_num_bins_ = nb;
}

__attribute__((visibility("default"))) const void NeuronalNetwork::SetMaxNeighbors(const int mn) noexcept
{
	/*
		sets static estimated max number of neighbors
		used to reserve memory in vectors, to reduce computation time of growing a vector dynamically
	*/
	NeuronalNetwork::s_max_neighbors_ = mn;
}

pthread_mutex_t* NeuronalNetwork::ResultMutex() noexcept
{
	/*
		returns result mutex
	*/
	return &s_result_m_;
}

pthread_mutex_t* NeuronalNetwork::QueueMutex() noexcept
{
	/*
		returns queue mutex
	*/
	return &s_queue_m_;
}

NeuronalNetwork::NeuronArg::NeuronArg() {}

NeuronalNetwork::NeuronArg::NeuronArg(Neuron* neuron, double dt)
{
	neuron_ = neuron;
	dt_ = dt;
}

// move constructor
NeuronalNetwork::NeuronArg::NeuronArg(NeuronArg&& other): neuron_(std::move(other.neuron_)), dt_(std::move(other.dt_)) {}

NeuronalNetwork::NeuronArg::~NeuronArg() {}

NeuronalNetwork::NeuronThread::NeuronThread(std::vector<NeuronArg>* queue, void* results, std::atomic<int>* count)
{
	queue_ = queue;
	results_ = results;
	a_count_ = count;
}

// move construtor
NeuronalNetwork::NeuronThread::NeuronThread(NeuronThread&& other): queue_(std::move(other.queue_)), results_(std::move(other.results_)), a_count_(std::move(other.a_count_)) {}

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
		pthread_mutex_lock(&s_queue_m_);
		// breaks while loop if queue is empty or stopped flag is triggered
		if (queue_->empty() || stopped()) {
			// unlock queue mutex
			pthread_mutex_unlock(&s_queue_m_);
			break;
		}
		// moves neuron argument from back of queue to local variable
		arg = std::move(queue_->back());
		// pop last element in the queue
		queue_->pop_back();
		// unlock queue mutex
		pthread_mutex_unlock(&s_queue_m_);
		
		if (arg.neuron_) {
			// update membrane potential
			arg.neuron_->Process(arg.dt_);
		}
		// increments count
		(*a_count_)++;
	}
	
	return NULL;
}






