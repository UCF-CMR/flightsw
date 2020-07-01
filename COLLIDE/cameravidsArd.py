#Picam code for COLLIDE experiment
#Last edited 2/21/2020
from picamera import PiCamera
from time import sleep
import os
import RPi.GPIO as GPIO
import datetime
import time

GPIO.setup(26, GPIO.IN, pull_up_down=GPIO.PUD_DOWN)

def buttonEventHandler (pin):
    detectLow = 0

camera = PiCamera()
#camera.start_preview()

def main():
    GPIO.setmode(GPIO.BCM)
   
GPIO.setup(26,GPIO.IN)
#global camera
global detectLow
detectLow = 0
    
Focus = 1
Run = 1
RunTime = 0
CheckTime = 0


camera.resolution = (640, 480)
camera.framerate = 60

#add interrupt to make new vid
#GPIO.add_event_detect(26,GPIO.RISING)
#GPIO.add_event_callback(26,buttonEventHandler)

#camera.start_preview()

while Run > 0:
    camera.start_preview()
    date = datetime.datetime.now().strftime("%Y_%m_%d_%H_%M_%S")
    camera.start_recording("/home/pi/Desktop/Vids/"+"Test" + date + ".h264")
    
    RunTime = time.time()+30
    
    if Focus > 0:
        while Focus > 0:
            detectLow = GPIO.input(26)
            if detectLow == 1:
                Focus = 0
            CheckTime = time.time()
            if CheckTime > RunTime:
                break
    else:
        while Run > 0:
            CheckTime = time.time() 
            if CheckTime > RunTime:
                Run=0
                break
#command = "MP4Box -add /home/pi/Desktop/Vids/"+"Test" + date + ".h264 /home/pi/Desktop/Vids/"+"Test" + date + ".mp4"
#call([command], shell=True)           
            
    #sleep(20)
camera.stop_recording()
camera.stop_preview()    
    #sleep(3)
    
    


#if __name__=="__main__":
#    main()
#os.system('omxplayer /home/pi/Videos/COLLIDE_3dmount.h264')