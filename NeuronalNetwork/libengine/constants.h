//
//  constants.h
//  NeuronalNetwork
//
//  Created by Nicolas Fricker on 04/10/20.
//  Copyright © 2019 Nicolas Fricker. All rights reserved.
//

#ifndef constants_
#define constants_

#pragma GCC visibility push(default)

namespace constants
{
	// number of bins, num_bins * dt = µs
	constexpr const double num_bins = 10000;
	// number of neighboring neurons
	constexpr const int num_neighbors = 10000;
}

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

#endif /* constants_h */
