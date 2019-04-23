#define  TRIGGER              13   // Trigger experiment to start in microgravity phase, stop at the end

// Digital output pins for enabling entrapulator motors
#define  ENABLE_A              9   // Enable pin for Motor A
#define  ENABLE_B              6   // Enable pin for Motor B
#define  ENABLE_C              5   // Enable pin for Motor C
#define  ENABLE_D              1   // Enable pin for Motor D

// Digital output pins for extending entrapulators
#define  MA_A1                10   // Motor A - A1
#define  MB_A1                 7   // Motor B - A1
#define  MC_A1                 4   // Motor C - A1
#define  MD_A1                 2   // Motor D - A1

// Digital output pins for retracting entrapulators
#define  MA_A2                11   // Motor A - A2
#define  MB_A2                 8   // Motor B - A2
#define  MC_A2                12   // Motor C - A2
#define  MD_A2                 3   // Motor D - A2

// Analog input pins for motor position
#define  POS_A                A0   // Feedback on Motor A position - actually wired to D
#define  POS_B                A1   // Feedback on Motor B position - actually wired to C
#define  POS_C                A2   // Feedback on Motor C position - actually wired to B
#define  POS_D                A3   // Feedback on Motor D position - actually wired to A

// Arrays to define sets of pins for each entrapulator
// TODO: fix this
const char PIN_EN_ARR[] = {ENABLE_A, ENABLE_B, ENABLE_C, ENABLE_D};
const char PIN_A1_ARR[] = {MA_A1,    MB_A1,    MC_A1,    MD_A1   };
const char PIN_A2_ARR[] = {MA_A2,    MB_A2,    MC_A2,    MD_A2   };
const char PIN_PS_ARR[] = {POS_A,    POS_B,    POS_C,    POS_D   };

// Arrays to hold current positions for actuators
unsigned int act_pos[]   = {  0,   0,   0,   0};
unsigned int act_max[]   = {  0,   0,   0,   0};
unsigned int act_maxe[]  = { 75,  75,  75,  75};
unsigned int act_mine[]  = {400, 400, 400, 750};
unsigned int act_half[]  = {250, 250, 250, 250};
unsigned int act_quart[] = {350, 350, 350, 350};

// Flag to see if the experiment has been triggered or not
bool hastriggered = false;

// Variables for timing
// **** CHANGE FOR FLIGHT ****
int t_ret = 4500;      // leave Entraps retracted for 45s in µg phase 
int t_ext = 3000;      // leave Entraps extended for 3s in µg phase

// Timeout for extension/retraction in millseconds
// TODO: Decrease this for flight!
unsigned long timeout = 2000;

void printMaximum(char i, unsigned int value)
{
  Serial.print("Maximum ");
  Serial.print(i, DEC);
  Serial.print(": ");
  Serial.println(value);
}

void printRetracting(char i, unsigned int setpoint, unsigned int value)
{
  Serial.print("Retracting ");
  Serial.print(i, DEC);
  Serial.print(" to ");
  Serial.print(setpoint);
  Serial.print(" from ");
  Serial.println(value);
}

void printExtending(char i, unsigned int setpoint, unsigned int value)
{
  Serial.print("Extending ");
  Serial.print(i, DEC);
  Serial.print(" to ");
  Serial.print(setpoint);
  Serial.print(" from ");
  Serial.println(value);
}

void enableExtend(char i, bool enable)
{
  if( enable )
  {
    Serial.print("Enabling");
    digitalWrite(PIN_A1_ARR[i], HIGH);
    digitalWrite(PIN_EN_ARR[i], HIGH);
  }
  else
  {
    Serial.print("Disabling");
    digitalWrite(PIN_EN_ARR[i], LOW);
    digitalWrite(PIN_A1_ARR[i], LOW);
  }
  Serial.print(" extend on ");
  Serial.println(i, DEC);
}

void enableRetract(char i, bool enable)
{
  if( enable )
  {
    Serial.print("Enabling");
    digitalWrite(PIN_A2_ARR[i], HIGH);
    digitalWrite(PIN_EN_ARR[i], HIGH);
  }
  else
  {
    Serial.print("Disabling");
    digitalWrite(PIN_EN_ARR[i], LOW);
    digitalWrite(PIN_A2_ARR[i], LOW);
  }
  Serial.print(" retract on ");
  Serial.println(i, DEC);
}

// Retract entrapulator to setpoint
void retractEntrap(char i, unsigned int setpoint)
{
  // Read current position and print to serial monitor
  act_pos[i] = analogRead(PIN_PS_ARR[i]);
  printRetracting(i, setpoint, act_pos[i]);
  // Only retract if not past setpoint
  if( act_pos[i] < setpoint )
  {
    // Record start time
    unsigned long start = millis();
    // Enable retraction pins
    enableRetract(i, true);
    // Continue retracting while setpoint has not been reached
    while( act_pos[i] < setpoint )
    {
      // Exit loop if timeout expires
      if( millis() - start > timeout )
      {
        Serial.println("Timeout expired. Aborting!");
        break;
      }
      act_pos[i] = analogRead(PIN_PS_ARR[i]);
      printRetracting(i, setpoint, act_pos[i]);
    }
    // Disable retraction pins
    enableRetract(i, false);
  }
  else
  {
    Serial.println("Not retracting. Already past setpoint!");
  }
}

// Extend entrapulator to setpoint
void extendEntrap(char i, unsigned int setpoint)
{
  // Read current position and print to serial monitor
  act_pos[i] = analogRead(PIN_PS_ARR[i]);
  printExtending(i, setpoint, act_pos[i]);
  // Only extend if not past setpoint
  if( act_pos[i] > setpoint )
  {
    // Record start time
    unsigned long start = millis();
    // Enable extension pins
    enableExtend(i, true);
    // Continue extending while setpoint has not been reached
    while( act_pos[i] > setpoint )
    {
      // Exit loop if timeout expires
      if( millis() - start > timeout )
      {
        Serial.println("Timeout expired. Aborting!");
        break;
      }
      act_pos[i] = analogRead(PIN_PS_ARR[i]);
      printExtending(i, setpoint, act_pos[i]);
    }
    // Disable extension pins
    enableExtend(i, false);
  }
  else
  {
    Serial.println("Not extending. Already past setpoint!");
  }
}

void setup() {

  // Enable serial port
  Serial.begin(9600);
  Serial.println("########## INITIAL SETUP ##########");

  // Set trigger pin as input
  pinMode(TRIGGER, INPUT);

  // Make pin assignments
  for( char i = 0; i < 4; i++ )
  {
    // Set digital pins as outputs
    pinMode(PIN_EN_ARR[i], OUTPUT);
    pinMode(PIN_A1_ARR[i], OUTPUT);
    pinMode(PIN_A2_ARR[i], OUTPUT);

    // Set analog pins as inputs
    pinMode(PIN_PS_ARR[i], INPUT);

    // Give digital pins default LOW value
    digitalWrite(PIN_EN_ARR[i], LOW);
    digitalWrite(PIN_A1_ARR[i], LOW);
    digitalWrite(PIN_A2_ARR[i], LOW);
  }

  // Make sure all entrapulators are extended
  Serial.println("######### FULL EXTEND TEST ########");
  for( char i = 1; i < 4; i++ )
  {
    extendEntrap(i, act_maxe[i]);
    delay(1000);
  }

  // Read and print maximum entrapulator positions
  Serial.println("######## READ MAXIMUM TEST ########");
  for( char i = 0; i < 4; i++ )
  {
    act_max[i] = analogRead(PIN_PS_ARR[i]);
    printMaximum(i, act_max[i]);
  }

  Serial.println("########## SETUP COMPLETE #########");
  
}

void loop() {

  // Check for and debounce trigger signal
  Serial.print("Waiting for trigger signal ... ");
  if( digitalRead(TRIGGER) == HIGH && !hastriggered)
  {
    delay(10);
    if( digitalRead(TRIGGER) == HIGH )
    {
      // Set trigger flag so will close once out of µg
      Serial.println("detected!");
      hastriggered = true;

      // Slight delay
      delay(1000);
      
      // Fully retract all entrapulators
      Serial.println("######## FULL RETRACT TEST ########");
      for( char i = 0; i < 4; i++ )
      {
        retractEntrap(i, act_mine[i]);
        delay(1000);
      }  

      //StrataSphere stays retracted the rest of the time
      
      // leave B-D retracted for retract time
      Serial.print("Waiting for retract time to elapse ... ");
      delay(t_ret);
      Serial.println("done!");

      // Half-way extend entrapulators B-D
      Serial.println("######### HALF EXTEND TEST ########");
      for( char i = 0; i < 3; i++ )
      {
        extendEntrap(i, act_half[i]);
        delay(1000);
      }

      // leave B-D extended for ext. time
      Serial.print("Waiting for extend time to elapse ... ");
      delay(t_ext);
      Serial.println("done!");
      
      // Fully retract entrapulators B-D
      Serial.println("######## FULL RETRACT TEST ########");
      for( char i = 0; i < 3; i++ )
      {
        retractEntrap(i, act_mine[i]);
        delay(1000);
      }

      // leave retracted for time t_retract
      Serial.print("Waiting for retract time to elapse ... ");
      delay(t_ret);
      Serial.println("done!");
            
      // Extend entrapulators B-D about 1/4 way out
      Serial.println("####### QUARTER EXTEND TEST #######");
      for( char i = 0; i < 3; i++ )
      {
        extendEntrap(i, act_quart[i]);
        delay(1000);
      }

      // leave extended for time t_extend
      Serial.print("Waiting for extend time to elapse ... ");
      delay(t_ext);
      Serial.println("done!");

      // Fully retract entrapulators B-D
      Serial.println("######## FULL RETRACT TEST ########");
      for( char i = 1; i < 4; i++ )
      {
        retractEntrap(i, act_mine[i]);
        delay(1000);
      }
      
      // leave retracted for time t_retract
      Serial.print("Waiting for retract time to elapse ... ");
      delay(t_ret);
      Serial.println("done!");

      // Fully extend entrapulators for landing
      Serial.println("######### FULL EXTEND TEST ########");
      for( char i = 0; i < 4; i++ )
      {
        extendEntrap(i, act_maxe[i]);
        delay(1000);
      }

      Serial.println("####### EXPERIMENT COMPLETE ######");
      delay(1000);

    }
  }
  else if (hastriggered)
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
