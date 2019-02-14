//This code is for operating COLLIDE with an analog accelerometer and Raspberry Pi for recording attached
//Writen by Jacob Anthony
//For Single COLLIDE
//February 2019



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


#define OUTPUT_READABLE_ACCELGYRO 


int i;                            // Counter for Door
int j;                            // Temp door variable
int limitFlag = LOW;              // Limit Switch on Door

int t = 0;                        // Counter for accelerometer readings

int button = 0;                   // Button State

bool RUN = true;                  // Can the experiment run

int ZeroSwitch = 1;               // Limit Switch 
int CanZero = 0;                  // Can we count the zero
unsigned long ZeroTimeA = 0;      // Max time between counts
int ZeroCount = 0;                // Count Zeros
int ZeroStart = 150;              // Number of zeros before trigger
int TimeModA = 2000;              // Time Allowable between Counts


void Experiment();                // Create Experiment Function, filled out further down


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
    

  Serial.begin(9600);                               //Begin serial just in case connected to PC
  Serial.println("Starting Monitoring Process");

}



void loop()
{
  delay(100);

  button = digitalRead(PushButton);                       // Read pushbutton state
  ZeroSwitch = digitalRead(AccLim);                       // Read Analog Accelerometer
  Serial.print(ZeroSwitch);

  Serial.print("\t");
  Serial.println(ZeroTimeA);

  if(ZeroSwitch == 1){
    if(CanZero == 0){
      CanZero = 1;
      ZeroTimeA = millis();
      Serial.println("Initialize");
    }
    else{
      if(millis() < (ZeroTimeA + TimeModA)){
        ZeroCount++;
        ZeroTimeA = millis();
        Serial.println("Increment");
      }
      else{
        Serial.println(millis());
        CanZero = 0;
        ZeroCount = 0;
        Serial.println("Reset");
      }
    }
    if(ZeroCount >= ZeroStart && RUN == true){
      Serial.println("Running Experiment!");
      Experiment();
      RUN = false;
      ZeroCount = 0;     
    }
  }
  else if(button==1 && RUN == true){
      Experiment();
      RUN = false;
      Serial.println("Button Pushed");
  }
  else{
    t = 0;                                                // Reset t if accel is not low
    //digitalWrite(DOOR,LOW);                               // Keep the microstep driver off.
    digitalWrite(MW,LOW);                                 // Keep the muscle wire off.
  }
    
    
    
  //delay(6000);                                          //extended time from 30000 to 540000 for temp reading
 
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
    for(i=0; i<6750; i++)                                 // The stepper motor will receive 6500 pulses.
    {                                                     // This range dictates how far the tray door
    //  if (limitFlag == LOW)
      {
        digitalWrite(CP,HIGH);                            // will open, and how long the stepper motor
        delay(2);                                         // will be running. A delay MUST BE USED between
        digitalWrite(CP,LOW);                             // signal pulses, with "2" being the minimum. A
        j=i;                                              // higher number will open the door slower.
      }                                                   
   //   limitFlag = digitalRead(LIMIT);
    }
    
    // Activate the projectile launcher.
    delay(10000);
    digitalWrite(MW,HIGH);                                // Turns on the muscle wire circuit. This is a
    delay(1750);                                          // high current circuit, limited to just under 
    digitalWrite(MW,LOW);                                 // a 500mA draw. It is active for 2.0 seconds
    delay(20000);                                         // and is then deactivated. The 1.0 second delay
//                                                        // after the muscle wire goes LOW is the amount of
//                                                        // time used for data recording.
    
    RUN = false;                                          // Set Flag saying experiment has been run


    digitalWrite(DIR,HIGH);                               // Set stepper motor direction to HIGH (closing).
    i=0;                                                  // Reset variable "i" to zero.                         
    for(i=0; i<j; i++)                                    // Perform the same routine as opening the tray 
    {                                                     // door.
      digitalWrite(CP,HIGH);
      delay(2);
      digitalWrite(CP,LOW);
    }
    //digitalWrite(DOOR, LOW);                              // Turn off the microstep driver
    
    //delay(300000);                                      // Delay to close door
    //insert code to close door
}

