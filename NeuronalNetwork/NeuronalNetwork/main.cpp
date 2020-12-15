//
//  main.cpp
//  NeuronalNetwork
//
//  Created by Nicolas Fricker on 10/11/20.
//

#include <iostream>
#include <chrono>
#include <vector>

#include <../libengine/PythonWrapper.h>

void pythonExecFunc();

int main(int argc, const char * argv[]) {
	
	std::cout << sizeof(Neuron) << "\n";
	std::cout << sizeof(NeuronalNetwork) << "\n";
	
//	pythonExecFunc();

	return 0;
}

void pythonExecFunc()
{
//	//	Get dylib symbols
//	std::system("nm -g /Users/fricker/Library/Developer/Xcode/DerivedData/NeuronalNetwork-ewjdcsoexwcnzucgtoqsskdeydmw/Build/Products/Debug/libengine.dylib > libengine_symbols.txt");
//
//	//	Get assembly code
//	std::system("objdump -disassemble -x86-asm-syntax=intel /Users/fricker/Library/Developer/Xcode/DerivedData/NeuronalNetwork-ewjdcsoexwcnzucgtoqsskdeydmw/Build/Products/Debug/libengine.dylib > libengine_symbols.txt > libengine.asm");
	
//	//	Run python graphing script
	std::system("python3 /Users/fricker/Documents/GitHub/NeuronalNetworkModel/NeuronalNetwork/NeuronalNetwork/test.py");
}


/*
	Things to think about further implementation
 
 // [um^2] surface area of the membrane
 //  inline constexpr static const double membrane_surface = 4000.0;
 // [uF/cm^2] membrane capacitance density
 //  inline constexpr static const double membrane_capacitance_density = 0.01;
 // [uF] membrane capacitance
 //  inline constexpr static const double membrane_capacitance = membrane_capacitance_density * membrane_surface * 1e-8;
 
 // Na conductance [mS]
 inline constexpr static const double sodium_conductance = GNa * membrane_surface * 1e-8;
 // K conductance [mS]
 inline constexpr static const double potassium_conductance = GK * membrane_surface * 1e-8;
 // leak conductance [mS]
 inline constexpr static const double leak_conductance = GL * membrane_surface * 1e-8;
 
*/
