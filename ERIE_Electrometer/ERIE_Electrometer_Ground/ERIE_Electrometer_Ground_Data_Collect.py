import serial, os
from time import sleep
from datetime import datetime

fn = datetime.now().strftime("%Y%m%d-%H%M%S") + ".csv"

com = 9 # NOTE: Update this to match COM port for Arduino
if os.path.exists("/dev/ttyS%d" % com):
    ser = serial.Serial("/dev/ttyS%d" % com)
else:
    ser = serial.Serial("COM%d" % com)

ser.setDTR(False)
sleep(1)
ser.flushInput()
ser.setDTR(True)

def parseLine(line):
    try:
        if line[1].startswith("RESET"):
            return float(line[0]), [line[1]]+[float(x) for x in line[2:]]
        else:
            line = [float(x) for x in line]
            return line[0], line[1:]
    except:
        return None, None

with open(fn, "w+") as fh:
    while True:
        try:
            line = ser.readline()
            line = [x.strip().decode() for x in line.split(b",")]
            t, v = parseLine(line)
            if t is None or v is None:
                print(line)
            else:
                print(t, v)
                if type(t) == float and t > 300:
                    break
            if t is not None and v is not None and (len(v) == 8 or (len(v) == 3 and v[0].startswith("RESET"))):
                fh.write(", ".join(line) + "\n")
                fh.flush()
            else:
                print("[WARN] Skipping invalid data!")
        except KeyboardInterrupt:
            break

ser.close()
