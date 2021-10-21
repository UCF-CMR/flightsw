//Door Testing for Dummies

//Define variables - DO NOT TOUCH //
#define    DIR                 5                                    // Output for stepper direction
#define    CP                  4                                    // Output for stepper signal    
#define    TRIGGER             3                                    // Trigger experiment (not using right now)
#define     STEP_EN            A1                                   // Stepper motor enable 
#define     REG_12V            A2                                   // 12V regulator enable (lights will turn on)
int i = 0; 
int x = 0;
//*** THIS SETS THE NUMBER OF STEPS TO OPEN/CLOSE - change this value ***//
int steps=6750;                    

//Define pins - DO NOT TOUCH //
void setup(){
  pinMode(STEP_EN,  OUTPUT);   digitalWrite(STEP_EN,  LOW);           // 5V regulator ON/OFF control pin (for opening tray door) 
  pinMode(REG_12V,   OUTPUT);  digitalWrite(REG_12V,   HIGH);         // Lights should turn on (EM also has power)
  pinMode(CP,    OUTPUT);  digitalWrite(CP,    LOW);                  // Microstepper Pulse pin          (for opening tray door)
  pinMode(DIR,   OUTPUT);  digitalWrite(DIR,   LOW);                  // Microstepper Direction pin      (for opening tray door)  
  delay(2000);
  Serial.begin(115200);                                                       
}


///Main Loop Begins Here///

void loop()
{  

//flash lights to signal experiment start - DO NOT TOUCH //
//digitalWrite(LED,LOW);                                              // Flash the LED's to signal the end of the experiment 
//delay(1500);
//digitalWrite(LED,HIGH);                                             // LEDs will turn off when power is shut off

// Prepare stepper to open tray door - DO NOT TOUCH //
digitalWrite(STEP_EN, HIGH);                                                
delay(3000);                                                                               



//  Open the tray door  //
//((If following instructions at the bottom, start highlighting here))//
digitalWrite(DIR, HIGH);
Serial.print("opening, steps= ");
Serial.println(steps);
for(i=0; i<steps; i++){               
    digitalWrite(CP, HIGH);
    delay(5);
    digitalWrite(CP, LOW);     }
//((Stop highlighting here))//

delay(5000); // stay open 5 seconds


//  Close the tray door  //
//((If following instructions at the bottom, start highlighting here))//                 
digitalWrite(DIR, LOW);
Serial.print("closing, steps= ");
Serial.println(steps);
// *TO DO** EDIT the i number for full opening - 6750
for(i=0; i<steps; i++){               
    digitalWrite(CP, HIGH);
    delay(5);
    digitalWrite(CP, LOW); }
//((Stop highlighting here))//


delay(5000); // wait 5 seconds before shutting off

//  Turn off the system components - DO NOT TOUCH //  
digitalWrite(STEP_EN,LOW);                                            // Turn off the microstep driver.
//digitalWrite(LED,LOW);                                              // Flash the LED's to signal the end of the experiment 
delay(1500);
//digitalWrite(LED,HIGH);                                             // LEDs will turn off when power is shut off

// You can add the below delay back when you want to run through the whole experiment start to end //
delay(10000);                                                                                                              
}


//INSTRUCTIONS FOR OPENING OR CLOSING THE DOOR ALL THE WAY WITHOUT CHANGING ANY NUMBERS//
/*
 * To Close Door All the Way:
        1. Find the "Open the tray door" section
        2. Follow the comments there for what parts to highlight with your cursor
        3. Use the command " ctrl / " to automatically turn all the lines selected into comments. Doing that again will undo the comments.
        
//use this line to test

        4. Compile and upload the new code to the arduino
        5. Turn on the power supply. The motor should skip the opening sequence and begin closing immediately
        6. Turn off the power supply when the door is completely closed
        7. Undo the steps you just did by following the exact same instructions (see steps 1-4)    
*/

/* To Open Door All the Way:
 *      1. Literally follow the exact same instructions but highlight the "Close the tray door" section instead
 */

 //When in doubt, panic and call Sara <3
