//
//  Neuron.cpp
//  NeuronalNetwork
//
//  Created by Nicolas Fricker on 04/10/20.
//  Copyright © 2019 Nicolas Fricker. All rights reserved.
//


#include "Neuron.h"

Neuron::Neuron() 
{
	 neighbors = new Neuron*[constants::max_neighbors];
	 history = new double[constants::num_samples];
}

Neuron::Neuron(const double oc, const double nc)
{
	 neighbors = new Neuron*[constants::max_neighbors];
	 history = new double[constants::num_samples];
	this->oc = oc;
	this->nc = nc;
}

Neuron::Neuron(const double Vm, const double n, const double m, const double h)
{
	neighbors = new Neuron*[constants::max_neighbors];
	history = new double[constants::num_samples];
	this->Vm = Vm;
	this->n = n; 
	this->m = m;
	this->h = h;
}

Neuron::~Neuron() 
{
	if (neighbors) {
		delete[] neighbors;
	}
	if (history) {
		delete[] history;
	}
}

const double Neuron::Process(const double dt) noexcept
{
	double Ic = Isum.load();
	Isum.store(0);
	return HodgkinHuxley(dt, Ic);
}

const void Neuron::InjectCurrent(const double input) noexcept
{
	Isum.store(Isum.load() + input);
}

const void Neuron::AddPostsynapticNeuron(Neuron* postsynaptic) noexcept
{
	this->postsynaptic = postsynaptic;
}

const void Neuron::AddNeighbor(Neuron* neighbor) noexcept
{
	if (neighbors) {
		neighbors[num_neighbors] = neighbor;
		num_neighbors++;
	}
}

double* Neuron::GetHistory() noexcept
{
	return history;
}

const int Neuron::GetHistorySize() noexcept
{
	return num_history;
}

const bool Neuron::IsInhibitory() noexcept
{
	return oc < 0;
}

const bool Neuron::IsExhitatory() noexcept
{
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
	*/

	// Currents: Na, K, leak
	const double iNa = GNa * pow(m, 3) * h;
	const double iK = GK * pow(n, 4);
	const double iL = GL;

	const double iTotal = iNa + iK + iL;

	const double V_inf = ((ENa * iNa + EK * iK + EL * iL) + current_stimulus) / iTotal;

	const double tau_v = constants::membrane_capacitance_density / iTotal;

	Vm = V_inf + (Vm - V_inf) * exp(- dt / tau_v);

	 history[num_history] = Vm;
	 num_history++;

	spiked = (Vm >= V_threashold) ? true : false;

	if (postsynaptic) {
		postsynaptic->InjectCurrent(oc * spiked);
	}

	if (neighbors) {
	 	for (int i = 0; i < num_neighbors; ++i) {
	 		if (neighbors[i]) {
	 			neighbors[i]->InjectCurrent(NeighborCurrent() * spiked);
	 		}
	 	}
	 }
	
	Step(m, Vm, dt, &Neuron::AM, &Neuron::BM);
	Step(h, Vm, dt, &Neuron::AH, &Neuron::BH);
	Step(n, Vm, dt, &Neuron::AN, &Neuron::BN);

	return Vm;
}

const double Neuron::NeighborCurrent() noexcept
{
	/*
		Caclculates ∆I effect of this spiking neuron to its neighbors, for t = 0.01 ms
	*/
	return nc * exp(- Vm / constants::V_rest);
}


