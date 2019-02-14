#!/usr/bin/env python3

from picamera import PiCamera
from time import sleep
import datetime
import time

# Duration of each video file in seconds
runtime = 1200

# Camera settings
res = (640, 480) # Resolution
fps = 120        # Framerate

# Define loop flag (handle Ctrl+C)
run = True

print("Connecting to camera with %dx%d resolution at %d fps" % (res + (fps,)))
with PiCamera(resolution=res, framerate=fps) as camera:
    while run:
        try:
            date = datetime.datetime.now().strftime("%Y_%m_%d_%H_%M_%S")
            filename = "/home/pi/Desktop/Vids/" + date + ".h264"
            print("Starting %d second recording" % runtime)
            print("Saving stream to %s" % filename)
            camera.start_recording(filename)
            camera.wait_recording(runtime)
        except KeyboardInterrupt:
            print("Keyboard interrupt detected! Aborting!")
            run = False
        except:
            print("Error detected! Restarting!")
            break
        finally:
            if camera.recording:
                print("Stopping recording")
                camera.stop_recording()
