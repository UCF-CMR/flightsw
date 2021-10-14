//Door Testing for Dummies

//Define variables - DO NOT TOUCH //
//#include <Servo.h>
#define    LED                 7                                    // Output for LED's                                      
#define    DIR                 5                                    // Output for stepper direction
#define    CP                  4                                    // Output for stepper signal    
#define    TRIGGER             3                                    // Trigger experiment 
#define    VCTRL               2                                    // Output for 5V regulator control    

int i = 0; 
int x = 0; 

//Define pins - DO NOT TOUCH //
void setup(){
pinMode(VCTRL,  OUTPUT);  digitalWrite(VCTRL,  LOW);                // 5V regulator ON/OFF control pin (for opening tray door) 
pinMode(LED,   OUTPUT);  digitalWrite(LED,   HIGH);                 // Light (LED) control pin
pinMode(CP,    OUTPUT);  digitalWrite(CP,    LOW);                  // Microstepper Pulse pin          (for opening tray door)
pinMode(DIR,   OUTPUT);  digitalWrite(DIR,   LOW);                  // Microstepper Direction pin      (for opening tray door)  
delay(2000);                                                        
}


///Main Loop Begins Here///

void loop()
{  

//flash lights to signal experiment start - DO NOT TOUCH //
digitalWrite(LED,LOW);                                              // Flash the LED's to signal the end of the experiment 
delay(1500);
digitalWrite(LED,HIGH);                                             // LEDs will turn off when power is shut off

// Prepare stepper to open tray door - DO NOT TOUCH //
digitalWrite(VCTRL, HIGH);                                                
delay(3000);                                                                               



//  Open the tray door  //
//((If following instructions at the bottom, start highlighting here))//
digitalWrite(DIR, HIGH);
for(i=0; i<6750; i++){               
    digitalWrite(CP, HIGH);
    delay(5);
    digitalWrite(CP, LOW);     }
//((Stop highlighting here))//

delay(5000); // stay open 5 seconds


//  Close the tray door  //
//((If following instructions at the bottom, start highlighting here))//                 
digitalWrite(DIR, LOW);
// *TO DO** EDIT the i number for full opening - 6750
for(i=0; i<6750; i++){               
    digitalWrite(CP, HIGH);
    delay(5);
    digitalWrite(CP, LOW);     }
//((Stop highlighting here))//


delay(5000); // wait 5 seconds before shutting off

//  Turn off the system components - DO NOT TOUCH //  
digitalWrite(VCTRL,LOW);                                            // Turn off the microstep driver.
digitalWrite(LED,LOW);                                              // Flash the LED's to signal the end of the experiment 
delay(1500);
digitalWrite(LED,HIGH);                                             // LEDs will turn off when power is shut off

// You can add the below delay back when you want to run through the whole experiment start to end //
//delay(3600000);                                                                                                              
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
