import os, xlrd, pickle
import numpy as np

VCC = 5.0
BINS = 1024

def parse_electrometer_data(fn, cuttimemin=None, cuttimemax=None, millivolt=False, zpo=None):
    print("Parsing payload data file {:} ...".format(fn))
    events = [];
    C = []
    T = [[], [], [], [], [], [], [], []]
    V = [[], [], [], [], [], [], [], []]
    S = [[], [], [], [], [], [], [], []]
    with open(fn, "r") as fh:
        for line in fh:
            eventline = [x.strip() for x in line.split(":")]
            if len(eventline) == 2:
                events.append([int(eventline[0])]+eventline[1:])
                continue
            dataline = [x.strip() for x in line.split(",")]
            if len(dataline) == 9:
                try:
                    if dataline[-1].endswith("[ERROR]"):
                        print("  [WARN] ", end="")
                        try:    print("%7.3f: " % (int(dataline[0])/1000.), end="")
                        except: print("         ", end="")
                        print("Error detected in log", end="")
                        try:    print(" (likely channel %d skipped)" % ((int(dataline[2])-1)%8), end="")
                        except: pass
                        print()
                        dataline[-1] = dataline[-1][:-7]
                    elif dataline[-1].endswith("[REPAIRED]"):
                        print("  [WARN] ", end="")
                        try:    print("%7.3f: " % (int(dataline[0])/1000.), end="")
                        except: print("         ", end="")
                        print("Repair detected in log", end="")
                        print()
                        dataline[-1] = dataline[-1][:-11]
                    dataline = [eval(x) for x in dataline]
                    c  = dataline[2]
                    t  = dataline[3]/1000.0
                    if cuttimemin is not None and t <= cuttimemin: continue
                    if cuttimemax is not None and t >= cuttimemax: continue
                    v0 = dataline[4]/dataline[5]
                    v  = VCC*v0/BINS
                    if zpo is not None:
                        v -= zpo[c]
                    if millivolt:
                        v *= 1000.
                    s0 = np.sqrt(dataline[7]/(dataline[5]-1))
                    s  = VCC*s0/BINS
                    if millivolt:
                        s *= 1000.
                    C.append(c)
                    T[c].append(t)
                    V[c].append(v)
                    S[c].append(s)
                    if abs(v0-dataline[6]) > 1:
                        print("  [WARN] %7.3f: Mean %.3f differs from %.3f significantly (%.3f)!" % (t, v0, dataline[6], abs(v0-dataline[6])))
                    if abs(s0-np.sqrt(dataline[8])) > 0.001:
                        print("  [WARN] %7.3f: Variance %.3f differs from %.3f significantly (%.3f)!" % (t, s0**2, dataline[8], abs(s0-np.sqrt(dataline[7]))))
                except ValueError:
                    continue
    return events, C, T, V, S
