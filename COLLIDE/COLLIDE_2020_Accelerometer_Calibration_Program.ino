// This code is for calibrating the MPU 6050 accelerometeter installed on COLLIDE.
// Written July 2020 by Richard Wakefield.

// PROCEDURE / HOW TO USE THIS PROGRAM:
// 1) Place COLLIDE next to your PC on a **LEVEL** tabletop.
// 2) Ensure that the experiment is NOT connected to external power.  It is not necessary to unplug any connections between the experiment and the Arduino.
// 3) Connect the COLLIDE Arduino to your PC via USB cable.
// 4) Upload this program to the Arduino.
// 5) Under the "Tools" tab, click "Serial Monitor."
// 6) After this point, do not touch or move the experiment.
// 7) Wait for the program to state that it is finished on the serial monitor.
// 8) Write down the 3 ouput values (x,y,z).
// 9) In the COLLIDE2020_V2.0.ino flight code file, enter the output values into their correct places: the x value into line 152, y into 153, and z into 154.
// 10) Acclerometer is now fully calibrated, and the flight code has been updated with the correct calibration values.
// 11) Close the serial monitor window.
// 12) Unplug the Arduino from your PC.


#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h"

// Arduino Wire library is required if I2Cdev I2CDEV_ARDUINO_WIRE implementation
// is used in I2Cdev.h
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
#include "Wire.h"
#endif

// Defines the number of accelerometer data points per packet
// A packet size of 75 corresponds to a sampling window of approximately 50ms
#define PACKET_SIZE 75

// Declares a new MPU6050 object named "accelerometer"
MPU6050 accelerometer;

// This is where "getmotion6" places its outputs.  These represent RAW readings from accelerometer and gyroscope.  Gyroscope is currently unused.
int16_t ax, ay, az;
int16_t gx, gy, gz;

// Arrays and ints for calculating averages in each packet
int packetData[(PACKET_SIZE * 3)];
float averageAccel[3];
float axAccum = 0;
float ayAccum = 0;
float azAccum = 0;

// Freefall threshold; max total acceleration value (in g's) that can be considered freefall
float freefallThreshold = 0.35;

// Bools that say, for each packet interval, if each acceleration component is under the threshold
bool isInFreefallXInterval = false;
bool isInFreefallYInterval = false;
bool isInFreefallZInterval = false;

bool calibrationDone = false;
bool gettingClose = false;

int iter = -1;
int closeEnoughCount = 0;

int xOffset = 0;
int yOffset = 0;
int zOffset = 0;


void setup() {

  // Begin setup of the accelerometer I2C communications

  // join I2C bus (I2Cdev library doesn't do this automatically)
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
  Wire.begin();
  Wire.setClock(400000); // 400kHz I2C clock. Comment this line if having compilation difficulties
#elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
  Fastwire::setup(400, true);
#endif

  // Initialize serial communication at 115200 baud
  Serial.begin(115200);

  // initialize device
  Serial.println(F("Initializing I2C devices..."));
  accelerometer.initialize();

  // verify connection
  Serial.println(F("Testing device connections..."));
  Serial.println(accelerometer.testConnection() ? F("MPU6050 connection successful") : F("MPU6050 connection failed"));

  // End setup of accelerometer I2C

  accelerometer.setFullScaleAccelRange(MPU6050_ACCEL_FS_4);

  accelerometer.setXGyroOffset(0);
  accelerometer.setYGyroOffset(0);
  accelerometer.setZGyroOffset(0);
  accelerometer.setXAccelOffset(0);
  accelerometer.setYAccelOffset(0);
  accelerometer.setZAccelOffset(0);

}

void loop() {

while (!calibrationDone)
{
  // *BEGIN COARSE CALIBRATION*
  
  GetPacket();

  iter++;
  
  Serial.print("Number of iterations completed: ");
  Serial.print(iter);
  Serial.print("\n");
  Serial.println(averageAccel[0], 6);
  Serial.println(averageAccel[1], 6);
  Serial.println(averageAccel[2], 6);
  Serial.println("");

  if(averageAccel[0] < 0)
  {
    xOffset = xOffset + 5;
    accelerometer.setXAccelOffset(xOffset);
  }
  else if(averageAccel[0] > 0)
  {
    xOffset = xOffset - 5;
    accelerometer.setXAccelOffset(xOffset);
  }
  else if(averageAccel[0] == 0)
  {
    // do nothing
  }

  if(averageAccel[1] < 0)
  {
    yOffset = yOffset + 5;
    accelerometer.setYAccelOffset(yOffset);
  }
  else if(averageAccel[1] > 0)
  {
    yOffset = yOffset - 5;
    accelerometer.setYAccelOffset(yOffset);
  }
  else if(averageAccel[1] == 0)
  {
    // do nothing
  }

  if(averageAccel[2] < 1)
  {
    zOffset = zOffset + 5;
    accelerometer.setZAccelOffset(zOffset);
  }
  else if(averageAccel[2] > 1)
  {
    zOffset = zOffset - 5;
    accelerometer.setZAccelOffset(zOffset);
  }
  else if(averageAccel[2] == 1)
  {
    // do nothing
  }

  if((abs(averageAccel[0]) < .005) && (abs(averageAccel[1]) < .005) && (abs(averageAccel[2] - 1) < .005))
  {
    gettingClose = true;
  }

  // *BEGIN FINE CALIBRATION*

  while(gettingClose)
  {
    GetPacket();

  iter++;
  
  Serial.print("Number of iterations completed: ");
  Serial.print(iter);
  Serial.print("\n");
  Serial.println(averageAccel[0], 6);
  Serial.println(averageAccel[1], 6);
  Serial.println(averageAccel[2], 6);
  Serial.println("");
    
    
    if(averageAccel[0] < 0)
  {
    xOffset = xOffset + 1;
    accelerometer.setXAccelOffset(xOffset);
  }
  else if(averageAccel[0] > 0)
  {
    xOffset = xOffset - 1;
    accelerometer.setXAccelOffset(xOffset);
  }
  else if(averageAccel[0] == 0)
  {
    // do nothing
  }

  if(averageAccel[1] < 0)
  {
    yOffset = yOffset + 1;
    accelerometer.setYAccelOffset(yOffset);
  }
  else if(averageAccel[1] > 0)
  {
    yOffset = yOffset - 1;
    accelerometer.setYAccelOffset(yOffset);
  }
  else if(averageAccel[1] == 0)
  {
    // do nothing
  }

  if(averageAccel[2] < 1)
  {
    zOffset = zOffset + 1;
    accelerometer.setZAccelOffset(zOffset);
  }
  else if(averageAccel[2] > 1)
  {
    zOffset = zOffset - 1;
    accelerometer.setZAccelOffset(zOffset);
  }
  else if(averageAccel[2] == 1)
  {
    // do nothing
  }

  if((abs(averageAccel[0]) < .0015) && (abs(averageAccel[1]) < .0015) && (abs(averageAccel[2] - 1) < .0015))
  {
    closeEnoughCount++;
    Serial.println(closeEnoughCount);
    Serial.println("");
  }
  else
  {
    closeEnoughCount = closeEnoughCount - 1;
  }

  if(closeEnoughCount < 0)
  {
    closeEnoughCount = 0;
  }

  if(closeEnoughCount > 19)
  {
    gettingClose = false;
    calibrationDone = true;
  }

  
}

}
  Serial.println("*** CALIBRATION COMPLETE ***");
  Serial.print("FINAL X ACCEL OFFSET VALUE = ");
  Serial.print(xOffset);
  Serial.print("\n");
  Serial.print("FINAL Y ACCEL OFFSET VALUE = ");
  Serial.print(yOffset);
  Serial.print("\n");
  Serial.print("FINAL Z ACCEL OFFSET VALUE = ");
  Serial.print(zOffset);
  Serial.print("\n");

  delay(1000000);
}


// Places the packet code into its own function
void GetPacket()
{
  // Resets these values for running through the loop again
  axAccum = 0;
  ayAccum = 0;
  azAccum = 0;

  // Loads the packetData array with PACKET_SIZE sets of ax, ay, and az values
  for (int i = 0; i < PACKET_SIZE; i++)
  {
    accelerometer.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
    packetData[(i * 3)] = ax;
    packetData[(i * 3) + 1] = ay;
    packetData[(i * 3) + 2] = az;
  }

  // Averages the data points into a single vector averageAccel
  for (int i = 0; i < PACKET_SIZE; i++)
  {
    axAccum = axAccum + packetData[(i * 3)];
    ayAccum = ayAccum + packetData[(i * 3) + 1];
    azAccum = azAccum + packetData[(i * 3) + 2];
  }
  averageAccel[0] = axAccum / PACKET_SIZE;
  averageAccel[1] = ayAccum / PACKET_SIZE;
  averageAccel[2] = azAccum / PACKET_SIZE;

  // Converts raw into g's.  See accelerometer range setting for divisor.
  // +/- 2g = 16384
  // +/- 4g = 8192
  // +/- 8g = 4096
  // +/- 16g = 2048
  averageAccel[0] = averageAccel[0] / 8192;
  averageAccel[1] = averageAccel[1] / 8192;
  averageAccel[2] = averageAccel[2] / 8192;

  //Compares average acceleration for this packet to the freefall threshold and returns 3 bools
  if (abs(averageAccel[0]) < freefallThreshold)
  {
    isInFreefallXInterval = true;
  }
  else
  {
    isInFreefallXInterval = false;
  }

  if (abs(averageAccel[1]) < freefallThreshold)
  {
    isInFreefallYInterval = true;
  }
  else
  {
    isInFreefallYInterval = false;
  }

  if (abs(averageAccel[2]) < freefallThreshold)
  {
    isInFreefallZInterval = true;
  }
  else
  {
    isInFreefallZInterval = false;
  }

  // ***END OF GetPacket***
}
