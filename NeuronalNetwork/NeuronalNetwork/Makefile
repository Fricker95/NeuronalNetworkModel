CFLAGS = -std=c++17 -stdlib=libc++ -O3

NeuronalNetwork: libengine.so
	clang++ ${CFLAGS} -o NeuronalNetwork main.cpp -I. -I./libengine/ -L. -lengine -lpthread
	rm -rf *.o
# 	python3 ../NeuronalNetwork/test.py

Neuron.o: ../libengine/Neuron.h ../libengine/Neuron.cpp
	clang++ ${CFLAGS} -c ../libengine/Neuron.cpp

NeuronalNetwork.o: ../libengine/NeuronalNetwork.h ../libengine/NeuronalNetwork.cpp
	clang++ ${CFLAGS} -c ../libengine/NeuronalNetwork.cpp

PythonWrapper.o: ../libengine/PythonWrapper.h ../libengine/PythonWrapper.cpp
	clang++ ${CFLAGS} -c ../libengine/PythonWrapper.cpp

libengine.so: Neuron.o NeuronalNetwork.o PythonWrapper.o
	clang++ -shared -o libengine.so *.o -I.

clean:
	rm -rf NeuronalNetwork libengine.so *.o