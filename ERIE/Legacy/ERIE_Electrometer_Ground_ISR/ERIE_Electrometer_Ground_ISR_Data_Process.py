import os, sys, glob
import numpy as np
import matplotlib.pyplot as plt

VCC = 5.0
BINS = 1024
error = False
connect = True

if len(sys.argv) > 1 and os.path.exists(sys.argv[1]):
    fn = sys.argv[1]
else:
    fn = sorted(glob.glob("*-ISR*.csv"))[-1]

print("Parsing {:}".format(fn))

C = []
T = [[], [], [], [], [], [], [], []]
V = [[], [], [], [], [], [], [], []]
S = [[], [], [], [], [], [], [], []]
with open(fn, "r") as fh:
    for line in fh:
        line = [x.strip() for x in line.split(",")]
        if len(line) == 7:
            try:
                line = [eval(x) for x in line]
                c  = line[0]
                t  = line[1]/1000.0
                v0 = line[2]/line[3]
                v  = VCC*v0/BINS
                s0 = np.sqrt(line[5]/(line[3]-1))
                s  = VCC*s0/BINS
                C.append(c)
                T[c].append(t)
                V[c].append(v)
                S[c].append(s)
                if abs(v0-line[4]) > 1:
                    print("[WARN] Mean %.3f differs from %.3f significantly (%.3f)!" % (v0, line[4], abs(v0-line[4])))
                if abs(s0-np.sqrt(line[6])) > 0.001:
                    print("[WARN] Variance %.3f differs from %.3f significantly (%.3f)!" % (s0**2, line[6], abs(s0-np.sqrt(line[6]))))
            except ValueError:
                continue
        elif len(line) == 3:
            if line[2].startswith("RESET"):
                print("%d %.3f %s" % (int(line[0]), int(line[1])/1000.0, line[2]))

for c in range(8):
    if error:
        if connect:
            plt.errorbar(T[c], V[c], S[c], label="Sensor {:}".format(c+1), fmt=".", ms=2., capsize=2., errorevery=np.ceil(len(T[0])/100))
        else:
            plt.errorbar(T[c], V[c], S[c], label="Sensor {:}".format(c+1), fmt=".", ms=2., ls="none", capsize=2., errorevery=np.ceil(len(T[0])/100))
    else:
        if connect:
            plt.plot(T[c], V[c], label="Sensor {:}".format(c+1))
        else:
            plt.scatter(T[c], V[c], label="Sensor {:}".format(c+1), s=2.)
plt.xlabel("Elapsed Time (s)")
plt.ylabel("Sensor Output (V)")
plt.title(fn)
plt.legend()
plt.show()
