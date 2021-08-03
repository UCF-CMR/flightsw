//CORE software modified by Sara Wijas March 2021 for ERIE 2021
// Pin numbering modified July 2021



#include <Servo.h>
#define    LED                 A1                                    // Output for LED's                        
//#define    SERVO               9                                    // Output for Servo motor ****HAS TO BE ON 9 or 10?***                
#define    DIR                 5                                    // Output for stepper direction
#define    CP                  4                                    // Output for stepper signal    
#define    TRIGGER             3                                    // Trigger experiment 
#define    VCTRL               A2                                    // Output for 5V regulator control    

int i = 0; 
int x = 0;
//Servo myservo; 



void setup(){
pinMode(VCTRL,  OUTPUT);  digitalWrite(VCTRL,  LOW);                // 5V regulator ON/OFF control pin (for opening tray door) 
pinMode(LED,   OUTPUT);  digitalWrite(LED,   HIGH);                 // Light (LED) control pin
pinMode(CP,    OUTPUT);  digitalWrite(CP,    LOW);                  // Microstepper Pulse pin          (for opening tray door)
pinMode(DIR,   OUTPUT);  digitalWrite(DIR,   LOW);                  // Microstepper Direction pin      (for opening tray door)
pinMode(TRIGGER, INPUT);                                            // Trigger signal from Blue Origin IPC     
delay(2000);                                                        //***** ADJUST THIS DELAY AS NEEDED FOR FLIGHT *****// 
}





//Main Loop Begins Here//


//** first testing SERVO and lights
void loop()
{  
if(digitalRead(TRIGGER) == HIGH) 
  {
delay(1000);
if(digitalRead(TRIGGER) == HIGH)
  {
delay(1000);

//flash lights to signal experiment start
digitalWrite(LED,LOW);                                              // Flash the LED's to signal the end of the experiment 
delay(1500);
digitalWrite(LED,HIGH);                                             // LEDs will turn off when power is shut off

// Prepare stepper to open tray door //
digitalWrite(VCTRL, HIGH);                                          // I THINK WE'RE MISSING SOMETHING TO POWER ON AND OFF THE DRIVER      
delay(3000);                                                       
//digitalWrite(DIR,  LOW);             
//delay(1000);                         



// Open the tray door  //

digitalWrite(DIR, HIGH);
// *TO DO** EDIT the i number for full opening - 6750
for(i=0; i<3050; i++){               
    digitalWrite(CP, HIGH);
    delay(5);
    digitalWrite(CP, LOW);     }

delay(5000); // stay open 5 seconds

//  Close the tray door  //                 
digitalWrite(DIR, LOW);
// *TO DO** EDIT the i number for full opening - 6750
for(i=0; i<3050; i++){               
    digitalWrite(CP, HIGH);
    delay(5);
    digitalWrite(CP, LOW);     }

delay(5000); // wait 5 seconds before shutting off

//  Turn off the system components
digitalWrite(VCTRL,LOW);                                            // Turn off the microstep driver.
digitalWrite(LED,LOW);                                              // Flash the LED's to signal the end of the experiment 
delay(1500);
digitalWrite(LED,HIGH);                                             // LEDs will turn off when power is shut off
//delay(3600000);  
delay(20000);                                                     
   }                                                                //These two end brackets correspond to the triggers at the beginning of the main loop, add them back in when done testing
  }                                          
}
