//This code is for operating single COLLIDE with an MPU 6050 accelerometer module for freefall detection and a Raspberry Pi (w/ integrated PiCam) for recording
//Written by Richard Wakefield
//January 2020

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

//#define  "X"                 SCL  // Not used.
//#define  "X"                 SDA  // Not used.
//#define  "X"                 AREF // Not used.
//#define  "X"                 GND  // Not used.
#define    AccLim              12   // Analog accelerometer Read
//#define    PiTrigger         11   // Out to Pi
#define    PushButton          10   // PushButton Detection                                     
#define    PushSource          9    // +V for PushButton Detection                                      
#define    LED                 8    // Output for LED's   
                    
#define    LIMIT               7    // Limit Switch
#define    MW                  6    // Output for muscle wire                          
#define    DOOR                5    // Output for microstep power                
#define    DIR                 4    // Output for stepper direction
#define    CP                  3    // Output for stepper signal    
#define    CAM                 2    // Output for camera trigger *disabled*   
//#define  "X"                 1    // Not used.
//#define  "X"                 0    // Not used.

//#define  SCL                 A5   // Not used.                                
//#define  SDA                 A4   // Not used.                                
//#define  "X"                 A3   // Not used.
//#define  "X"                 A2   // Not used.
//#define  "X"                 A1   // Not used.

//  O                          V(in)
//  O                          GND
//  O                          GND
//  O                          5V
//  X                          3.3V
//  X                          RESET
//  X                          IOREF
//  X     

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

// Counts the number of times that GetPacket has returned saying that freefall conditions are met
int runCounter = 0;

// Threshold that runCounter must reach to trigger the experiment.  This corresponds to the amount of time required in freefall to trigger the experiment.
int runThreshold = 300;

//When runExperiment turns to true, the program exits the trigger loop and runs the experiment
bool runExperiment = false;

int i;                            // Counter for Door
int j;                            // Temp door variable
int limitFlag = LOW;              // Limit Switch on Door

int button = 0;                   // Button State

bool RUN = true;                  // When the experiment runs for the first time, this turns false.  When false, this prevents the experiment from running again erroneously

void Experiment();                // Create Experiment Function, filled out further down
void GetPacket();

void setup()
{ 
  pinMode(LED,         OUTPUT);   // Set LED pin as output. 
  //pinMode(CAM,       OUTPUT);   // Set camera trigger pin as output.
  pinMode(MW,          OUTPUT);   // Set muscle wire pin as output.
  pinMode(DOOR,        OUTPUT);   // Set microstep driver pin as output.
  pinMode(CP,          OUTPUT);   // Set stepper motor signal pin as output.
  pinMode(DIR,         OUTPUT);   // Set stepper motor direction pin as output.
  pinMode(LIMIT,       INPUT);    // Set the limit switch on the door as input
  pinMode(PushButton,  INPUT);    // Pin detection for pushbutton
  //pinMode(PiTrigger, OUTPUT);   // Output signal for R-Pi
  pinMode(PushSource,  OUTPUT);   // +V for pushbutton
  pinMode(AccLim,      INPUT);    // Accelerometer Limit Switch

  digitalWrite(LED,         HIGH);    // Turn on LEDs
  //digitalWrite(CAM,       LOW);     // CAM is disabled
  digitalWrite(MW,          LOW);     // Keep muscle wire off
  digitalWrite(DOOR,        LOW);     // Disable door
  digitalWrite(CP,          LOW);     //
  digitalWrite(DIR,         LOW);     //
  //digitalWrite(PiTrigger, LOW);     // Start with Pi Trigger Low
  digitalWrite(PushSource,  HIGH);    // Drive source for PushButton High

  delay(200);
  digitalWrite(DOOR, HIGH);
  delay(1000);
  digitalWrite(DOOR, LOW);

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

  // Set custom offsets to manually calibrate the accelerometer.
  // When sitting still on a level surface and correctly calibrated,
  // the accelerometer/gyro should output all 0's for  the g values, 0 for x and y accel, and +1g (scaled) for z accel.
  // **CURRENTLY CALIBRATED FOR +/- 4g RANGE**
  accelerometer.setXGyroOffset(0);
  accelerometer.setYGyroOffset(270);
  accelerometer.setZGyroOffset(0);
  accelerometer.setXAccelOffset(-2132);
  accelerometer.setYAccelOffset(780);
  accelerometer.setZAccelOffset(1462);

  // Sets the accelerometer sensitivity.
  // MPU6050_ACCEL_FS_2 sets range to +/- 2g
  // MPU6050_ACCEL_FS_4 sets range to +/- 4g
  // MPU6050_ACCEL_FS_8 sets range to +/- 8g
  // MPU6050_ACCEL_FS_16 sets range to +/- 16g
  // Raw acceleration values are always returned as an integer between -32768 and +32768, scaled to the set range.
  // Example: When set to +/- 2g, a normal 1g from gravity will output as 16384.  If set to +/- 4g, 1g gravity will output as 8192, etc.
  accelerometer.setFullScaleAccelRange(MPU6050_ACCEL_FS_4);

}



void loop()
{
  
while (!runExperiment)
  {
    //digitalWrite(DOOR,LOW);                               // Keep the microstep driver off.
    digitalWrite(MW,LOW);                                 // Keep the muscle wire off.
    
    // Gets PushButton state
    button = digitalRead(PushButton);
    
    // Gets packet of accelerometer readings and returns bools that say if accelerations are below the microgravity detection threshold
    GetPacket();

    // Checks bools returned by GetPacket()
    if (isInFreefallXInterval && isInFreefallYInterval && isInFreefallZInterval)
    {
      // If freefall was detected from that packet, increment runCounter up by 1
      runCounter++;
    }
    else
    {
      // If freefall conditions were NOT detected from that packet, subtract 10 from runCounter, down to a minimum of 0
      runCounter = runCounter - 10;
      if(runCounter < 0)
      {
        runCounter = 0;
      }
    }

    // Once runCounter exceeeds runThreshold, runExperiment turns true and the experiment begins
    if (runCounter >= runThreshold && RUN == true)
    {
      runExperiment = true;
    }
    //Runs the experiment when the button is pressed
    else if(button == 1 && RUN == true)
    {
      runExperiment = true;
      Serial.println("Button Pushed");
    }

    Serial.println(averageAccel[0], 4);
    Serial.println(averageAccel[1], 4);
    Serial.println(averageAccel[2], 4);
    Serial.println(isInFreefallXInterval);
    Serial.println(isInFreefallYInterval);
    Serial.println(isInFreefallZInterval);\
    Serial.print("RUN COUNTER: ");
    Serial.print(runCounter);
    Serial.println("");
    Serial.println(millis());
    Serial.println("");
  }

Serial.println("RUNNING EXPERIMENT");
Experiment();

}

void Experiment()
{
    delay(1000);                                            // Wait 1.0 seconds.

    //digitalWrite(DOOR,HIGH);                              // Turn on the microstep driver.
    delay(1000);                                          // Wait 1.0 seconds.
    digitalWrite(DIR, LOW);                               // Set stepper motor direction to LOW (opening).
    delay(1000);                                          // Wait 1.0 seconds.
    
     // Open the tray door.
    i=0;
    for(i=0; i<7000; i++)                                 // The stepper motor will receive up to 7000 pulses (~6750 needed for full door open in testing)
    {                                                     // This range dictates how far the tray door opens
      if (limitFlag == LOW)
      {
        digitalWrite(CP,HIGH);                            // will open, and how long the stepper motor
        delay(2);                                         // will be running. A delay MUST BE USED between
        digitalWrite(CP,LOW);                             // signal pulses, with "2" being the minimum. A
        j=i;                                              // higher number will open the door slower.
      }                                                   
      limitFlag = digitalRead(LIMIT);
    }
    
    // Activate the projectile launcher.
    delay(2000);
    digitalWrite(MW,HIGH);                                // Turns on the muscle wire circuit. This is a
    delay(1000);                                          // high current circuit, limited to just under 
    digitalWrite(MW,LOW);                                 // a 500mA draw. It is active for 1.00 seconds
    delay(103000);                                         // and is then deactivated. The 103.0 second delay
//                                                        // after the muscle wire goes LOW is the amount of
//                                                        // time used for data recording before the door closes again.
    
    RUN = false;                                          // Set Flag saying experiment has been run
    runExperiment = false;


    digitalWrite(DIR,HIGH);                               // Set stepper motor direction to HIGH (closing).
    i=0;                                                  // Reset variable "i" to zero.                         
    for(i=0; i<(j); i++)                                    // Perform the same routine as opening the tray 
    {                                                     // door.
      digitalWrite(CP,HIGH);
      delay(2);
      digitalWrite(CP,LOW);
    }
    //digitalWrite(DOOR, LOW);                              // Turn off the microstep driver
    
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
  averageAccel[2] = (averageAccel[2] / 8192);

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
