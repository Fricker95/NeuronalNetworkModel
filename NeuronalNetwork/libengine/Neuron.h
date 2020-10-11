//
//  Neuron.h
//  NeuronalNetwork
//
//  Created by Nicolas Fricker on 04/10/20.
//  Copyright © 2019 Nicolas Fricker. All rights reserved.
//

#include <iostream>
#include <cstdlib>
#include <atomic>
#include <cmath>
#include <vector>

#ifndef Neuron_
#define Neuron_

#pragma GCC visibility push(hidden)

namespace constants
{
	constexpr const double time_step = 0.01;
	constexpr const double num_samples = 10000;

	constexpr const double V_initial = -64.9964;
	constexpr const double V_rest = -64.9964;
	constexpr const double V_threashold = -55.0;

	constexpr const double membrane_surface = 4000.0;  // [um^2] surface area of the membrane
	constexpr const double membrane_capacitance_density = 0.01;   // [uF/cm^2] membrane capacitance density
	constexpr const double membrane_capacitance = membrane_capacitance_density * membrane_surface * 1e-8;   // [uF] membrane capacitance

	constexpr const double GNa = 1.20;
	constexpr const double GK = 0.36;
	constexpr const double GL = 0.003;
	constexpr const double ENa = 50.0;
	constexpr const double EK = -77.0;
	constexpr const double EL = -54.4;

	constexpr const double sodium_conductance = GNa * membrane_surface * 1e-8;// Na conductance [mS]
	constexpr const double potassium_conductance = GK * membrane_surface * 1e-8;  // K conductance [mS]
	constexpr const double leak_conductance = GL * membrane_surface * 1e-8;  // leak conductance [mS]

	constexpr const int max_neighbors = 100;
	constexpr const int max_num_syn = 100;
}

class Neuron
{
	Neuron* postsynaptic = nullptr;
	// Neuron** neighbors = nullptr;
	std::vector<Neuron*> neighbors;

	// Vm log
	double* history = nullptr;

	// atomic input current sum [µA]
	std::atomic<double> Isum{0};

	// membrane potential [mV]
	double Vm = -64.9964;

	// [uF/cm^2] membrane capacitance density
	// double Cm = 0.01;

	// potassium channel activation conductance
	double n = 0.3177; 
	// sodium channel activation conductance
	double m = 0.0530;
	// leak ions channel deactivation conductance
	double h = 0.5960;

	// output current [µA]
	double oc = 0.01;

	// neighbor current increase [µA]
	double nc = 0.01;

	// neighbors neurons size
	unsigned int num_neighbors = 0;
	
	// Vm log size
	unsigned int num_history = 0;

	bool spiked = false;

	inline constexpr static const double V_rest = -64.9964;
	inline constexpr static const double V_threashold = -55.0;

	inline constexpr static const double GNa = 1.20;
	inline constexpr static const double GK = 0.36;
	inline constexpr static const double GL = 0.003;
	inline constexpr static const double ENa = 50.0;
	inline constexpr static const double EK = -77.0;
	inline constexpr static const double EL = -54.4;

	constexpr static const unsigned int max_neighbors = 100;

public:
	Neuron();
	Neuron(const double oc, const double nc);
	Neuron(const double Vm, const double n, const double m, const double h);
	Neuron(const Neuron& other);
	Neuron(Neuron&& other);
	~Neuron();

	const double Process(const double dt) noexcept;
	const void InjectCurrent(const double input) noexcept;

	const void AddPostsynapticNeuron(Neuron* next) noexcept;
	const void AddNeighbor(Neuron* neighbor) noexcept;

	double* GetHistory() noexcept;
	const int GetHistorySize() noexcept;

	const bool IsInhibitory() noexcept;
	const bool IsExhitatory() noexcept;

private:
	static const double AM(const double Vm) noexcept;
	static const double BM(const double Vm) noexcept;

	static const double AH(const double Vm) noexcept;
	static const double BH(const double Vm) noexcept;

	static const double AN(const double Vm) noexcept;
	static const double BN(const double Vm) noexcept;

	const void Step(double& x, const double Vm, const double dt, const double (*a)(const double), const double (*b)(const double)) noexcept;

	const double HodgkinHuxley(const double dt, const double current_stimulus) noexcept;

	const double NeighborCurrent() noexcept;

};
#pragma GCC visibility pop
#endif /* Neuron_ */


