/*  SLOPE Parabolic Flight experiment
 *  Authors: Diego Briceno & Seamus Anderson
 *  Created:     6 July  2017
 *  
 *  Wiring for the motor:
 *    Black: A+
 *    Green: A-
 *    Red:   B+
 *    Blue:  B-
 *  Motor Resolution: 1.8 deg/step 
 *  
 *  Updates: 6 March 2018
 *  25 July 2019: 
 *    updated to clean up code, add comments
 *    change rotation time
 */

 

// Define pin functions
//Driver Pins
#define PUL 13   // pull up?
#define DIR 12   // motor direction
#define ENA 11   // motor enable

//Limit and Vibration Pins
#define LIM  8   // limit switch
#define VIB  7   // vibration motor

//Controller Pins
#define FLIP 4   // main exp. power switch
#define PUSH 3   // increment push button switch


// Global Variables
bool switch_hist = 0;    // History of the switch, 0 if un-switched, 1 if it has been switched


//*************//
// Start Setup //
//*************//
      
void setup(){

  // Beginning Serial Output
  int i = 0;
  Serial.begin(9600);
  
  // Configuring Input/Output pins
  // Stepper Motor Pins
  // initialize all pins as output, set to low - no motion
  pinMode(ENA,   OUTPUT);    digitalWrite(ENA,  LOW);    delay(1); 
  pinMode(DIR,   OUTPUT);    digitalWrite(DIR,  LOW);    delay(1);  // LOW is clockwise
  pinMode(PUL,   OUTPUT);    digitalWrite(PUL,  LOW);    delay(1);

  //Inner box pins
  // initialize vibe motor pin as an output 
  // - set pin to low, toggle to high to turn on the motor
  pinMode(VIB,   OUTPUT);    digitalWrite(VIB,  LOW);    delay(1);
  // initialize limit switch as an input - will look to see if triggered
  pinMode(LIM,    INPUT);
  
  //Controller Pins
  // 
  pinMode(FLIP,   INPUT);    
  pinMode(PUSH,   INPUT); 
 
  delay(100);          
  
  // Power on Routine //
  //------------------//

  digitalWrite(DIR, HIGH);  delay(1);  //Counter clockwise

  while(digitalRead(LIM)!=1){          // While loop rotates counter-clock until limit switch is triggered
      digitalWrite(PUL, HIGH);
      delay(15);
      digitalWrite(PUL, LOW);
      delay(15);    
      }

  digitalWrite(DIR, LOW);  delay(1); //Clockwise
   
  }



     // Subroutines //
     //*************//

// Initial Rotation Subroutine//
//---------------------------------//
/*  Parameters
 *  ----------
 *  int steps:  number of steps to iterate (n_deg = 1.8 * n_steps)
 *  int rpm=10: revolutions per minute of the motor
 */
void SUB_ROTATE(int n_steps, int rpm=10){

  Serial.println("\t # SUB_ROTATE #");
  int       i = 0;
  float dtime = 3000 / (20 * rpm);    
  switch_hist = 1;
  digitalWrite(DIR, LOW);
  delay(1);
  digitalWrite(VIB, HIGH);
       
       // Iterate through steps
  for(i=0; i < n_steps; i++){ 
            // Stop moving if switch is off
       if(digitalRead(FLIP)==0){
            return;             }
       digitalWrite(PUL, HIGH);
       delay(dtime); 
       digitalWrite(PUL, LOW);
       delay(dtime);        }   }



// Subroutine for iterating motor by 5.4 deg //
//-----------------------------------------//
/*  Parameters
 *  ----------
 *  int n_steps =  3: number of steps to move through
 *  int rmp     = 10: revolutions per minute of the motor 
 */
void SUB_ITERATE(int n_steps=3, int rpm=10){

  Serial.println("\t #SUB_ITERATE #");  
  int   i = 0;
  float dtime = 3000 / (20 * rpm);  
  digitalWrite(DIR, LOW);     
  delay(1);
  
  for(i=0; i < n_steps ; i++){          // Iterate 5.4 deg or 3 steps
       digitalWrite(PUL, HIGH);
       delay(dtime);                    // wait half a second
       digitalWrite(PUL, LOW);
       delay(dtime);            }    }



// Reseting to Inital Position Subroutine //
//--------------------------------------------//
/*  Parameters
 *  ----------
 *  int rpm=10:   revolutions per minute
 */

void SUB_RESET(int rpm=10){

  Serial.println("\t # SUB_RESET #");
  float dtime = 3000 / (20 * rpm);

       // Reverse direction of the motor to counter-clockwise
  digitalWrite(DIR, HIGH);
  delay(1);

       // Rotate counter-clockwise until box hits limit switch
  while(digitalRead(LIM)!=1){  
       digitalWrite(PUL, HIGH);
       delay(dtime); 
       digitalWrite(PUL, LOW);
       delay(dtime);           }

       // Reset switch history and direction of motor
  switch_hist = 0;
  digitalWrite(DIR, LOW);     }



// Main Function Loop //
//--------------------//

void loop(){
 
       // Reading Switch and Button Logic
  int button_logic  = digitalRead(PUSH);  // Both of these logics should be '0' or '1'
  int switch_logic  = digitalRead(FLIP);
  int contact_logic = digitalRead(LIM);

       // Printing user inputs
  Serial.print("Button: ");
  Serial.print(button_logic);
  Serial.print("\t Switch: ");
  Serial.print(switch_logic);
  Serial.print("\t History: ");
  Serial.print(switch_hist);
  Serial.print("\t Contact: ");
  Serial.println(contact_logic);
  delay(150);
  

       // Initial Rotation event (x * 1.8 degrees, x steps)
       // if switch is on, but hasn't been turned on in the past
  if((switch_hist==0) && (switch_logic==1)){
       Serial.println("### Initial Rotation ###");
       SUB_ROTATE(45);
       delay(500);       }  // Rotate 25 steps or 45 deg, entered parameter is number of steps (n_steps)   
    
       // Button Iterating event (5.4 degrees, 3 steps)
       // if push button is being pressed
  if(button_logic==1){
       Serial.println("### Iteration Event ###");
       SUB_ITERATE();   
       delay(300);       }

       // Reseting event 
       // if switch is off, but has been turned on in the past
  if((switch_hist==1) && (switch_logic==0)){
       Serial.println("### Resetting ###");
       SUB_RESET();
       delay(500);   
       digitalWrite(VIB, LOW);}
  }
