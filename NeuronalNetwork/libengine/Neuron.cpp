//
//  Neuron.cpp
//  NeuronalNetwork
//
//  Created by Nicolas Fricker on 04/10/20.
//  Copyright © 2019 Nicolas Fricker. All rights reserved.
//

#include "Neuron.h"

#include <cstdlib>
#include <cmath>


Neuron::Neuron(neuron_id_t neuron_id, const int num_bins, const int max_neighbors)
{
	id_ = neuron_id;
	neighbors_.reserve(max_neighbors);
	history_.reserve(num_bins);
	oc_ = Rand(0.01, 0.05);
	nc_ = Rand(0.001, 0.005);
}

Neuron::Neuron(neuron_id_t neuron_id, const double oc, const double nc, const int num_bins, const int max_neighbors)
{
	id_ = neuron_id;
	neighbors_.reserve(max_neighbors);
	history_.reserve(num_bins);
	oc_ = oc;
	nc_ = nc;
}

Neuron::Neuron(neuron_id_t neuron_id, const double Vm, const double Cm, const double n, const double m, const double h, const int num_bins, const int max_neighbors)
{
	id_ = neuron_id;
	neighbors_.reserve(max_neighbors);
	history_.reserve(num_bins);
	Vm_ = Vm;
	Cm_ = Cm;
	n_ = n;
	m_ = m;
	h_ = h;
	oc_ = Rand(0.01, 0.05);
	nc_ = Rand(0.001, 0.005);
}

Neuron::Neuron(Neuron&& other)
{
	/*
		move constructor
	*/
	a_Isum_ = other.a_Isum_.load();
	
	postsynaptic_ = std::move(other.postsynaptic_);
	neighbors_ = std::move(other.neighbors_);
	history_ = std::move(other.history_);
	
	n_ = std::move(other.n_);
	m_ = std::move(other.m_);
	h_ = std::move(other.h_);
	
	oc_ = std::move(other.oc_);
	nc_ = std::move(other.nc_);
	
	id_ = std::move(other.id_);
	
	spiked_ = std::move(other.spiked_);
}

Neuron::~Neuron() {}

Neuron& Neuron::operator=(const Neuron& other)
{
	/*
		equal operator copy assignment
	*/
	if (this != &other) {
		a_Isum_ = other.a_Isum_.load();
		
		postsynaptic_ = other.postsynaptic_;
		
		neighbors_.clear();
		for (int i = 0; i < other.neighbors_.size(); i++) {
			neighbors_.emplace_back(other.neighbors_[i]);
		}
		
		history_.clear();
		
		for (int i = 0; i < other.history_.size(); i++) {
			history_.emplace_back(other.history_[i]);
		}
		
		n_ = other.n_;
		m_ = other.m_;
		h_ = other.h_;
		
		oc_ = other.oc_;
		nc_ = other.nc_;
		
		spiked_ = other.spiked_;
	}
	
	return *this;
}

const double Neuron::Process(const double dt) noexcept
{
	/*
		dt = delta time
		sum of current retrived atomically
		return HodgkinHuxley Model updated membrane potential
	*/
	const double Ic = a_Isum_.load();
	a_Isum_ = 0;
	return HodgkinHuxley(dt, Ic);
}

const void Neuron::InjectCurrent(const double input) noexcept
{
	/*
		input = input current in µA
		sums up currents atomically
	*/
	a_Isum_.store(a_Isum_.load() + input);
}

__attribute__((visibility("default"))) const void Neuron::AddPostsynapticNeuron(Neuron* postsynaptic) noexcept
{
	/*
		postsynaptic Neuron pointer assignment
	*/
	postsynaptic_ = postsynaptic;
}

__attribute__((visibility("default"))) const void Neuron::AddNeighbor(Neuron* neighbor) noexcept
{
	/*
		stores neighboring Neuron pointer
	*/
	neighbors_.emplace_back(neighbor);
}

__attribute__((visibility("default"))) const void Neuron::SetMembraneCapacitance(const double Cm) noexcept
{
	/*
		Cm = Membrane Capacitance Density assignment
	*/
	Cm_ = Cm;
}

__attribute__((visibility("default"))) const neuron_id_t Neuron::GetNeuronId() noexcept
{
	/*
		rerturn neuron id;
	*/
	return id_;
}

__attribute__((visibility("default"))) std::vector<double>& Neuron::GetHistory() noexcept
{
	/*
		returns membrane potential history log
	*/
	return history_;
}

__attribute__((visibility("default"))) const size_t Neuron::GetHistorySize() noexcept
{
	/*
		returns membrane potential history log's size
	*/
	return history_.size();
}

__attribute__((visibility("default"))) const bool Neuron::IsInhibitory() noexcept
{
	/*
		return true if the output current is inhibitor, negative
	*/
	return oc_ < 0;
}

__attribute__((visibility("default"))) const bool Neuron::IsExhitatory() noexcept
{
	/*
		return true if the output current is exhitatory, positive
	*/
	return oc_ > 0;
}

const double Neuron::AM(const double Vm) noexcept
{
	/*
		αm function
	*/
	return 0.1 * (Vm + 40.0) / (1.0 - exp(- (Vm + 40.0) / 10.0));
}

const double Neuron::BM(const double Vm) noexcept
{
	/*
		βm function
	*/
	return 4.0 * exp(- (Vm + 64.0) / 18.0);
}

const double Neuron::AH(const double Vm) noexcept
{
	/*
		αh function
	*/
	return 0.07 * exp(- (Vm + 65) / 20);
}

const double Neuron::BH(const double Vm) noexcept
{
	/*
		βh function
	*/
	return 1.0 / (1.0 + exp(- (Vm + 35.0) / 10.0));
}

const double Neuron::AN(const double Vm) noexcept
{
	/*
		αn function
	*/
	return 0.01 * (Vm + 55) / (1 - exp(- (Vm + 55.0) / 10.0));
}

const double Neuron::BN(const double Vm) noexcept
{
	/*
		βn function
	*/
	return 0.125 * exp(- (Vm + 65.0) / 80.0);
}

const void Neuron::Step(double& x, const double Vm, const double dt, const double (*a)(const double), const double (*b)(const double)) noexcept
{
	// α, β functions
	const double aX = (*a)(Vm);
	const double bX = (*b)(Vm);

	// τ
	const double tau = 1 / (aX + bX);
	// channel activation value at ∞
	const double inf = aX * tau;

	// update channel activation value
	x = inf + (x - inf) * exp(- dt / tau);
}

const double Neuron::HodgkinHuxley(const double dt, const double current_stimulus) noexcept
{
	/*
		Hodgkin-Huxley Model

		Intergrate ODE
		∫-C * dV(t)/dt * dt = ∫(-iK -iL -iNa) + i
		-C * V(t) = i * t - x * t - y * t - z * z + C1
		V(t) = ((x + y + z - i) * t - c1) / C
	 
		return updated membrane potential
	*/

	// Currents: Na, K, leak
	const double iNa = s_GNa_ * pow(m_, 3) * h_;
	const double iK = s_GK_ * pow(n_, 4);
	const double iL = s_GL_;

	// Sum of ion currents
	const double iTotal = iNa + iK + iL;
	
	// membrane potential as it tends to ∞
	const double V_inf = ((s_ENa_ * iNa + s_EK_ * iK + s_EL_ * iL) + current_stimulus) / iTotal;
	
	// update membrane potential τ
	const double tau_v = Cm_ / iTotal;
	
	// update membrane potential
	Vm_ = V_inf + (Vm_ - V_inf) * exp(- dt / tau_v);

	// stores membrane potential in history log
	history_.emplace_back(Vm_);
	
	// branchless cell state update
	spiked_ = (Vm_ >= s_V_threashold_) ? true : false;
	
	// propagates output current to postsynaptic Neuron and neighboring current to neighboring Neurons in time step t
	//for processing in time step t + 1
	if (postsynaptic_) {
		// increment postsynaptic neuron's current by transmitted output current
		// branchless if spiked == true
		postsynaptic_->InjectCurrent(oc_ * spiked_);
	}

	for (int i = 0; i < neighbors_.size(); i++) {
		if (neighbors_[i]) {
			// increment neighboring neurons' current exponentially
			// branchless if spike == true
			neighbors_[i]->InjectCurrent(NeighborCurrent() * spiked_);
		}
	}
	
	// update sodium channel activation membrane
	Step(m_, Vm_, dt, &Neuron::AM, &Neuron::BM);
	// update leak ion channels activation membrane
	Step(h_, Vm_, dt, &Neuron::AH, &Neuron::BH);
	// update potassium channel activation membrane
	Step(n_, Vm_, dt, &Neuron::AN, &Neuron::BN);

	return Vm_;
}

const double Neuron::NeighborCurrent() noexcept
{
	/*
		Calculates ∆I effect of this spiking neuron to its neighbors
		using an exponential function
	*/
	return nc_ * exp(- Vm_ / s_V_rest_);
}


