////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
//     Linear actuator/Entrapulator controls for Strata-S1 experiment
//
//     University of Central Florida
//     Center for Microgravity Research 
//  
//     Code written by: Addie Dove, James Phillips
//   
//     
////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
//
//  This code is for use with the Strata-S1 experiment for ground-based testing.
//
//  It is written for the Arduino UNO R3 microcontroller utilizing the ATMEL 
//  ATMEGA328 chipset. Details for this board can be found at "www.arduino.cc".
//
////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////

#define  TRIGGER              13   // Trigger experiment to start
//#define RESETPIN                  // Use if using reset button to reset instead of trigger

// Define pins related to the linear actuator
#define  EN_ENTRAP              1   // Digital output pins for enabling entrapulator linear actuator
#define  EXT_ENTRAP             2   // Digital output pins for extending entrapulator
#define  RET_ENTRAP             3   // Digital output pins for retracting entrapulators
#define  POS_LA                A3   // Feedback on actuator position

// Set positions for actuators
unsigned int act_pos   = 0;
unsigned int act_max   = 0;
unsigned int act_maxe  = 75;
unsigned int act_mine  = 400;
unsigned int act_half  = 250;
unsigned int act_quart = 350;

// Flag to see if the experiment has been triggered or not
bool hastriggered = false;
bool expcomplete  = false;

// Variables for timing
long t_ret = 45000;      // leave Entraps retracted for 45s 
long t_ext =  5000;      // leave Entraps extended  for  5s

// Timeout for extension/retraction in millseconds
unsigned long timeout = 900;

void printMaximum(unsigned int value)
{
  Serial.print("Maximum: ");
  Serial.println(value);
}

void printExtending(unsigned int setpoint, unsigned int value)
{
  Serial.print("Extending ");
  Serial.print(" to ");
  Serial.print(setpoint);
  Serial.print(" from ");
  Serial.println(value);
}

void printRetracting(unsigned int setpoint, unsigned int value)
{
  Serial.print("Retracting ");
  Serial.print(" to ");
  Serial.print(setpoint);
  Serial.print(" from ");
  Serial.println(value);
}

void enableExtend(bool enable)
{
  if( enable )
  {
    Serial.print("Enabling");
    digitalWrite(EXT_ENTRAP, HIGH);
    digitalWrite(EN_ENTRAP, HIGH);
  }
  else
  {
    Serial.print("Disabling");
    digitalWrite(EN_ENTRAP, LOW);
    digitalWrite(EXT_ENTRAP, LOW);
  }
  Serial.println(" extending ");
}

void enableRetract(bool enable)
{
  if( enable )
  {
    Serial.print("Enabling");
    digitalWrite(RET_ENTRAP, HIGH);
    digitalWrite(EN_ENTRAP, HIGH);
  }
  else
  {
    Serial.print("Disabling");
    digitalWrite(EN_ENTRAP, LOW);
    digitalWrite(RET_ENTRAP, LOW);
  }
  Serial.println(" retracting ");
}


// Extend entrapulator to setpoint
void extendEntrap(unsigned int setpoint)
{
  // Read current position and print to serial monitor
  act_pos = analogRead(POS_LA);
  printExtending(setpoint, act_pos);
  // Only extend if not past setpoint
  if( act_pos > setpoint )
  {
    // Record start time
    unsigned long start = millis();
    // Enable extension pins
    enableExtend(true);
    // Continue extending while setpoint has not been reached
    while( act_pos > setpoint )
    {
      // Exit loop if timeout expires
      if( millis() - start > timeout )
      {
        Serial.println("Timeout expired. Aborting!");
        break;
      }
      act_pos = analogRead(POS_LA);
      printExtending(setpoint, act_pos);
    }
    // Disable extension pins
    enableExtend(false);
  }
  else
  {
    Serial.println("Not extending. Already past setpoint!");
  }
}


// Retract entrapulator to setpoint
void retractEntrap(unsigned int setpoint)
{
  // Read current position and print to serial monitor
  act_pos = analogRead(POS_LA);
  printRetracting(setpoint, act_pos);
  // Only retract if not past setpoint
  if( act_pos < setpoint )
  {
    // Record start time
    unsigned long start = millis();
    // Enable retraction pins
    enableRetract(true);
    // Continue retracting while setpoint has not been reached
    while( act_pos < setpoint )
    {
      // Exit loop if timeout expires
      if( millis() - start > timeout )
      {
        Serial.println("Timeout expired. Aborting!");
        break;
      }
      act_pos = analogRead(POS_LA);
      printRetracting(setpoint, act_pos);
    }
    // Disable retraction pins
    enableRetract(false);
  }
  else
  {
    Serial.println("Not retracting. Already past setpoint!");
  }
}



/////***************** SETUP *****************/////
void setup() {

  // Enable serial port
  Serial.begin(9600);
  Serial.println("########## INITIAL SETUP ##########");

  // Set trigger pin as input
  pinMode(TRIGGER, INPUT);

  // Make pin assignments
  // Set digital pins as outputs
    pinMode(EN_ENTRAP, OUTPUT);
    pinMode(EXT_ENTRAP, OUTPUT);
    pinMode(RET_ENTRAP, OUTPUT);

    // Set analog pins as inputs
    pinMode(POS_LA, INPUT);

    // Give digital pins default LOW value
    digitalWrite(EN_ENTRAP, LOW);
    digitalWrite(EXT_ENTRAP, LOW);
    digitalWrite(RET_ENTRAP, LOW);
 

  // Make sure all entrapulators are extended
  Serial.println("######### FULL EXTEND TEST ########");
    extendEntrap(act_maxe);
    delay(1000);
 

  // Read and print maximum entrapulator positions
  Serial.println("######## READ MAXIMUM TEST ########");
    act_max = analogRead(POS_LA);
    printMaximum(act_max);
 
  Serial.println("########## SETUP COMPLETE #########");

// delay so that doesn't go into main loop when IPC is first powered up. 
// TODO: add delay back in
//  delay(55000);

}

/////***************** MAIN LOOP *****************/////
void loop() {
// For single line testing
  // Check for and debounce trigger signal
  Serial.print("Waiting for trigger signal ... ");
  if( digitalRead(TRIGGER) == HIGH && !hastriggered)
  {
    delay(10);
    if( digitalRead(TRIGGER) == HIGH )
    {
      // Set trigger flag so will close once out of µg
      Serial.println("detected!");

// TODO: uncomment for flight!
      hastriggered = true;

      // Slight delay
      delay(1000);
      
      // Fully retract entrapulators
      Serial.println("######## FULL RETRACT ########");
      retractEntrap(act_mine);
      delay(1000);
    

      Serial.print("Waiting for retract time to elapse ... ");
      delay(t_ret);
      Serial.println("done!");

      // Half-way extend entrapulator
      Serial.println("######### HALF EXTEND ########");
      extendEntrap(act_half);
      delay(1000);


      // leave extended for ext. time
      Serial.print("Waiting for extend time to elapse ... ");
      delay(t_ext);
      Serial.println("done!");
      
      // Fully retract entrapulator
      Serial.println("######## FULL RETRACT ########");
      retractEntrap(act_mine);
      delay(1000);


      // leave retracted for time t_retract
      Serial.print("Waiting for retract time to elapse ... ");
      delay(t_ret);
      Serial.println("done!");


      // Fully extend entrapulators
      Serial.println("######### FULL EXTEND ########");
      extendEntrap(act_maxe);
      delay(1000);

      Serial.println("####### EXPERIMENT COMPLETE ######");
      delay(1000);

    }
  }
  else if (hastriggered && !expcomplete)
  {
    // Indicate experiment has already been run and will not retrigger
    Serial.println("experiment complete!");
    delay(1000);
    expcomplete = true;
  }
  else if (hastriggered && expcomplete)
  {
    // Indicate experiment has already been run and will not retrigger
    Serial.println("experiment complete!");
    delay(1000);
  }
  else
  {
    // No trigger detected so print newline
    Serial.println();
    // Delay if no trigger detected
    // TODO: Decrease for flight
    delay(100);
  }
}
