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

#include <atomic>
#include <vector>

typedef unsigned int neuron_id_t;

class Neuron
{
	// array of neighboring neurons pointer
	std::vector<Neuron*> neighbors_;

	// Vm log
	std::vector<double> history_;
	
	// pointer to postsynaptic neuron
	Neuron* postsynaptic_ = nullptr;

	// atomic input current sum [µA]
	std::atomic<double> a_Isum_ = 0;

	// membrane potential [mV]
	double Vm_ = -64.9964;

	// [uF/cm^2] membrane capacitance density
	double Cm_ = 0.01;

	// potassium channel activation conductance
	double n_ = 0.3177;
	// sodium channel activation conductance
	double m_ = 0.0530;
	// leak ions channel deactivation conductance
	double h_ = 0.5960;

	// output current [µA]
	double oc_ = 0.01;

	// neighbor current increase [µA]
	double nc_ = 0.01;
	
	neuron_id_t id_;
	
	// boolean indicate of cell state
	bool spiked_ = false;

	// static membrane resting potential
	inline constexpr static const double s_V_rest_ = -64.9964;
	// static membrane threashold potential
	inline constexpr static const double s_V_threashold_ = -55.0;
	
	// ion channel current constants
	inline constexpr static const double s_GNa_ = 1.20;
	inline constexpr static const double s_GK_ = 0.36;
	inline constexpr static const double s_GL_ = 0.003;
	// ion channel potential constants
	inline constexpr static const double s_ENa_ = 50.0;
	inline constexpr static const double s_EK_ = -77.0;
	inline constexpr static const double s_EL_ = -54.4;

public:
	Neuron(neuron_id_t neuron_id, const int num_bins = 10000, const int max_neighbors = 10000);
	Neuron(neuron_id_t neuron_id, const double oc, const double nc, const int num_bins = 10000, const int max_neighbors = 10000);
	Neuron(neuron_id_t neuron_id, const double Vm, const double Cm, const double n, const double m, const double h, const int num_bins = 10000, const int max_neighbors = 10000);
	Neuron(Neuron&& other);
	~Neuron();
	
	Neuron& operator=(const Neuron& other);

	const double Process(const double dt) noexcept;
	const void InjectCurrent(const double input) noexcept;

	const void AddPostsynapticNeuron(Neuron* next) noexcept;
	const void AddNeighbor(Neuron* neighbor) noexcept;
	const void SetMembraneCapacitance(const double Cm) noexcept;
	
	const neuron_id_t GetNeuronId() noexcept;

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
