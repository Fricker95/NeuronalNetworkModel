//
//  Neuron.h
//  NeuronalNetwork
//
//  Created by Nicolas Fricker on 04/10/20.
//  Copyright © 2019 Nicolas Fricker. All rights reserved.
//

#ifndef Neuron_
#define Neuron_

#pragma GCC visibility push(hidden)

#include <iostream>
#include <cstdlib>
#include <atomic>
#include <cmath>
#include <vector>

class Neuron
{
	// array of neighboring neurons pointer
	std::vector<Neuron*> neighbors;

	// Vm log
	std::vector<double> history;
	
	// pointer to postsynaptic neuron
	Neuron* postsynaptic = nullptr;

	// atomic input current sum [µA]
	std::atomic<double> Isum = 0;

	// membrane potential [mV]
	double Vm = -64.9964;

	// [uF/cm^2] membrane capacitance density
	double Cm = 0.01;

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
	
	// boolean indicate of cell state
	bool spiked = false;

	// static membrane resting potential
	inline constexpr static const double V_rest = -64.9964;
	// static membrane threashold potential
	inline constexpr static const double V_threashold = -55.0;
	
	// ion channel current constants
	inline constexpr static const double GNa = 1.20;
	inline constexpr static const double GK = 0.36;
	inline constexpr static const double GL = 0.003;
	// ion channel potential constants
	inline constexpr static const double ENa = 50.0;
	inline constexpr static const double EK = -77.0;
	inline constexpr static const double EL = -54.4;

public:
	Neuron(const int num_bins = 10000, const int max_neighbors = 10000);
	Neuron(const double oc, const double nc, const int num_bins = 10000, const int max_neighbors = 10000);
	Neuron(const double Vm, const double Cm, const double n, const double m, const double h, const int num_bins = 10000, const int max_neighbors = 10000);
	Neuron(Neuron&& other);
	~Neuron();
	
	Neuron& operator=(const Neuron& other);

	const double Process(const double dt) noexcept;
	const void InjectCurrent(const double input) noexcept;

	const void AddPostsynapticNeuron(Neuron* next) noexcept;
	const void AddNeighbor(Neuron* neighbor) noexcept;
	const void SetMembraneCapacitance(const double Cm) noexcept;

	std::vector<double>& GetHistory() noexcept;
	const size_t GetHistorySize() noexcept;

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



// [um^2] surface area of the membrane
//  inline constexpr static const double membrane_surface = 4000.0;
// [uF/cm^2] membrane capacitance density
//  inline constexpr static const double membrane_capacitance_density = 0.01;
// [uF] membrane capacitance
//  inline constexpr static const double membrane_capacitance = membrane_capacitance_density * membrane_surface * 1e-8;

/*
 // Na conductance [mS]
 inline constexpr static const double sodium_conductance = GNa * membrane_surface * 1e-8;
 // K conductance [mS]
 inline constexpr static const double potassium_conductance = GK * membrane_surface * 1e-8;
 // leak conductance [mS]
 inline constexpr static const double leak_conductance = GL * membrane_surface * 1e-8;
 */
