import os, sys, glob
import numpy as np
import matplotlib.pyplot as plt

VCC = 5.0
BINS = 1024
error = False
connect = True
profile = "PROFILE.TXT"

if len(sys.argv) > 1 and os.path.exists(sys.argv[1]):
    fn = sys.argv[1]
else:
    fn = sorted(glob.glob("[0-9]**.TXT"))[-1]

print("Parsing {:}".format(fn))

profhead = {}
profdata = []
if os.path.exists(profile) and os.path.isfile(profile):
    print("Flight profile detected! Parsing %s ..." % profile)
    with open(profile, 'r') as fh:
        for line in fh:
            if not line.startswith('#'):
                line = line.strip().split(':')
                if len(line) == 2:
                    profhead[line[0]] = eval(line[1])
                elif len(line) == 1:
                    dataline = line[0].split(',')
                    profdata.append([float(dataline[0])]+dataline[1:])

    if 'OFFSET' in profhead:
        for data in profdata:
            data[0] += profhead['OFFSET']

info = [];
C = []
T = [[], [], [], [], [], [], [], []]
V = [[], [], [], [], [], [], [], []]
S = [[], [], [], [], [], [], [], []]
with open(fn, "r") as fh:
    for line in fh:
        infoline = [x.strip() for x in line.split(":")]
        if len(infoline) == 2:
            info.append([int(infoline[0])]+infoline[1:])
            continue
        dataline = [x.strip() for x in line.split(",")]
        if len(dataline) == 9:
            try:
                if dataline[-1].endswith("[ERROR]"):
                    print("ERROR")
                    dataline[-1] = dataline[-1][:-7]
                dataline = [eval(x) for x in dataline]
                c  = dataline[2]
                t  = dataline[3]/1000.0
                v0 = dataline[4]/dataline[5]
                v  = VCC*v0/BINS
                s0 = np.sqrt(dataline[7]/(dataline[5]-1))
                s  = VCC*s0/BINS
                C.append(c)
                T[c].append(t)
                V[c].append(v)
                S[c].append(s)
                if abs(v0-dataline[6]) > 1:
                    print("[WARN] Mean %.3f differs from %.3f significantly (%.3f)!" % (v0, dataline[6], abs(v0-dataline[6])))
                if abs(s0-np.sqrt(dataline[8])) > 0.001:
                    print("[WARN] Variance %.3f differs from %.3f significantly (%.3f)!" % (s0**2, dataline[8], abs(s0-np.sqrt(dataline[7]))))
            except ValueError:
                continue

ymax = max([max(V[c]) for c in range(8)])
ymin = min([min(V[c]) for c in range(8)])

plt.figure(figsize=(16,9))
for time, data in info:
    if data.startswith("Transitioning to state "):
        text = data[len("Transitioning to state "):]
        if not text.startswith("DELAY"):
            plt.axvline(time/1000., linestyle=':', linewidth=0.75, color="gray")
            plt.annotate(text, (time/1000., ymin), rotation=90, color="gray")
    print("%{:}.3f: %s".format(len(str(info[-1][0]))+1) % (time/1000., data))

if profdata:
    print("\nVehicle profile events:")
    for time, data in profdata:
        plt.axvline(time, linestyle=':', linewidth=0.75, color="red")
        plt.annotate(data.upper().replace('/', '\n'), (time,ymin), rotation=90, color="red")
        print("%4d: %s" % (time, data))

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
plt.tight_layout()
box = plt.gca().get_position()
plt.gca().set_position([box.x0, box.y0 + box.height * 0.05, box.width, box.height * 0.95])
plt.legend(loc='upper center', bbox_to_anchor=(0.5, -0.07), ncol=8)
plt.savefig(fn.replace(".TXT", ".png").replace(".txt", ".png"), dpi=150)
plt.show()
