import glob, os
import numpy as np
from datetime import datetime
import matplotlib.pyplot as plt
import matplotlib.colors as mcolors

psfile = sorted(glob.glob("*-power_status.csv"))[-1]
flfile = sorted(glob.glob("*-flight_log.csv"))[-1]
prfile = "PROFILE.TXT"
imgdir = "images"

figw, figh = 13., 5.

verbose = False

if not os.path.exists(prfile): prfile = None
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
    if data[0]:
        print("%s started %s" % (name, rising))
    elif len(trans) == 0:
        print("%s %s never" % (name, rising))
    else:
        for i, j in enumerate(trans):
            print("%s " % name, end='')
            if data[j]:
                print(falling, end='')
            else:
                print(rising, end='')
            print(" at %.2f s" % time[j], end='')
            if i > 0:
                if time[j] == time[trans[i-1]]:
                    trans[i]   = -1
                    trans[i-1] = -1
                    print(" [omitting pair]", end='')
            print()

    pad = []
    if len(trans) > 0 and data[trans[0]]:
        pad = [0]

    return np.asarray(pad+[x for x in trans if x >= 0])

def plot_transitions(time, trans):
    for i, t in enumerate(trans):
        for j in t:
            plt.axvline(time[j], linestyle=':', linewidth=1, color=tuple(mcolors.TABLEAU_COLORS)[i])

################################################################################

events = []
offset = 0.
if prfile is not None:
    with open(prfile, 'r') as fh:
        try:
            offset = float(fh.readline().split("OFFSET:")[1])
        except:
            offset = 0.
        print("%-13s %+10.{}f".format(len(str(float(offset)).split('.')[1])) % ("Offset:", offset))
        for line in fh:
            line = line.strip()
            if line.startswith('#'): continue
            v = line.split(',')
            val = [np.round(float(v[0]), len(str(float(offset)).split('.')[1])), v[1]]
            print("%-13s %+10.{}f".format(len(str(float(offset)).split('.')[1])) % (val[1], val[0]))
            events.append(val)
    print()

################################################################################

psdata, pshead, pscount = parse(psfile, "ps")

enable_trans = []
for c in range(4):
    enable_trans.append(detect_transitions("Channel %d" % c, psdata['enabled_%d' % c], psdata['Experiment Time'], "enabled", "disabled"))
print()

fault_trans = []
for c in range(4):
    fault_trans.append(detect_transitions("Channel %d" % c, psdata['fault_%d' % c], psdata['Experiment Time'], "faulted", "cleared"))
print()

channels = []
for c in range(4):
    if enable_trans[c].size != 0 or psdata['enabled_%d' % c][0] or \
       fault_trans[c].size  != 0 or psdata['fault_%d' % c][0]:
        channels.append(c)
enable_trans = [enable_trans[c] for c in channels]
fault_trans  = [fault_trans[c]  for c in channels]

jmin = 0; jmax = len(psdata['Experiment Time']) - 1
for i, trans in enumerate(enable_trans):
    if len(trans) > 0:
        if jmin == 0:
            jmin = trans.min()
        else:
            jmin = min([trans.min(), jmin])
        if jmax == len(psdata['Experiment Time']) - 1:
            jmax = trans.max()
        else:
            jmax = max([trans.max(), jmax])

tmin = psdata['Experiment Time'][jmin]
tmax = psdata['Experiment Time'][jmax]
jminoff = max(0, jmin - (len(psdata['Experiment Time']) - jmax - 1))
tminoff = psdata['Experiment Time'][jminoff]
tmaxoff = psdata['Experiment Time'][-1]

for c in channels:
    psdata['power_%d' % c] = psdata['volts_%d' % c] * psdata['current_%d' % c]

for column, title, units, trans in [("enabled", "Enabled", "boolean", enable_trans),
                                    ("fault",   "Faulted", "boolean", fault_trans),
                                    ("volts",   "Voltage", "V",       enable_trans),
                                    ("current", "Current", "A",       enable_trans),
                                    ("power",   "Power",   "W",       enable_trans)]:
    plt.figure(title, figsize=(figw, figh))
    plt.ticklabel_format(useOffset=False)
    plt.title(title)
    plt.ylabel("%s (%s)" % (title, units))
    plt.xlabel("Experiment Time (s)")
    for c in channels:
        plt.plot(psdata['Experiment Time'], psdata['%s_%d' % (column, c)], label=("Channel %d" % c), color=tuple(mcolors.TABLEAU_COLORS)[c])
    plot_transitions(psdata['Experiment Time'], trans)
    for t, v in events:
        plot_transitions(psdata['Experiment Time'], [[], [], [], [], [(np.abs(np.asarray(psdata['Experiment Time'])-t)).argmin()]])
    if units == "boolean":
        plt.yticks([0, 1])
        plt.ylim([-0.1, 1.1])
    plt.xlim(tminoff, tmaxoff)
    plt.tight_layout()
    # Borrowed from https://stackoverflow.com/a/4701285
    box = plt.gca().get_position()
    plt.gca().set_position([box.x0, box.y0 + box.height * 0.1, box.width, box.height * 0.9])
    plt.legend(loc='upper center', bbox_to_anchor=(0.5, -0.13), ncol=len(channels))
    plt.savefig(os.path.join(imgdir, "PS_%s.png" % title), dpi=150)

################################################################################

fldata, flhead, flcount = parse(flfile, "fl")

if 'Longitude' in fldata and 'Latitude' in fldata:
    with open("trajectory.kml", "w+") as fh:
        fh.write('<?xml version="1.0" encoding="UTF-8"?>\n<kml xmlns="http://www.opengis.net/kml/2.2">\n\t<Document>\n\t\t<name>Rocket Trajectory</name>\n\t\t<description>Rocket trajectory from {} flight</description>\n\t\t<Style id="yellowLineGreenPoly">\n\t\t\t<LineStyle>\n\t\t\t\t<color>7f00ffff</color>\n\t\t\t\t<width>4</width>\n\t\t\t</LineStyle>\n\t\t\t<PolyStyle>\n\t\t\t\t<color>7f00ff00</color>\n\t\t\t</PolyStyle>\n\t\t</Style>\n\t\t<Placemark>\n\t\t\t<name>{}</name>\n\t\t\t<description>Rocket trajectory from {} flight on {} at {}</description>\n\t\t\t<styleUrl>#yellowLineGreenPoly</styleUrl>\n\t\t\t<LineString>\n\t\t\t\t<extrude>1</extrude>\n\t\t\t\t<tessellate>1</tessellate>\n\t\t\t\t<altitudeMode>absolute</altitudeMode>\n\t\t\t\t<coordinates>\n'.format(*([flfile.split('-')[0]] + [flfile] + flfile.split('-')[:3])))
        fmt_string = "\t\t\t\t\t%.12f,%.12f,%.3f\n"
        decimate_n = 5
        if 'GPS_Altitude' in fldata:
            for i, (lon, lat, alt) in enumerate(zip(fldata['Longitude'], fldata['Latitude'], fldata['GPS_Altitude'])):
                if i % decimate_n == 0:
                    fh.write(fmt_string % (lon/np.pi*180.0, lat/np.pi*180.0, alt))
        else:
            for i, (lon, lat) in enumerate(zip(fldata['Longitude'], fldata['Latitude'])):
                if i % decimate_n == 0:
                    fh.write(fmt_string % (lon/np.pi*180.0, lat/np.pi*180.0, 0.))
        fh.write('\t\t\t\t</coordinates>\n\t\t\t</LineString>\n\t\t</Placemark>\n\t</Document>\n</kml>\n')

warnings = {}
for column, warning in [('liftoff_warn',      "Liftoff"),
                        ('rcs_warn',          "RCS thruster"),
                        ('drogue_chute_warn', "Drogue chute"),
                        ('landing_warn',      "Landing"),
                        ('chute_fault_warn',  "Chute fault")]:
    try:
        warnings[column] = detect_transitions("%s warning" % warning, np.asarray(fldata[column], dtype=bool), fldata['Experiment Time'], "tripped", "cleared")
        print()
    except KeyError:
        warnings[column] = None

trigger_trans = []
trigger_column = 'GPIOline'
if trigger_column in fldata:
    plt.figure("Trigger", figsize=(figw, figh))
    plt.ticklabel_format(useOffset=False)
    plt.title("Trigger")
    plt.ylabel("Trigger (boolean)")
    plt.xlabel("Experiment Time (s)")
    plt.plot(fldata['Experiment Time'], fldata[trigger_column])
    trigger_trans.append(detect_transitions("Trigger", fldata[trigger_column], fldata['Experiment Time'], "enabled", "disabled"))
    plot_transitions(fldata['Experiment Time'], trigger_trans)
    for t, v in events:
        plot_transitions(psdata['Experiment Time'], []*4+[[(np.abs(np.asarray(psdata['Experiment Time'])-t)).argmin()]])
    plt.yticks([0, 1])
    plt.ylim([-0.1, 1.1])
    plt.xlim(tminoff, tmaxoff)
    plt.tight_layout()
    plt.savefig(os.path.join(imgdir, "FL_Trigger.png"), dpi=150)

if 'Longitude' in fldata and 'Latitude' in fldata:
    plt.figure("Ground Trajectory", figsize=(figw, figh))
    plt.ticklabel_format(useOffset=False)
    plt.title("Ground Trajectory")
    plt.ylabel("Latitude (rad)")
    plt.xlabel("Longitude (rad)")
    plt.plot(fldata['Longitude'], fldata['Latitude'])
    plt.tight_layout()
    plt.savefig(os.path.join(imgdir, "FL_Trajectory.png"), dpi=150)

# Compute magnitude of velocity
if 'Velocity_Up' in fldata and 'Velocity_East' in fldata and 'Velocity_North' in fldata:
    fldata['Velocity_Magnitude'] = np.sqrt(fldata['Velocity_Up']**2+fldata['Velocity_East']**2+fldata['Velocity_North']**2)

# Unroll the roll array from range of -pi to +pi to continuous range
for rollvar in ['Roll_Phi', 'CC_roll']:
    if rollvar in fldata:
        for i in np.where(abs(np.diff(fldata[rollvar])) > np.pi)[0] + 1:
            fldata[rollvar][i:] += 2*np.pi * (fldata[rollvar][i-1] - fldata[rollvar][i])/abs(fldata[rollvar][i] - fldata[rollvar][i-1])

for title, units, scale, profile, warn, plots in [
        ["Altitude",         "ft",     1.00, True,  False, [('Altitude (AGL)', ('Altitude_AGL',                        )),
                                                            ('Altitude (GPS)', ('GPS_Altitude',                        ))]],
        ["Latitude",         "rad",    1.00, True,  False, [('Latitude',       ('Latitude',                            ))]],
        ["Longitude",        "rad",    1.00, True,  False, [('Longitude',      ('Longitude',                           ))]],
        ["Angular Velocity", "rad/s",  1.00, True,  False, [('P Axis',         ('Angular_Velocity_P',                  )),
                                                            ('Q Axis',         ('Angular_Velocity_Q',                  )),
                                                            ('R Axis',         ('Angular_Velocity_R',                  ))]],
        ["Velocity",         "ft/s",   1.00, True,  False, [('Up Axis',        ('Velocity_Up',                         )),
                                                            ('East Axis',      ('Velocity_East',                       )),
                                                            ('North Axis',     ('Velocity_North',                      )),
                                                            ('Magnitude',      ('Velocity_Magnitude',                  ))]],
        ["Acceleration",     "g",     32.17, True,  False, [('X Axis',         ('Acceleration_X',         'CCx_accel'  )),
                                                            ('Y Axis',         ('Acceleration_Y',         'CCy_accel'  )),
                                                            ('Z Axis',         ('Acceleration_Z',         'CCz_accel'  )),
                                                            ('Magnitude',      ('Acceleration_Magnitude', 'CCaccel_mag'))]],
        ["Orientation",      "rad",    1.00, True,  False, [('Roll (phi)',     ('Roll_Phi',               'CC_roll'    )),
                                                            ('Pitch (theta)',  ('Pitch_Theta',            'CC_pitch'   )),
                                                            ('Yaw (psi)',      ('Yaw_Psi',                'CC_yaw'     ))]]]:
    plt.figure(title, figsize=(figw, figh))
    plt.ticklabel_format(useOffset=False)
    plt.title(title)
    plt.ylabel("%s (%s)" % (title, units))
    plt.xlabel("Experiment Time (s)")
    numplots = 0
    for label, columns in plots:
        for column in columns:
            if column in fldata:
                plt.plot(fldata['Experiment Time'], fldata[column]/scale, label=label)
                numplots += 1
                break
    if not plt.gca().lines:
        plt.close(title)
        continue
    plot_transitions(psdata['Experiment Time'], enable_trans+trigger_trans)
    if profile:
        for t, v in events:
            plot_transitions(psdata['Experiment Time'], []*4+[[(np.abs(np.asarray(psdata['Experiment Time'])-t)).argmin()]])
    if warn:
        for k, v in warnings.items():
            if v is None: continue
            for i in range(len(v)//2):
                plt.axvspan(fldata['Experiment Time'][v[2*i]], fldata['Experiment Time'][v[2*i+1]], alpha=0.1, color='red')
    plt.xlim(tminoff, tmaxoff)
    plt.tight_layout()
    if numplots > 1:
        # Borrowed from https://stackoverflow.com/a/4701285
        box = plt.gca().get_position()
        plt.gca().set_position([box.x0, box.y0 + box.height * 0.1, box.width, box.height * 0.9])
        plt.legend(loc='upper center', bbox_to_anchor=(0.5, -0.13), ncol=numplots)
    plt.savefig(os.path.join(imgdir, "FL_%s.png" % title.replace(' ', '_')), dpi=150)

################################################################################

plt.show()
