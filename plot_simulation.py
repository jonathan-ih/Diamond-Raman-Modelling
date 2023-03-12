import numpy as np
import matplotlib.pyplot as plt
import sys

simulated_signal_file = sys.argv[1]
simulated_pressure_file = sys.argv[2]
output_file = sys.argv[3]

simulated_signal = np.loadtxt(simulated_signal_file)
simulated_pressure = np.loadtxt(simulated_pressure_file)

fig, ax = plt.subplots(1, 2, figsize=(10, 5))

ax[0].plot(simulated_signal[:,0], simulated_signal[:,1])
ax[1].plot(simulated_pressure[:,0], simulated_pressure[:,1])

ax[0].set_title("Raman signal")
ax[0].set_xlabel("Frequency (cm$^\mathregular{-1}$)")
ax[0].set_ylabel("Intensity")
ax[1].set_title("Pressure profile")
ax[1].set_xlabel("Depth")
ax[1].set_ylabel("Pressure (GPa)")

plt.savefig(output_file)

