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
//			Code written by: Hunter Nagler
//				               Fall 2014
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

#define    SVPWR              12    // Not used.  <-pretty sure it is? -Seamus                                     
#define    SERVO               9    // Output for Servo motor                        
#define    LED                 8    // Output for LED's 
#define    LIMIT               7    // switch for door                 // I don't think this is actually used -Seamus
#define    MW                  6    // Output for muscle wire                          
#define    DOOR                5    // Output for microstep power                                        
#define    DIR                 4    // Output for stepper direction
#define    CP                  3    // Output for stepper signal    
#define    CAM                 2    // Output for camera trigger   
#define    TRIGGER             10   // Trigger experiment 

int i = 0; 
int x = 0;
Servo myservo; 

int limitFlag = LOW;                                                   // I don't think this is actually used -Seamus

void setup(){
  
     pinMode(LED,   OUTPUT);  digitalWrite(LED,   HIGH);   // Light (LED) control pin
     pinMode(CAM,   OUTPUT);  digitalWrite(CAM,   LOW);    // Camera control pin
     pinMode(MW,    OUTPUT);  digitalWrite(MW,    LOW);    // Muscle Wire control pin,        (for lowering the cylinder)
     pinMode(DOOR,  OUTPUT);  digitalWrite(DOOR,  LOW);    // Microstepper ON/OFF control pin (for opening tray door) 
     pinMode(CP,    OUTPUT);  digitalWrite(CP,    LOW);    // Microstepper Pulse pin          (for opening tray door)
     pinMode(DIR,   OUTPUT);  digitalWrite(DIR,   LOW);    // Microstepper Direction pin      (for opening tray door)
  //   pinMode(SVPWR, OUTPUT);  digitalWrite(SVPWR, HIGH);    // Servo power                     (for turning the scooper)
  
     pinMode(LIMIT, INPUT);                                // I don't think is actually used -Seamus
     pinMode(TRIGGER, INPUT);
     
     myservo.attach(12);            // attaches the servo on pin 12 to the servo object 
     delay(10);
     myservo.write(90);             // Writes the rotation speed, 0 is max clock, 90 is stopped, 180 is max counter-clock
     delay(60000);

}

////////////////////////////////////////////////////////////////////////////////////////
//       Main Loop Begins Here
////////////////////////////////////////////////////////////////////////////////////////

void loop()
{  
  if(digitalRead(TRIGGER) == HIGH) 
  {
    delay(1000);

  if(digitalRead(TRIGGER) == HIGH)
  {
    delay(1000);
  

          // Prepare stepper to open tray door //
          //-----------------------------------//
          
     digitalWrite(DOOR, HIGH);            
     delay(1000);                         
     digitalWrite(DIR,  LOW);             
     delay(1000);                         

    
          //  Open the tray door //
          //---------------------//
                             
     for(i=0; i<6750; i++){               
          digitalWrite(CP, HIGH);
          delay(2);
          digitalWrite(CP, LOW);     }
          
     digitalWrite(DOOR, LOW);             //Turn off the microstep driver

          //  Activate the projectile launcher //
          //-----------------------------------//
    
     delay(2000);
     digitalWrite(MW, HIGH);              // Turns on the muscle wire circuit. This is a
     delay(2000);                         // high current circuit, limited to just under 
     digitalWrite(MW, LOW);               // a 500mA draw. It is active for 2.0 seconds
     delay(1000);                         

                                         
          //  Activate the servo motor //
          //---------------------------//
          
     myservo.write(0);
     delay(900);
     
     myservo.detach();
     
          //  Turn off the system components //
          //---------------------------------//
    
     digitalWrite(DOOR,LOW);              // Turn off the microstep driver.
     digitalWrite(LED,LOW);               // Turn off the LED's.
     delay(3600000);  
  }
  }                    
}
