//
//  Neuron.cpp
//  NeuronalNetwork
//
//  Created by Nicolas Fricker on 04/10/20.
//  Copyright © 2019 Nicolas Fricker. All rights reserved.
//


#include "Neuron.h"

Neuron::Neuron(const int num_bins, const int max_neighbors)
{
	neighbors.reserve(max_neighbors);
	history.reserve(num_bins);
}

Neuron::Neuron(const double oc, const double nc, const int num_bins, const int max_neighbors)
{
	neighbors.reserve(max_neighbors);
	history.reserve(num_bins);
	this->oc = oc;
	this->nc = nc;
}

Neuron::Neuron(const double Vm, const double Cm, const double n, const double m, const double h, const int num_bins, const int max_neighbors)
{
	neighbors.reserve(max_neighbors);
	history.reserve(num_bins);
	this->Vm = Vm;
	this->Cm = Cm;
	this->n = n; 
	this->m = m;
	this->h = h;
}

Neuron::Neuron(Neuron&& other)
{
	/*
		move constructor
	*/
	Isum = other.Isum.load();
	
	postsynaptic = std::move(other.postsynaptic);
	neighbors = std::move(other.neighbors);
	history = std::move(other.history);
	
	n = std::move(other.n);
	m = std::move(other.m);
	h = std::move(other.h);
	
	oc = std::move(other.oc);
	nc = std::move(other.nc);
	
	spiked = std::move(other.spiked);
}

Neuron::~Neuron() {}

Neuron& Neuron::operator=(const Neuron& other)
{
	/*
		equal operator copy assignment
	*/
	if (this != &other) {
		Isum = other.Isum.load();
		
		postsynaptic = other.postsynaptic;
		
		neighbors.clear();
		for (int i = 0; i < other.neighbors.size(); i++) {
			neighbors.emplace_back(other.neighbors[i]);
		}
		
		history.clear();
		
		for (int i = 0; i < other.history.size(); i++) {
			history.emplace_back(other.history[i]);
		}
		
		n = other.n;
		m = other.m;
		h = other.h;
		
		oc = other.oc;
		nc = other.nc;
		
		spiked = other.spiked;
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
	const double Ic = Isum.load();
	Isum = 0;
	return HodgkinHuxley(dt, Ic);
}

const void Neuron::InjectCurrent(const double input) noexcept
{
	/*
		input = input current in µA
		sums up currents atomically
	*/
	Isum.store(Isum.load() + input);
}

const void Neuron::AddPostsynapticNeuron(Neuron* postsynaptic) noexcept
{
	/*
		postsynaptic Neuron pointer assignment
	*/
	this->postsynaptic = postsynaptic;
}

const void Neuron::AddNeighbor(Neuron* neighbor) noexcept
{
	/*
		stores neighboring Neuron pointer
	*/
	neighbors.emplace_back(neighbor);
}

const void Neuron::SetMembraneCapacitance(const double Cm) noexcept
{
	/*
		Cm = Membrane Capacitance Density assignment
	*/
	this->Cm = Cm;
}

std::vector<double>& Neuron::GetHistory() noexcept
{
	/*
		returns membrane potential history log
	*/
	return history;
}

const size_t Neuron::GetHistorySize() noexcept
{
	/*
		returns membrane potential history log's size
	*/
	return history.size();
}

const bool Neuron::IsInhibitory() noexcept
{
	/*
		return true if the output current is inhibitor, negative
	*/
	return oc < 0;
}

const bool Neuron::IsExhitatory() noexcept
{
	/*
		return true if the output current is exhitatory, positive
	*/
	return oc > 0;
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
	const double iNa = GNa * pow(m, 3) * h;
	const double iK = GK * pow(n, 4);
	const double iL = GL;

	// Sum of ion currents
	const double iTotal = iNa + iK + iL;
	
	// membrane potential as it tends to ∞
	const double V_inf = ((ENa * iNa + EK * iK + EL * iL) + current_stimulus) / iTotal;
	
	// update membrane potential τ
	const double tau_v = Cm / iTotal;
	
	// update membrane potential
	Vm = V_inf + (Vm - V_inf) * exp(- dt / tau_v);

	// stores membrane potential in history log
	history.emplace_back(Vm);
	
	// branchless cell state update
	spiked = (Vm >= V_threashold) ? true : false;
	
	// propagates output current to postsynaptic Neuron and neighboring current to neighboring Neurons in time step t
	//for processing in time step t + 1
	if (postsynaptic) {
		// increment postsynaptic neuron's current by transmitted output current
		// branchless if spiked == true
		postsynaptic->InjectCurrent(oc * spiked);
	}

	for (int i = 0; i < neighbors.size(); i++) {
		if (neighbors[i]) {
			// increment neighboring neurons' current exponentially
			// branchless if spike == true
			neighbors[i]->InjectCurrent(NeighborCurrent() * spiked);
		}
	}
	
	// update sodium channel activation membrane
	Step(m, Vm, dt, &Neuron::AM, &Neuron::BM);
	// update leak ion channels activation membrane
	Step(h, Vm, dt, &Neuron::AH, &Neuron::BH);
	// update potassium channel activation membrane
	Step(n, Vm, dt, &Neuron::AN, &Neuron::BN);

	return Vm;
}

const double Neuron::NeighborCurrent() noexcept
{
	/*
		Caclculates ∆I effect of this spiking neuron to its neighbors
	*/
	return nc * exp(- Vm / V_rest);
}


