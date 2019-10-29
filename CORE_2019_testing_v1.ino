////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
//
//        ////////  ////////  ///////    /////////////
//       ///       //    //  ///   //   ///
//      ///       //    //  ///   //   ///
//     ///       //    //  ///////    ////////
//    ///       //    //  /// ///    ///
//   ///       //    //  ///  ///   ///                                            Mk.II
//  ////////  ////////  ///   ///  ////////////////////////////////////////////////////
//
//                        University of Central Florida
//              Colwell Research Group, Center for Microgravity Research 
//                            All rights Reserved.
//      Code written by: Hunter Nagler
//                       Fall 2014
//
//      Revised by:      Seamus Anderson and Lex Yates
//                       8 Feb 2018
//     
////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
//
//  This code is for use with the CORE experiment at the University of Central
//  Florida. It is written for the Arduino UNO R3 microcontroller utilizing the ATMEL 
//  ATMEGA328 chipset. Details for this board can be found at "www.arduino.cc".
//
////////////////////////////////////////////////////////////////////////////////////////
//  Main Code Begins Here
////////////////////////////////////////////////////////////////////////////////////////

#include <Servo.h>
                                     
//#define    SERVO               9    // Output for Servo motor                        
#define    MW                  8    // Output for muscle wire  
#define    LED                 7    // Output for LED's 
                        
//#define    DOOR                5    // Output for microstep power                                        
#define    DIR                 4    // Output for stepper direction
#define    CP                  3    // Output for stepper signal    
#define    TRIGGER             10   // Trigger experiment 

int i = 0; 
int x = 0;
Servo myservo; 

void setup(){
  
     pinMode(LED,   OUTPUT);  digitalWrite(LED,   HIGH);   // Light (LED) control pin
     pinMode(MW,    OUTPUT);  digitalWrite(MW,    LOW);    // Muscle Wire control pin,        (for lowering the cylinder)
//    pinMode(DOOR,  OUTPUT);  digitalWrite(DOOR,  LOW);    // Microstepper ON/OFF control pin (for opening tray door) 
     pinMode(CP,    OUTPUT);  digitalWrite(CP,    LOW);    // Microstepper Pulse pin          (for opening tray door)
     pinMode(DIR,   OUTPUT);  digitalWrite(DIR,   LOW);    // Microstepper Direction pin      (for opening tray door)
  //   pinMode(SVPWR, OUTPUT);  digitalWrite(SVPWR, HIGH);    // Servo power                     (for turning the scooper)
  
     pinMode(TRIGGER, INPUT);       // Trigger signal from Blue Origin IPC 
     
     myservo.attach(9);            // attaches the servo on pin 12 to the servo object 
     myservo.write(90);             // Writes the rotation speed, 0 is max clock, 90 is stopped, 180 is max counter-clock
 
//***** ADJUST THIS DELAY AS NEEDED FOR FLIGHT *****//     
     delay(2000);         

}

////////////////////////////////////////////////////////////////////////////////////////
//       Main Loop Begins Here
////////////////////////////////////////////////////////////////////////////////////////
//

//** first testing SERVO and lights
void loop()
{  

          // Prepare stepper to open tray door //
          //-----------------------------------//
          
//     digitalWrite(DOOR, HIGH);      // I THINK WE'RE MISSING SOMETHING TO POWER ON AND OFF THE DRIVER      
//     delay(1000);                         
     digitalWrite(DIR,  LOW);             
     delay(1000);                         
//
//    
//          //  Open the tray door //
//          //---------------------//
//                             
     digitalWrite(DIR, HIGH);

     for(i=0; i<5050; i++){               
          digitalWrite(CP, HIGH);
          delay(5);
          digitalWrite(CP, LOW);     }
          
//     digitalWrite(DOOR, LOW);             //Turn off the microstep driver

//          //  Activate the projectile launcher //
//          //-----------------------------------//
//    
     delay(5000);
     
     digitalWrite(MW, HIGH);              // Turns on the muscle wire circuit. This is a
     delay(1000);                         // high current circuit, limited to just under 
     digitalWrite(MW, LOW);               // a 500mA draw. It is active for 2.0 seconds
     delay(1000);                         

                                         
          //  Activate the servo motor //
          //---------------------------//
          
     myservo.write(45);       // halfway one direction
     delay(900);              // SET THIS TIME BASED ON HOW MUCH THIS NEEDS TO TURN

     myservo.write(90);
     delay(1000);
     
//     myservo.detach();
     
          //  Turn off the system components //
          //---------------------------------//
    
//     digitalWrite(DOOR,LOW);              // Turn off the microstep driver.
     digitalWrite(LED,LOW);               // Turn off the LED's.
//     delay(3600000);  
    delay(10000);
                      
}
