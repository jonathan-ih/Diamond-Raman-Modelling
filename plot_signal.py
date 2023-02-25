import numpy as np
import matplotlib.pyplot as plt
import sys

min_freq = 1000
max_freq = 1500
num_sample_points = 1000
signal_file = sys.argv[1]

signal = np.loadtxt(signal_file)

fig, ax = plt.subplots()

ax.plot(signal[:,0], signal[:,1])
ax.set_xlabel("Frequency (cm$^\mathregular{-1}$)")
ax.set_ylabel("Intensity")
plt.show()