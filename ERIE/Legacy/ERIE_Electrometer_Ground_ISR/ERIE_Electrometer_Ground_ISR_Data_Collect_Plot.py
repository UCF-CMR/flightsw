import serial, os
from time import sleep
from datetime import datetime
import matplotlib.pyplot as plt
import numpy as np

fn = datetime.now().strftime("%Y%m%d-%H%M%S-ISR") + ".csv"

com = 0 # NOTE: Update this to match COM port for Arduino
if os.path.exists("/dev/ttyACM%d" % com):
    ser = serial.Serial("/dev/ttyACM%d" % com, baudrate=115200)
elif os.path.exists("/dev/ttyS%d" % com):
    ser = serial.Serial("/dev/ttyS%d" % com, baudrate=115200)
else:
    ser = serial.Serial("COM%d" % com, baudrate=115200)

ser.setDTR(False)
sleep(1)
ser.flushInput()
ser.setDTR(True)

def parseLine(line):
    try:
        if len(line) == 3 and line[2].startswith("RESET"):
            return int(line[0]), int(line[1]), line[2]
        else:
            line = [eval(x) for x in line]
            return line[0], line[1], line[2:]
    except:
        return None, None, None

plt.ion()

fig = plt.figure(figsize=(16,8))
ax = fig.add_subplot(111)

plots = [ax.scatter([], [], marker='.', label=("Channel %d" % (c+1))) for c in range(8)]
ax.set_xlim(0, 0.0)
ax.set_ylim(0.0, 5.0)
ymin = None; ymax = None
ax.set_title("Real-Time Electrometer Measurements")
ax.set_xlabel("Time (s)")
ax.set_ylabel("Voltage (V)")
ax.legend()
fig.set_tight_layout(True)
fig.canvas.flush_events()
plt.show(block=False)

with open(fn, "w+") as fh:
    while True:
        try:
            line = ser.readline()
            line = [x.strip().decode() for x in line.split(b",")]
            c, t, v = parseLine(line)
            if c is None or t is None or v is None:
                print(line)
            else:
                print(c, t, v)
                if type(t) == int and t > 300000:
                    break
            if c is not None and t is not None and v is not None:
                if len(v) == 5 or (type(v) == str and v.startswith("RESET")):
                    fh.write(", ".join(line) + "\n")
                    fh.flush()
                else:
                    print("[WARN] Skipping invalid data!")
                if c == c:
                    try:
                        calval = 5./1024.*float(v[2])
                        data = np.array([[t/1000., calval]])
                    except ValueError:
                        print("[WARN] Not plotting invalid data!")
                    array = plots[c].get_offsets()
                    array = np.append(array, data, axis=0)
                    plots[c].set_offsets(array)

                    ax.set_xlim(0, array[:,0].max() + 0.5)
                    if ymin is None: ymin = calval
                    if ymax is None: ymax = calval
                    if calval > ymax:
                        ymax = calval
                        ax.set_ylim(ymin, ymax)
                    if calval < ymin:
                        ymin = calval
                        ax.set_ylim(ymin, ymax)

                    fig.canvas.flush_events()
                    plt.show(block=False)
            else:
                print("[WARN] Skipping invalid data!")
        except KeyboardInterrupt:
            break

plt.ioff()
plt.show(block=True)

ser.close()
