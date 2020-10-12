//
//  constants.h
//  NeuronalNetwork
//
//  Created by Nicolas Fricker on 04/10/20.
//  Copyright © 2019 Nicolas Fricker. All rights reserved.
//

#ifndef constants_
#define constants_

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

	constexpr const int max_neighbors = 10000;
}

#pragma GCC visibility pop

#endif /* constants_h */
