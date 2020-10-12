//
//  main.cpp
//  NeuronalNetwork
//
//  Created by Nicolas Fricker on 10/11/20.
//

#include <iostream>
#include <chrono>

#include "NeuronalNetwork.h"

int main(int argc, const char * argv[]) {
	std::chrono::time_point<std::chrono::system_clock> start, end;
	
	start = std::chrono::system_clock::now();
	
	NeuronalNetwork n;

	n.Start();
	
	end = std::chrono::system_clock::now();
	
	std::chrono::duration<double> elapsed_seconds = end - start;
	std::time_t end_time = std::chrono::system_clock::to_time_t(end);
	
	std::cout << "finished computation at " << std::ctime(&end_time)
	<< "elapsed time: " << elapsed_seconds.count() << "s\n";
	
	printf("Done");
	
	return 0;
}
