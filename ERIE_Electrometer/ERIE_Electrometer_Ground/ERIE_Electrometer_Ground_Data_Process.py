import os, sys, glob
import numpy as np
import matplotlib.pyplot as plt

if len(sys.argv) > 1 and os.path.exists(sys.argv[1]):
    fn = sys.argv[1]
else:
    fn = sorted(set(glob.glob("*.csv"))-set(glob.glob("*-ISR*.csv")))[-1]

print("Parsing {:}".format(fn))

T = []; V = []
with open(fn, "r") as fh:
    for line in fh:
        line = [x.strip() for x in line.split(",")]
        if len(line) == 9:
            try:
                line = [float(x) for x in line]
                T.append(line[0])
                V.append(line[1:])
            except ValueError:
                continue
        elif len(line) == 4:
            if line[1].startswith("RESET"):
                print("RESET", int(line[1].split(' ')[1])+1, line[0], line[2], line[3])

T = np.array(T)
dt = np.mean(T[1:]-T[:-1])
V = np.array(V)
T -= T[0]

for c in range(8):
    plt.plot(T+c*dt/8., V[:, c], label="Sensor {:}".format(c+1))
plt.xlabel("Elapsed Time (s)")
plt.ylabel("Sensor Output (V)")
plt.title(fn)
plt.legend()
plt.show()
