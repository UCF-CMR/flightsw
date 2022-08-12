import serial, os
from time import sleep
from datetime import datetime

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
            if c is not None and t is not None and v is not None and (len(v) == 5 or (type(v) == str and v.startswith("RESET"))):
                fh.write(", ".join(line) + "\n")
                fh.flush()
            else:
                print("[WARN] Skipping invalid data!")
        except KeyboardInterrupt:
            break

ser.close()
