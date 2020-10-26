//
//  main.cpp
//  NeuronalNetwork
//
//  Created by Nicolas Fricker on 10/11/20.
//

#include <iostream>
#include <chrono>

#include "PythonWrapper.h"

int main(int argc, const char * argv[]) {
	
//	(void)run();
//	Get dylib symbols
	std::system("nm -g /Users/fricker/Library/Developer/Xcode/DerivedData/NeuronalNetwork-ewjdcsoexwcnzucgtoqsskdeydmw/Build/Products/Debug/libengine.dylib > libengine_symbols.txt");
	
//	Get assembly code
	std::system("objdump -disassemble -x86-asm-syntax=intel /Users/fricker/Library/Developer/Xcode/DerivedData/NeuronalNetwork-ewjdcsoexwcnzucgtoqsskdeydmw/Build/Products/Debug/libengine.dylib > libengine_symbols.txt > libengine.asm");
	
//	Run python graphing script
	std::system("python3 /Users/fricker/Documents/GitHub/NeuronalNetworkModel/NeuronalNetwork/NeuronalNetwork/test.py");
	return 0;
}
