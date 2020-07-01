#Basic camera code to test connection of PiCamera

from picamera import PiCamera
from time import sleep
import datetime
import time

camera = PiCamera()

camera.rotation = 180
camera.start_preview()

date = datetime.datetime.now().strftime("%Y_%m_%d_%H_%M_%S")
#fname = '/home/pi/jacmus_vid.h264'
camera.start_recording("/home/pi/Desktop/Vids/"+date+ ".h264")

camera.wait_recording(15)

camera.stop_recording()
camera.stop_preview()
