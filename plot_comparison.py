import numpy as np
import matplotlib.pyplot as plt
import sys

simulated_signal_file = sys.argv[1]
fitted_signal_file = sys.argv[2]
simulated_pressure_file = sys.argv[3]
fitted_pressure_file = sys.argv[4]
output_file = sys.argv[5]

simulated_signal = np.loadtxt(simulated_signal_file)
fitted_signal = np.loadtxt(fitted_signal_file)
simulated_pressure = np.loadtxt(simulated_pressure_file)
fitted_pressure = np.loadtxt(fitted_pressure_file)

fig, ax = plt.subplots(1, 2, figsize=(10, 5))

ax[0].plot(simulated_signal[:,0], simulated_signal[:,1], label="Simulated")
ax[0].plot(fitted_signal[:,0], fitted_signal[:,1], label="Fitted")
ax[1].plot(simulated_pressure[:,0], simulated_pressure[:,1], label="Simulated")
ax[1].plot(fitted_pressure[:,0], fitted_pressure[:,1], label="Fitted")

ax[0].legend()
ax[0].set_title("Raman signal")
ax[0].set_xlabel("Frequency (cm$^\mathregular{-1}$)")
ax[0].set_ylabel("Intensity")
ax[1].legend()
ax[1].set_title("Pressure profile")
ax[1].set_xlabel("Depth")
ax[1].set_ylabel("Pressure (GPa)")

plt.savefig(output_file)

