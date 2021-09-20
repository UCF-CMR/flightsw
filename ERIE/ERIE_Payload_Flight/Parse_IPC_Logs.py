import glob, os
import numpy as np
from datetime import datetime
import matplotlib.pyplot as plt
import matplotlib.colors as mcolors

psfile = sorted(glob.glob("*-power_status.csv"))[-1]
flfile = sorted(glob.glob("*-flight_log.csv"))[-1]
imgdir = "images"

figw, figh = 13., 5.

channels = (0, 1) # range(4)

verbose = False

if not os.path.exists(imgdir): os.mkdir(imgdir)

def parse(filename, style):
    data = {}
    count = 1
    error = 0

    print("Parsing '%s' as " % filename, end='')
    if   style == "ps": print("power status")
    elif style == "fl": print("flight log")

    with open(filename, 'r') as fh:
        head = [x.strip() for x in fh.readline().split(',')]
        for i, h in enumerate(head):
            data[h] = []
            print("%2d: %s" % (i, h))
        start = None
        for line in fh:
            count += 1
            line = [x.strip() for x in line.split(',')]
            if len(line) > len(head):
                error += 1
                if verbose: print("Invalid line %d (%d elements instead of %d) detected! Skipping!" % (count, len(line), len(head)))
                continue
            elif len(line) < len(head):
                error += 1
                if verbose: print("Invalid line %d (%d elements instead of %d) detected! Skipping!" % (count, len(line), len(head)))
                continue
            if style == "ps":
                start = psparse(line, head, start)
            elif style == "fl":
                start = flparse(line, head, start)
            for h in head:
                data[h].append(line[head.index(h)])
    if error > 0: print("Invalid lines (%{}d/%d | %.1f%%) detected!".format(len(str(count))) % (error, count, 100.*error/count))
    print()
    for h in head:
        data[h] = np.array(data[h])
    return data, head, count

def psparse(line, pshead, psstart):
    for h in pshead:
        if h == "UTC Time":
            if psstart is None:
                psstart = datetime.strptime(line[pshead.index(h)], "%m/%d/%Y %H:%M:%S")
                line[pshead.index(h)] = psstart.strftime("%H:%M:%S")
            line[pshead.index(h)] = datetime.strptime(psstart.strftime("%m/%d/%Y ") + line[pshead.index(h)], "%m/%d/%Y %H:%M:%S")
        elif h.startswith("enabled") or h.startswith("fault"):
            line[pshead.index(h)] = (line[pshead.index(h)] == '1')
        else:
            line[pshead.index(h)] = float(line[pshead.index(h)])
    return psstart

def flparse(line, flhead, flstart):
    for h in flhead:
        if h == "UTC Time":
            if flstart is None:
                flstart = datetime.strptime(line[flhead.index(h)], "%m/%d/%Y %H:%M:%S")
                line[flhead.index(h)] = flstart.strftime("%H:%M:%S")
            line[flhead.index(h)] = datetime.strptime(flstart.strftime("%m/%d/%Y ") + line[flhead.index(h)], "%m/%d/%Y %H:%M:%S")
        elif h == "GPIOline":
            line[flhead.index(h)] = (line[flhead.index(h)] == '1')
        else:
            line[flhead.index(h)] = float(line[flhead.index(h)])
    return flstart

def detect_transitions(name, data, time, rising="enabled", falling="disabled"):
    trans = np.where(data[1:] ^ data[:-1])[0]
    if len(trans) == 0:
        print("%s %s never" % (name, rising))
    else:
        for j in trans:
            print("%s " % name, end='')
            if data[j]:
                print(falling, end='')
            else:
                print(rising, end='')
            print(" at %.2f s" % time[j])
    return trans

def plot_transitions(time, trans):
    for i, t in enumerate(trans):
        for j in t:
            plt.axvline(time[j], linestyle=':', linewidth=1, color=tuple(mcolors.TABLEAU_COLORS)[i])

################################################################################

psdata, pshead, pscount = parse(psfile, "ps")

enable_trans = []
for i in channels:
    enable_trans.append(detect_transitions("Channel %d" % i, psdata['enabled_%d' % i], psdata['Experiment Time'], "enabled", "disabled"))
print()

fault_trans = []
for i in channels:
    fault_trans.append(detect_transitions("Channel %d" % i, psdata['fault_%d' % i], psdata['Experiment Time'], "faulted", "cleared"))
print()

jmin = 0; jmax = len(psdata['Experiment Time']) - 1
for i, trans in enumerate(enable_trans):
    if len(trans) > 0:
        if jmin is None: jmin = trans.min()
        else:            jmin = min([trans.min(), jmin])
        if jmax is None: jmax = trans.max()
        else:            jmax = max([trans.max(), jmax])
tmin = psdata['Experiment Time'][jmin]
tmax = psdata['Experiment Time'][jmax]
tminoff = psdata['Experiment Time'][jmin - (len(psdata['Experiment Time']) - jmax - 1)]
tmaxoff = psdata['Experiment Time'][-1]

plt.figure(figsize=(figw, figh))
plt.title("Enabled")
plt.ylabel("Enabled (boolean)")
plt.xlabel("Experiment Time (s)")
for i in channels:
    plt.plot(psdata['Experiment Time'], psdata['enabled_%d' % i], label=("Channel %d" % i))
plot_transitions(psdata['Experiment Time'], enable_trans)
plt.xlim(tminoff, tmaxoff)
plt.legend()
plt.tight_layout()
plt.savefig(os.path.join(imgdir, "PS_Enabled.png"), dpi=150)

plt.figure(figsize=(figw, figh))
plt.title("Faulted")
plt.ylabel("Faulted (boolean)")
plt.xlabel("Experiment Time (s)")
for i in channels:
    plt.plot(psdata['Experiment Time'], psdata['fault_%d' % i], label=("Channel %d" % i))
plot_transitions(psdata['Experiment Time'], fault_trans)
plt.xlim(tminoff, tmaxoff)
plt.legend()
plt.tight_layout()
plt.savefig(os.path.join(imgdir, "PS_Faulted.png"), dpi=150)

#plt.figure(figsize=(figw, figh))
#plt.title("Voltage")
#plt.ylabel("Voltage (V)")
#plt.xlabel("Experiment Time (s)")
#for i in channels:
#    plt.plot(psdata['Experiment Time'], psdata['volts_%d' % i], label=("Channel %d" % i))
#plot_transitions(psdata['Experiment Time'], enable_trans)
#plt.xlim(tminoff, tmaxoff)
#plt.legend()
#plt.tight_layout()
#plt.savefig(os.path.join(imgdir, "PS_Voltage.png"), dpi=150)

#plt.figure(figsize=(figw, figh))
#plt.title("Current")
#plt.ylabel("Current (A)")
#plt.xlabel("Experiment Time (s)")
#for i in channels:
#    plt.plot(psdata['Experiment Time'], psdata['current_%d' % i], label=("Channel %d" % i))
#plot_transitions(psdata['Experiment Time'], enable_trans)
#plt.xlim(tminoff, tmaxoff)
#plt.legend()
#plt.tight_layout()
#plt.savefig(os.path.join(imgdir, "PS_Current.png"), dpi=150)

#plt.figure(figsize=(figw, figh))
#plt.title("Power")
#plt.ylabel("Power (W)")
#plt.xlabel("Experiment Time (s)")
#for i in channels:
#    plt.plot(psdata['Experiment Time'], psdata['volts_%d' % i]*psdata['current_%d' % i], label=("Channel %d" % i))
#plot_transitions(psdata['Experiment Time'], enable_trans)
#plt.xlim(tminoff, tmaxoff)
#plt.legend()
#plt.tight_layout()
#plt.savefig(os.path.join(imgdir, "PS_Power.png"), dpi=150)

################################################################################

fldata, flhead, flcount = parse(flfile, "fl")

plt.figure(figsize=(figw, figh))
plt.title("Voltage")
plt.ylabel("Voltage (V)")
plt.xlabel("Experiment Time (s)")
for j, (data, label) in enumerate([(psdata, "PS"), (fldata, "FL")]):
    for i in channels:
        if ('volts_%d' % i) in data:
            if j == 0: style = ':'
            else:      style = '-'
            plt.plot(data['Experiment Time'], data['volts_%d' % i], label=("Channel %d %s" % (i, label)), color=tuple(mcolors.TABLEAU_COLORS)[i], linestyle=style)
plot_transitions(psdata['Experiment Time'], enable_trans)
plt.xlim(tminoff, tmaxoff)
plt.legend()
plt.tight_layout()
plt.savefig(os.path.join(imgdir, "FL_Voltage.png"), dpi=150)

plt.figure(figsize=(figw, figh))
plt.title("Current")
plt.ylabel("Current (A)")
plt.xlabel("Experiment Time (s)")
for j, (data, label) in enumerate([(psdata, "PS"), (fldata, "FL")]):
    for i in channels:
        if ('current_%d' % i) in data:
            if j == 0: style = ':'
            else:      style = '-'
            plt.plot(data['Experiment Time'], data['current_%d' % i], label=("Channel %d %s" % (i, label)), color=tuple(mcolors.TABLEAU_COLORS)[i], linestyle=style)
plot_transitions(psdata['Experiment Time'], enable_trans)
plt.xlim(tminoff, tmaxoff)
plt.legend()
plt.tight_layout()
plt.savefig(os.path.join(imgdir, "FL_Current.png"), dpi=150)

plt.figure(figsize=(figw, figh))
plt.title("Power")
plt.ylabel("Power (W)")
plt.xlabel("Experiment Time (s)")
for j, (data, label) in enumerate([(psdata, "PS"), (fldata, "FL")]):
    for i in channels:
        if ('volts_%d' % i) in data and ('current_%d' % i) in data:
            if j == 0: style = ':'
            else:      style = '-'
            plt.plot(psdata['Experiment Time'], psdata['volts_%d' % i]*psdata['current_%d' % i], label=("Channel %d %s" % (i, label)), color=tuple(mcolors.TABLEAU_COLORS)[i], linestyle=style)
plot_transitions(psdata['Experiment Time'], enable_trans)
plt.xlim(tminoff, tmaxoff)
plt.legend()
plt.tight_layout()
plt.savefig(os.path.join(imgdir, "FL_Power.png"), dpi=150)

trigger_trans = []
if 'GPIOline' in fldata:
    plt.figure(figsize=(figw, figh))
    plt.title("Trigger")
    plt.ylabel("Trigger (boolean)")
    plt.xlabel("Experiment Time (s)")
    plt.plot(fldata['Experiment Time'], fldata['GPIOline'])
    trigger_trans.append(detect_transitions("Trigger", fldata['GPIOline'], fldata['Experiment Time'], "enabled", "disabled"))
    plot_transitions(fldata['Experiment Time'], trigger_trans)
    plt.xlim(tminoff, tmaxoff)
    plt.tight_layout()
    plt.savefig(os.path.join(imgdir, "FL_Trigger.png"), dpi=150)

#plt.figure(figsize=(figw, figh))
#plt.title("Latitude")
#plt.ylabel("Latitude")
#plt.xlabel("Experiment Time (s)")
#plt.plot(fldata['Experiment Time'], fldata['Latitude'])
#plt.xlim(tminoff, tmaxoff)
#plt.tight_layout()
#plt.savefig(os.path.join(imgdir, "FL_Latitude.png"), dpi=150)

#plt.figure(figsize=(figw, figh))
#plt.title("Longitude")
#plt.ylabel("Longitude")
#plt.xlabel("Experiment Time (s)")
#plt.plot(fldata['Experiment Time'], fldata['Longitude'])
#plt.xlim(tminoff, tmaxoff)
#plt.tight_layout()
#plt.savefig(os.path.join(imgdir, "FL_Longitude.png"), dpi=150)

plt.figure(figsize=(figw, figh))
plt.title("Ground Trajectory")
plt.ylabel("Latitude")
plt.xlabel("Longitude")
plt.plot(fldata['Longitude'], fldata['Latitude'])
plt.tight_layout()
plt.savefig(os.path.join(imgdir, "FL_Trajectory.png"), dpi=150)

if   'CCx_accel'              in fldata: xaccel = 'CCx_accel'
elif 'Acceleration_X'         in fldata: xaccel = 'Acceleration_X'
if   'CCy_accel'              in fldata: yaccel = 'CCy_accel'
elif 'Acceleration_Y'         in fldata: yaccel = 'Acceleration_Y'
if   'CCz_accel'              in fldata: zaccel = 'CCz_accel'
elif 'Acceleration_Z'         in fldata: zaccel = 'Acceleration_Z'
if   'CCaccel_mag'            in fldata: accelm = 'CCaccel_mag'
elif 'Acceleration_Magnitude' in fldata: accelm = 'Acceleration_Magnitude'
plt.figure(figsize=(figw, figh))
plt.title("Acceleration")
plt.ylabel("Acceleration (g)")
plt.xlabel("Experiment Time (s)")
plt.plot(fldata['Experiment Time'], fldata[xaccel]/32.17, label="X Axis")
plt.plot(fldata['Experiment Time'], fldata[yaccel]/32.17, label="Y Axis")
plt.plot(fldata['Experiment Time'], fldata[zaccel]/32.17, label="Z Axis")
plt.plot(fldata['Experiment Time'], fldata[accelm]/32.17, label="Magnitude")
plot_transitions(psdata['Experiment Time'], enable_trans+trigger_trans)
plt.xlim(tminoff, tmaxoff)
plt.legend()
plt.tight_layout()
plt.savefig(os.path.join(imgdir, "FL_Acceleration.png"), dpi=150)

if   'CC_roll'     in fldata: roll  = 'CC_roll'
elif 'Roll_Phi'    in fldata: roll  = 'Acceleration_X'
if   'CC_pitch'    in fldata: pitch = 'CC_pitch'
elif 'Pitch_Theta' in fldata: pitch = 'Acceleration_Y'
if   'CC_yaw'      in fldata: yaw   = 'CC_yaw'
elif 'Yaw_Psi'     in fldata: yaw   = 'Acceleration_Z'
plt.figure(figsize=(figw, figh))
plt.title("Capsule Orientation")
plt.ylabel("Orientation")
plt.xlabel("Experiment Time (s)")
plt.plot(fldata['Experiment Time'], fldata[roll], label="Roll")
plt.plot(fldata['Experiment Time'], fldata[pitch], label="Pitch")
plt.plot(fldata['Experiment Time'], fldata[yaw], label="Yaw")
plot_transitions(psdata['Experiment Time'], enable_trans+trigger_trans)
plt.xlim(tminoff, tmaxoff)
plt.legend()
plt.tight_layout()
plt.savefig(os.path.join(imgdir, "FL_Orientation.png"), dpi=150)

################################################################################

plt.show()
