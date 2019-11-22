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
                                     
#define    MW                  8    // Output for muscle wire  
#define    LED                 7    // Output for LED's                        
//#define    SERVO               9    // Output for Servo motor ****HAS TO BE ON 9 or 10?***
 //actually "attached" below                  
#define    DIR                 5    // Output for stepper direction
#define    CP                  4    // Output for stepper signal    
#define    TRIGGER             3   // Trigger experiment 
#define    VCTRL               2    // Output for 5V regulator control    

int i = 0; 
int x = 0;
Servo myservo; 

void setup(){
  
//     pinMode(VCTRL,  OUTPUT);  digitalWrite(VCTRL,  LOW);  // 5V regulator ON/OFF control pin (for opening tray door) 
     pinMode(LED,   OUTPUT);  digitalWrite(LED,   HIGH);   // Light (LED) control pin
     pinMode(MW,    OUTPUT);  digitalWrite(MW,    LOW);    // Muscle Wire control pin,        (for lowering the cylinder)
     pinMode(CP,    OUTPUT);  digitalWrite(CP,    LOW);    // Microstepper Pulse pin          (for opening tray door)
     pinMode(DIR,   OUTPUT);  digitalWrite(DIR,   LOW);    // Microstepper Direction pin      (for opening tray door)
  //   pinMode(SVPWR, OUTPUT);  digitalWrite(SVPWR, HIGH);    // Servo power                     (for turning the scooper)
  
     pinMode(TRIGGER, INPUT);       // Trigger signal from Blue Origin IPC 
     
 
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

  if(digitalRead(TRIGGER) == HIGH) 
  {
    delay(1000);

  if(digitalRead(TRIGGER) == HIGH)
  {
    delay(1000);

//flash lights to signal experiment start
    digitalWrite(LED,LOW);               // Flash the LED's to signal the end of the experiment 
    delay(1500);
    digitalWrite(LED,HIGH);              // LEDs will turn off when power is shut off
   

          // Prepare stepper to open tray door //
          //-----------------------------------//
          
//     digitalWrite(VCTRL, HIGH);      // I THINK WE'RE MISSING SOMETHING TO POWER ON AND OFF THE DRIVER      
//     delay(10000);                         
     digitalWrite(DIR,  LOW);             
     delay(1000);                         
//
//    
//          //  Open the tray door //
//          //---------------------//
//                             
     digitalWrite(DIR, HIGH);

// *TODO** EDIT the i number for full opening - 6750
     for(i=0; i<6750; i++){               
          digitalWrite(CP, HIGH);
          delay(5);
          digitalWrite(CP, LOW);     }
          
//     digitalWrite(DOOR, LOW);             //Turn off the microstep driver

//          //  Activate the projectile launcher //
//          //-----------------------------------//
//    
     delay(5000);
     
     digitalWrite(MW, HIGH);              // Turns on the muscle wire circuit. This is a
     delay(2500);                         // high current circuit, limited to just under 
     digitalWrite(MW, LOW);               // a 500mA draw. It is active for 2.5 seconds
     delay(5000);                         

//                                         
          //  Activate the servo motor //
          //---------------------------//
     
     myservo.attach(9);            // attaches the servo on pin 6 to the servo object 
     myservo.write(90);
         
     myservo.write(45);       // halfway one direction
     delay(1800);              // SET THIS TIME BASED ON HOW MUCH THIS NEEDS TO TURN (900?)
                              //one full turn is about 1200 at 45speed

     myservo.write(90);
     delay(500);
     
     myservo.detach();
     
//          //  Turn off the system components //
          //---------------------------------//


//     digitalWrite(VCTRL,LOW);              // Turn off the microstep driver.
     digitalWrite(LED,LOW);               // Flash the LED's to signal the end of the experiment 
     delay(1500);
     digitalWrite(LED,HIGH);              // LEDs will turn off when power is shut off
     
     delay(3600000);  
//    delay(20000);
  }
  }                                          
}
