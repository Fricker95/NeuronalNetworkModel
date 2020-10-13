#include "PythonWrapper.h"

const double* run(const double x, const double dt, const int size)
{
	std::chrono::time_point<std::chrono::system_clock> start, end;
	
	start = std::chrono::system_clock::now();
	
	NeuronalNetwork n;
	
	std::vector<double> result = n.Start();
	
	end = std::chrono::system_clock::now();
	
	std::chrono::duration<double> elapsed_seconds = end - start;
	std::time_t end_time = std::chrono::system_clock::to_time_t(end);
	
	std::cout << "finished computation at " << std::ctime(&end_time)
	<< "elapsed time: " << elapsed_seconds.count() << "s\n";
	
	printf("Done");

	double* voltages = new double[result.size()];
	
	for (int i = 0; i < result.size(); i++) {
		voltages[i] = result[i];
	}

	return voltages;
}
