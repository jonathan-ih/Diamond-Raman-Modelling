import numpy as np
import matplotlib.pyplot as plt
import sys

pressure_file = sys.argv[1]
output_file = sys.argv[2]

pressures = np.loadtxt(pressure_file)

fig, ax = plt.subplots()
ax.plot(pressures[:,0], pressures[:,1])
ax.set_xlabel("Depth")
ax.set_ylabel("Pressure (GPa)")

plt.savefig(output_file)
