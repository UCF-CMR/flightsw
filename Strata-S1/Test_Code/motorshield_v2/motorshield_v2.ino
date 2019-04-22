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
#define  MC_A2                 0   // Motor C - A2
#define  MD_A2                 3   // Motor D - A2

// Analog input pins for motor position
#define  POS_A                A0   // Feedback on Motor A position - actually wired to D
#define  POS_B                A1   // Feedback on Motor B position - actually wired to C
#define  POS_C                A2   // Feedback on Motor C position - actually wired to B
#define  POS_D                A3   // Feedback on Motor D position - actually wired to A

// Arrays to define sets of pins for each entrapulator
const char PIN_EN_ARR[] = {ENABLE_A, ENABLE_B, ENABLE_C, ENABLE_D};
const char PIN_A1_ARR[] = {MA_A1,    MB_A1,    MC_A1,    MD_A1   };
const char PIN_A2_ARR[] = {MA_A2,    MB_A2,    MC_A2,    MD_A2   };
const char PIN_PS_ARR[] = {POS_A,    POS_B,    POS_C,    POS_D   };

// Arrays to hold current positions for actuators
int act_pos[]   = {  0,   0,   0,   0};
int act_max[]   = {  0,   0,   0,   0};
int act_maxe[]  = { 75,  75,  75,  75};
int act_mine[]  = {400, 400, 400, 400};
int act_half[]  = {250, 250, 250, 250};
int act_quart[] = {350, 350, 350, 350};

// Flag to see if the experiment has been triggered or not
bool hastriggered = false;

// Variables for timing
int t_ret = 4500;      // leave Entraps retracted for 45s in µg phase 
int t_ext =  3000;      // leave Entraps extended for 3s in µg phase

void setup() {
  // Enable serial port
  Serial.begin(9600);

  // Make pin assignments
  for( char i = 0; i < 4; i++ )
  {
    // Set digital pins as outputs
    pinMode(PIN_EN_ARR[i], OUTPUT);
    pinMode(PIN_A1_ARR[i], OUTPUT);
    pinMode(PIN_A2_ARR[i], OUTPUT);
    /* DO NOT SET MODE FOR ANALOG PINS */

    // Give digital pins default LOW value
    digitalWrite(PIN_EN_ARR[i], LOW);
    digitalWrite(PIN_A1_ARR[i], LOW);
    digitalWrite(PIN_A2_ARR[i], LOW);
  }

  // Make sure all entrapulators are extended
  Serial.println("Checking extension");
  for( char i = 1; i < 4; i++ )
  {
    act_pos[i] = analogRead(PIN_PS_ARR[i]);
    Serial.print(i, DEC);
    Serial.print(" starting at ");
    Serial.println(act_pos[i]);
  }
  for( char i = 1; i < 4; i++ )
  {
    if( act_pos[i] > act_maxe[i] )
    {
      while( act_pos[i] > act_maxe[i] )
      {
        digitalWrite(PIN_A1_ARR[i], HIGH);
        digitalWrite(PIN_EN_ARR[i], HIGH);
        act_pos[i] = analogRead(PIN_PS_ARR[i]);
        Serial.print("Setting ");
        Serial.print(i, DEC);
        Serial.print("to: ");
        Serial.println(act_pos[i]);
      }
      digitalWrite(PIN_EN_ARR[i], LOW);
      digitalWrite(PIN_A1_ARR[i], LOW);

      delay(1000);
    }
  }

  // Read maximum entrapulator positions
  for( char i = 1; i < 4; i++ )
  {
    act_pos[i] = analogRead(PIN_PS_ARR[i]);
    act_max[i] = act_pos[i];
  }

  // Print maximum actuation position values
  for( char i = 0; i < 4; i++ )
  {
    Serial.print("Max Position ");
    Serial.print(i, DEC);
    Serial.print(": ");
    Serial.println(act_max[i]);
  }
  
}

void loop() {

  // Check for and debounce trigger signal
  Serial.println("Waiting for trigger signal ... ");
  if( digitalRead(TRIGGER) == HIGH )
  {
    delay(10);
    if( digitalRead(TRIGGER) == HIGH )
    {
      Serial.println("detected!");
      // Set trigger flag so will close once out of µg
      hastriggered = !hastriggered;     

      // Slight delay
      delay(1000);
      
      // Retract all entrapulators
      for( char i = 0; i < 4; i++ )
      {
        Serial.print("Retracting ");
        Serial.print(i, DEC);
        Serial.print("to ");
        Serial.println(act_mine[i]);

        // Fully retract entrapulator
        digitalWrite(PIN_A2_ARR[i], HIGH);        // set pin and enable high
        digitalWrite(PIN_EN_ARR[i], HIGH);
        while( act_pos[i] < act_mine[i] )
        {
          act_pos[i] = analogRead(PIN_PS_ARR[i]);
          Serial.print(i, DEC);
          Serial.print("_A2 (bwd) ");
          Serial.println(act_pos[i]);
        }
        digitalWrite(PIN_EN_ARR[i], LOW);         // set pin and enable low
        digitalWrite(PIN_A2_ARR[i], LOW);

        delay(1000);    // pause in between them
      }  

      //StrataSphere stays retracted the rest of the time
      
      // leave B-D retracted for retract time
      delay(t_ret);

      // Half-way extend entrapulators B-D
      for( char i = 1; i < 4; i++ )
      {
        Serial.print("Extending halfway out ");
        Serial.print(i, DEC);
        Serial.print("to ");
        Serial.println(act_half[i]);

        digitalWrite(PIN_A1_ARR[i], HIGH);
        digitalWrite(PIN_EN_ARR[i], HIGH);
        while( act_pos[i] > act_half[i] )
        {
          act_pos[i] = analogRead(PIN_PS_ARR[i]);
          Serial.print(i, DEC);
          Serial.print("_A1 (fwd) ");
          Serial.println(act_pos[i]);
        }
        digitalWrite(PIN_EN_ARR[i], LOW);
        digitalWrite(PIN_A1_ARR[i], LOW);

        delay(1000);
      }

      // leave B-D extended for ext. time
      delay(t_ext);

      // Fully retract entrapulators B-D
      for( char i = 1; i < 4; i++ )
      {
        Serial.print("Retracting ");
        Serial.print(i, DEC);
        Serial.print("to ");
        Serial.println(act_mine[i]);

        // Fully retract entrapulator
        digitalWrite(PIN_A2_ARR[i], HIGH);
        digitalWrite(PIN_EN_ARR[i], HIGH);
        while( act_pos[i] < act_mine[i] )
        {
          act_pos[i] = analogRead(PIN_PS_ARR[i]);
          Serial.print(i, DEC);
          Serial.print("_A2 (bwd) ");
          Serial.println(act_pos[i]);
        }
        digitalWrite(PIN_EN_ARR[i], LOW);
        digitalWrite(PIN_A2_ARR[i], LOW);

        delay(1000);    // pause in between them
      }

      // leave retracted for time t_retract
      delay(t_ret);
      
      // Extend entrapulators B-D about 1/4 way out
      for( char i = 1; i < 4; i++ )
      {
        Serial.print("Extending ~1/4 way out ");
        Serial.print(i, DEC);
        Serial.print("to ");
        Serial.println(act_quart[i]);

        digitalWrite(PIN_A1_ARR[i], HIGH);
        digitalWrite(PIN_EN_ARR[i], HIGH);
        while( act_pos[i] > act_quart[i] )
        {
          act_pos[i] = analogRead(PIN_PS_ARR[i]);
          Serial.print(i, DEC);
          Serial.print("_A1 (fwd) ");
          Serial.println(act_pos[i]);
        }
        digitalWrite(PIN_EN_ARR[i], LOW);
        digitalWrite(PIN_A1_ARR[i], LOW);

        delay(1000);
      }

      // leave extended for time t_extend
      delay(t_ext);

      //retract for remaining time
      // Fully retract entrapulators B-D
      for( char i = 1; i < 4; i++ )
      {
        Serial.print("Retracting ");
        Serial.print(i, DEC);
        Serial.print("to ");
        Serial.println(act_mine[i]);

        // Fully retract entrapulator
        digitalWrite(PIN_A2_ARR[i], HIGH);
        digitalWrite(PIN_EN_ARR[i], HIGH);
        while( act_pos[i] < act_mine[i] )
        {
          act_pos[i] = analogRead(PIN_PS_ARR[i]);
          Serial.print(i, DEC);
          Serial.print("_A2 (bwd) ");
          Serial.println(act_pos[i]);
        }
        digitalWrite(PIN_EN_ARR[i], LOW);
        digitalWrite(PIN_A2_ARR[i], LOW);

        delay(1000);    // pause in between them
      }

      delay(t_ret);
    }
  }
  else if (hastriggered == true)
  {  
  // extend entrapulators for landing

    for( char i = 1; i < 4; i++ )
    {
      if( act_pos[i] > act_maxe[i] )
      {
        while( act_pos[i] > act_maxe[i] )
        {
          digitalWrite(PIN_A1_ARR[i], HIGH);
          digitalWrite(PIN_EN_ARR[i], HIGH);
          act_pos[i] = analogRead(PIN_PS_ARR[i]);
          Serial.print("Setting ");
          Serial.print(i, DEC);
          Serial.print("to: ");
          Serial.println(act_pos[i]);
        }
        digitalWrite(PIN_EN_ARR[i], LOW);
        digitalWrite(PIN_A1_ARR[i], LOW);
  
        delay(1000);
      }
     }
   }
   else{
   //nothing
   }
}
