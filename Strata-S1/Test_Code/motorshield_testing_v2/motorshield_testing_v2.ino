

////////////////////////////////////////////////////////////////////////////////////////
//       Digital I/O
////////////////////////////////////////////////////////////////////////////////////////
//#define  "X"                 SCL  // Not used.
//#define  "X"                 SDA  // Not used.
//#define  "X"                 AREF // Not used.
//#define  "X"                 GND  // Not used.
#define  TRIGGER              13   // Trigger experiment to start in microgravity phase, stop at the end
//#define  "X"                 12   // Not used.
#define  BWD                3   // Motor A - A2
#define  FWD                2   // Motor A - A1                                     
#define  ENABLE_A           1    // Enable pin for Motor A

//#define  MB_A2                 8    // Motor B - A2                     
//#define  MB_A1                 7    // Motor B - A1
//#define  ENABLE_B              6    // Enable pin for Motor B                                           
//#define  ENABLE_C              5    // Enable pin for Motor C                
//#define  MC_A1                 4    // Motor C - A1
//#define  MD_A2                 3    // Motor D - A2  
//#define  MD_A1                 2    // Motor D - A1   
//#define  ENABLE_D              1    // Enable pin for Motor D
//#define  MC_A2                 0    // Motor C - A2

////////////////////////////////////////////////////////////////////////////////////////
//       Analog In 6
////////////////////////////////////////////////////////////////////////////////////////
//#define  "X"                 A5   // Not used.                                
//#define  "X"                 A4   // Not used.                                
//#define  POS_D               A3   // Feedback on Motor D position - actually wired to A
//#define  POS_C               A2   // Feedback on Motor C position - actually wired to B
//#define  POS_B               A1   // Feedback on Motor B position - actually wired to C
//#define  POS_A               A0   // Feedback on Motor A position - actually wired to D

#define  POS_A               A3

////////////////////////////////////////////////////////////////////////////////////////
//       Command Routine Variables and Include files
////////////////////////////////////////////////////////////////////////////////////////

// Modified from test code for operating the P-Type Actuonix linear actuator
// Addie Dove
// April 19, 2019

int posA;
int posB;
int posC;
int posD;

int goal = 0;

int maxA = 0;
int maxB = 0;
int maxC = 0;
int maxD = 0;

int MAXE_A = 75;
int MAXE_B = 75;
int MAXE_C = 75; 
int MAXE_D = 75;

int MINE_A = 450;
int MINE_B = 450;
int MINE_C = 450;
int MINE_D = 450;

int allMAX = 1023;

void setup() {

  Serial.begin(9600);
 
// Set up pins for all of the motors

// For just testing one tube
  pinMode(ENABLE_A,   OUTPUT);
  pinMode(BWD,      OUTPUT);
  pinMode(FWD,      OUTPUT);

  pinMode(POS_A,      INPUT);

  digitalWrite(ENABLE_A,    LOW);
  digitalWrite(BWD,       LOW);
  digitalWrite(FWD,       LOW);
  
  
  posA = analogRead(POS_A);
  maxA = posA;

  Serial.println("Position A:");
  Serial.println(posA);

//// Tube A
//  pinMode(ENABLE_A,   OUTPUT);
//  pinMode(MA_A1,      OUTPUT);
//  pinMode(MA_A2,      OUTPUT);
//
//  pinMode(POS_A,      INPUT);
//
//  digitalWrite(ENABLE_A,    LOW);
//  digitalWrite(MA_A1,       LOW);
//  digitalWrite(MA_A2,       LOW);
//  
//  
//  posA = analogRead(POS_A);
//  maxA = posA;
//
//  Serial.println("Position A:");
//  Serial.println(posA);

// Tube B
//  pinMode(ENABLE_B,   OUTPUT);
//  pinMode(MB_A1,      OUTPUT);
//  pinMode(MB_A2,      OUTPUT);
//
//  pinMode(POS_B,      INPUT);
//
//  digitalWrite(ENABLE_B,    LOW);
//  digitalWrite(MB_A1,       LOW);
//  digitalWrite(MB_A2,       LOW);
//    
//  posB = analogRead(POS_B);
//  maxB = posB;
//
//  Serial.println("Position B:");
//  Serial.println(posB);
//
//// Tube C
//  pinMode(ENABLE_C,   OUTPUT);
//  pinMode(MC_A1,      OUTPUT);
//  pinMode(MC_A2,      OUTPUT);
//
//  pinMode(POS_C,      INPUT);
//
//  digitalWrite(ENABLE_C,    LOW);
//  digitalWrite(MC_A1,       LOW);
//  digitalWrite(MC_A2,       LOW);
//  
//  posC = analogRead(POS_C);
//  maxC = posC;
//
//  Serial.println("Position C:");
//  Serial.println(posC);
//
//// Tube D
//  pinMode(ENABLE_D,   OUTPUT);
//  pinMode(MD_A1,      OUTPUT);
//  pinMode(MD_A2,      OUTPUT);
//
//  pinMode(POS_D,      INPUT);
//
//  digitalWrite(ENABLE_D,    LOW);
//  digitalWrite(MD_A1,       LOW);
//  digitalWrite(MD_A2,       LOW);
//  
//  posD = analogRead(POS_D);
//  maxD = posD;
//
//  Serial.println("Position D:");
//  Serial.println(posD);

//// make sure the entrapulators are extended
//  if(posA>MAXE_A){
//    while(pos>MAXE_A){
//      digitalWrite(FWD,      HIGH);
//      digitalWrite(ENABLE_A,  HIGH);
//      posA = analogRead(POS_A);
//      Serial.print("Setting to: ");
//      Serial.println(posA);
//    }
//    digitalWrite(ENABLE_A,    LOW);
//    digitalWrite(FWD,        LOW);
//  }
}


//*************** MAIN LOOP *****************
void loop() {

 Serial.println("Beginning of main loop");
 delay(10000);        // wait a little bit between tests (10 sec)

// move Entrap one direction for bloop ms
//bigger numbers means contraction ~30 max (for LA_B, et least)
// use A2 to contract
//smaller numbers means extension ~15 min
// use A1 to extend


// add in check to make sure it doesn't go longer than like 900ms 
int bloop=450;

 if(digitalRead(TRIGGER) == HIGH)          // Check to see if the experiment is triggered.
 {                                       // If it is, start the main program.
  Serial.println("Trigger high 1");
  if(digitalRead(TRIGGER) == HIGH)      // Check again
  {
  Serial.println("Trigger high 2");

    // just control with how long it goes
    digitalWrite(BWD,     HIGH);
    digitalWrite(ENABLE_A,  HIGH);
    posA = analogRead(POS_A);
    Serial.print("BWD start ");
    Serial.println(posA);            // record position

    delay(bloop);                    // HIGH for bloop ms

    posA = analogRead(POS_A);
    Serial.print("BWD stop ");       // record position
    Serial.println(posA);
    digitalWrite(ENABLE_A,   LOW);
    digitalWrite(BWD,      LOW);
  
    delay(4000);       // wait 4 seconds
  
    // move Entrap the other direction for BLOOP ms
    digitalWrite(FWD,     HIGH);
    digitalWrite(ENABLE_A,  HIGH);
    posA = analogRead(POS_A);
    Serial.print("fwd start ");
    Serial.println(posA);          // record position
  
    delay(bloop);                  // HIGH for bloop ms
    
    posA = analogRead(POS_A);
    Serial.print("fwd stop ");
    Serial.println(posA);          // record position 
    digitalWrite(ENABLE_A,   LOW);
    digitalWrite(FWD,      LOW);
  

// fully retract 

// goal = MINE_A;
// Serial.print("Retracting to ");
// Serial.println(goal);
// 
// if(posA<goal){
//  while(posA<goal){
//   digitalWrite(MA_A2,     HIGH);
//   digitalWrite(ENABLE_A,  HIGH);
//   posA = analogRead(POS_A);
//   Serial.print("A_A2 (bkwd) ");
//   Serial.println(posA);            // record position
//  }
//  digitalWrite(ENABLE_A,   LOW);
//  digitalWrite(MA_A2,      LOW);
// }
// else{
//  while(posA>goal){
//    digitalWrite(MA_A1,     HIGH);
//    digitalWrite(ENABLE_A,  HIGH);
//    posA = analogRead(POS_A);
//    Serial.print("A_A1 (fwd)");
//    Serial.println(posA);
//  }
//  digitalWrite(ENABLE_A,   LOW);
//  digitalWrite(MA_A1,      LOW);
// } 
//
//// hold
// delay(5000); 
//
//// extend about halfway 
// goal = 250;
// Serial.print("Extending to ");
// Serial.println(goal);
//
// if(posA<goal){
//  while(posA<goal){
//   digitalWrite(MA_A2,     HIGH);
//   digitalWrite(ENABLE_A,  HIGH);
//   posA = analogRead(POS_A);
//   Serial.print("A_A2 (bkwd) ");
//   Serial.println(posA);            // record position
//  }
//  digitalWrite(ENABLE_A,   LOW);
//  digitalWrite(MA_A2,      LOW);
// }
// else{
//  while(posA>goal){
//    digitalWrite(MA_A1,     HIGH);
//    digitalWrite(ENABLE_A,  HIGH);
//    posA = analogRead(POS_A);
//    Serial.print("A_A1 (fwd)");
//    Serial.println(posA);
//  }
//  digitalWrite(ENABLE_A,   LOW);
//  digitalWrite(MA_A1,      LOW);
// }
//
//// hold 
// delay(5000);
//
//// bring all the way back
// goal = MINE_A;
// Serial.print("Retracting to ");
// Serial.println(goal);
//
// if(posA<goal){
//  while(posA<goal){
//   digitalWrite(MA_A2,     HIGH);
//   digitalWrite(ENABLE_A,  HIGH);
//   posA = analogRead(POS_A);
//   Serial.print("A_A2 (bkwd) ");
//   Serial.println(posA);            // record position
//  }
//  digitalWrite(ENABLE_A,   LOW);
//  digitalWrite(MA_A2,      LOW);
// }
// else{
//  while(posA>goal){
//    digitalWrite(MA_A1,     HIGH);
//    digitalWrite(ENABLE_A,  HIGH);
//    posA = analogRead(POS_A);
//    Serial.print("A_A1 (fwd)");
//    Serial.println(posA);
//  }
//  digitalWrite(ENABLE_A,   LOW);
//  digitalWrite(MA_A1,      LOW);
// } 
//
//// hold
// delay(5000); 

//// re-extend out
// goal = MAXE_A;
// Serial.print("Extending to ");
// Serial.println(goal);
//
// if(posA<goal){
//  while(posA<goal){
//   digitalWrite(MA_A2,     HIGH);
//   digitalWrite(ENABLE_A,  HIGH);
//   posA = analogRead(POS_A);
//   Serial.print("A_A2 (bkwd) ");
//   Serial.println(posA);            // record position
//  }
//  digitalWrite(ENABLE_A,   LOW);
//  digitalWrite(MA_A2,      LOW);
// }
// else{
//  while(posA>goal){
//    digitalWrite(MA_A1,     HIGH);
//    digitalWrite(ENABLE_A,  HIGH);
//    posA = analogRead(POS_A);
//    Serial.print("A_A1 (fwd)");
//    Serial.println(posA);
//  }
//  digitalWrite(ENABLE_A,   LOW);
//  digitalWrite(MA_A1,      LOW);
// } 



// hold

//// just testing to see if it will move and hold 
// if(posA<goal){
//  while(posA<goal){
//   digitalWrite(BWD,     HIGH);
//   digitalWrite(ENABLE_A,  HIGH);
//   posA = analogRead(POS_A);
//   Serial.print("bkwd ");
//   Serial.println(posA);            // record position
//  }
//  digitalWrite(ENABLE_A,   LOW);
//  digitalWrite(BWD,      LOW);
// }
// else{
//  while(posA>goal){
//    digitalWrite(FWD,     HIGH);
//    digitalWrite(ENABLE_A,  HIGH);
//    posA = analogRead(POS_A);
//    Serial.print("fwd");
//    Serial.println(posA);
//  }
//  digitalWrite(ENABLE_A,   LOW);
//  digitalWrite(FWD,      LOW);
// }



//// switch motors!    
//  delay(5000);      
//
//// just control with how long it goes
//   digitalWrite(MA_A2,     HIGH);
//   digitalWrite(ENABLE_A,  HIGH);
//   posA = analogRead(POS_A);
//   Serial.print("A_A2 start ");
//   Serial.println(posA);            // record position
//
//  delay(bloop);                    // HIGH for bloop ms
//
//  posA = analogRead(POS_A);
//  Serial.print("A_A2 stop ");       // record position
//  Serial.println(posA);
//  digitalWrite(ENABLE_A,   LOW);
//  digitalWrite(MA_A2,      LOW); 
//
//  delay(2000);       // wait 2 seconds
//
//// move Entrap the other direction for 10 ms
//  digitalWrite(MA_A1,     HIGH);
//  digitalWrite(ENABLE_A,  HIGH);
//  posB = analogRead(POS_A);
//  Serial.print("A_A1 start ");
//  Serial.println(posA);          // record position
//
//  delay(bloop);                  // HIGH for bloop ms
//
//  posA = analogRead(POS_A);
//  Serial.print("A_A1 stop ");
//  Serial.println(posA);          // record position 
//  digitalWrite(ENABLE_A,   LOW);
//  digitalWrite(MA_A1,      LOW);

//  }
  }
 }
}
