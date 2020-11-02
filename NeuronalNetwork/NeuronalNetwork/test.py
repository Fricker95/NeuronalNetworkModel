import numpy as np
from numpy.ctypeslib import ndpointer
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import subprocess
import ctypes 
import sys

def plot_multiple_V_vs_t(Vs, I, t, dt, arr = [16,4,1]):
	fig, axs = plt.subplots(sum(arr), 1, sharex=True, sharey=True, figsize=(20,20))
	fig.subplots_adjust(hspace=0)
	fig.suptitle('Voltage vs Time for I=%f $\mu$A'%I)
	fig.add_subplot(111, frameon=False)
	
	lines = []
	t = [i * dt for i in range(len(t))]
	for i in range(0, sum(arr)):
		if i < 16:
			if i % 4 == 0:
				line, = axs[i].plot(t, Vs[i*len(t):(i+1)*len(t)], color = "red", label=f"L1 N{i} -> L2 N{i % 4}")
			if i % 4 == 1:
				line, = axs[i].plot(t, Vs[i*len(t):(i+1)*len(t)], color = "green", label=f"L1 N{i} -> L2 N{i % 4}")
			if i % 4 == 2:
				line, = axs[i].plot(t, Vs[i*len(t):(i+1)*len(t)], color = "blue", label=f"L1 N{i} -> L2 N{i % 4}")
			if i % 4 == 3:
				line, = axs[i].plot(t, Vs[i*len(t):(i+1)*len(t)], color = "pink", label=f"L1 N{i} -> L2 N{i % 4}")
		elif i >= 16 and i < 20:
			if i % 4 == 0:
				line, = axs[i].plot(t, Vs[i*len(t):(i+1)*len(t)], color = "orange", label=f"L2 N{i % 4} -> L3 N{0}")
			if i % 4 == 1:
				line, = axs[i].plot(t, Vs[i*len(t):(i+1)*len(t)], color = "lightgreen", label=f"L2 N{i % 4} -> L3 N{0}")
			if i % 4 == 2:
				line, = axs[i].plot(t, Vs[i*len(t):(i+1)*len(t)], color = "lightblue", label=f"L2 N{i % 4} -> L3 N{0}")
			if i % 4 == 3:
				line, = axs[i].plot(t, Vs[i*len(t):(i+1)*len(t)], color = "purple", label=f"L2 N{i % 4} -> L3 N{0}")
		else:
			line, = axs[i].plot(t, Vs[i*len(t):(i+1)*len(t)], color = "black", label=f"L3 N0")

		axs[i].set_yticks([-65, -55, 0])
		axs[i].set_ylim(-90, 90)

		lines.append(line)
		axs[i].grid(True)
		axs[i].legend(loc='upper right', shadow=True, fancybox=True)
	print("Working")

	plt.tick_params(labelcolor='none', top=False, bottom=False, left=False, right=False)
	plt.xlabel('time in msec')
	plt.ylabel('Voltage in mV')
#	plt.ylim(-90, 90)

	ani = animation.FuncAnimation(fig, animate, len(t)//100, fargs=[Vs, t, lines], interval=200, blit=False)
	ani.save('test.gif')
	# plt.show()
	
def plot_V_vs_t(Vs, I, t, dt):
	plt.plot(t*dt, Vs)
	plt.grid()
	plt.title('Voltage vs Time for I=%f $\mu$A'%I)
	plt.xlabel('time in msec')
	plt.ylabel('Voltage in mV')
	plt.yticks([-65, -55, 0])
	plt.ylim(-90,)
	plt.xticks([x for x in range(0,len(t)//100,5)])
	plt.show()

def update(num, Vs, t, line):
	num *= 100
	line.set_data(t[:num], Vs[:num])
	# line.axes.axis([0, 10, 0, 1])
	return line,

def animate(frame, Vs, t, lines):
	frame *= 100
	for i in range(0, len(lines)):
		tmp = Vs[i*len(t):(i+1)*len(t)]
		lines[i].set_data(t[:frame], tmp[:frame])
	return lines

def main():
	# input current
	# iInput = np.random.uniform(0.01,0.05)
	iInput = 0.451
#	5,461 total neurons
	arr = [16,4,1]
	n = 10000
	dt = 0.01
	t=np.array([i for i in range(n)])

	# V=-64.9964
	# m=0.0530
	# h=0.5960
	# n=0.3177

	lib = ctypes.CDLL("/Users/fricker/Library/Developer/Xcode/DerivedData/NeuronalNetwork-ewjdcsoexwcnzucgtoqsskdeydmw/Build/Products/Debug/libengine.dylib")
	func = lib.run
	func.argtypes = [ctypes.c_double, ctypes.c_double, ctypes.c_int, ctypes.POINTER(ctypes.c_int), ctypes.c_int]
	func.restype = ndpointer(dtype = ctypes.c_double, shape = (n * sum(arr),))

	data = lib.run(iInput, dt, n, (ctypes.c_int * len(arr))(*arr), len(arr))
	print(data)
	plot_multiple_V_vs_t(data, iInput, t, dt, arr)
#	plot_V_vs_t(data[n * (sum(arr) - 1): n * (sum(arr))], iInput, t, dt)

	lib.deinitialize()
	

if __name__ == '__main__':
	main()
