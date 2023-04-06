/* Program Name: 20211201_HP1.ino
 *  Author: Jared M. Long-Fox
 *  For: Center for Microgravity Research
 *  Project: Strata-2P (PI - A. Dove)
 *  Date Created: 01 December 2021
 *  Date Last Modified: 01 December 2021
 *  Purpose: Control heat pulse tubes and collect data
 *           in the Strata-2P parabolic flight experiment
 *           
 *  Procedure:
 * 
*/





/**********************************************************************
//                       Included Libraries
**********************************************************************/


//included libraries
#include <SPI.h>
#include <SD.h>
#include <Adafruit_MAX31865.h>
#include <LiquidCrystal.h>
#include "HX711.h"



/**********************************************************************
//                            Assign Pins 
**********************************************************************/

//pins for sensor0 RTD
const int CS0 = 2;
const int DI0 = 3;
const int DO0 = 4;
const int CLK0 = 5;

//pins for sensor1 RTD
const int CS1 = 8;
const int DI1 = 9;
const int DO1 = 10;
const int CLK1 = 11;

//relay control pin
const int relayPin = 19;

//signal switch
const int switchPin = 18;


/* Sparkfun level-shifting microSD breakout pins
 * 
 * SD Card uses SPI
 *    - Breakout CS  --> UNO Pin 8  --> Mega Pin 53
 *    - Breakout CD  --> Not used for UNO or Mega
 *    - Breakout DI  --> UNO Pin 11 --> Mega Pin 51
 *    - Breakout DO  --> UNO Pin 12 --> Mega Pin 50
 *    - Breakout SCK --> UNO Pin 13 --> Mega Pin 52
*/


/*********************************************************************
*********************************************************************/





/**********************************************************************
//                            Set up RTDs 
**********************************************************************/

// Use software SPI: CS, DI, DO, CLK
Adafruit_MAX31865 sensor0 = Adafruit_MAX31865(CS0, DI0, DO0, CLK0);
Adafruit_MAX31865 sensor1 = Adafruit_MAX31865(CS1, DI1, DO1, CLK1);

#define RREF 4300.0
#define RNOMINAL  1000.0

//variables to hold temperatures
float temp0;
float temp1;

/*********************************************************************
*********************************************************************/





/**********************************************************************
//                            Set up LCD 
**********************************************************************/

//set up LCD
const int rs = A0, en = A1, d4 = A2, d5 = A3, d6 = A4, d7 = A5;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
String clearString = "                "; //string that is 16 spaces, used to clear a row of the 16 col x 2 row LCD
String displayString; //will hold formatted data to display on LCD

/*********************************************************************
*********************************************************************/





/**********************************************************************
//                           Set up Load Cell
**********************************************************************/

#define calibration_factor -7050.0 //This value is obtained using the SparkFun_HX711_Calibration sketch
HX711 loadCell;

/*********************************************************************
*********************************************************************/





/**********************************************************************
//                       Experiment Variables
**********************************************************************/

//timing variables
float onTime = millis(); //time the microcontroller was turned on 
float totalTime; //total time elapsed since the microcontroller was turned on
float startTime; //will hold the time that the most recent experiment started
float elapsedTime; //will hold the time passed since the microcontroller turned on

//data logging status tracking variables
bool startLogging = 0; //0 means that data logging has not started, 1 means data logging has started
bool doneLogging = 0; //0 means that data logging has not stopped, 1 means data logging has stopped

bool heaterStatus = 0; //0 means heater is off, 1 means heater is on
const long heaterOnTime = 1000  ; //duration of the heat pulse in milliseconds

float compression; //load cell readings

//file writing variables
int fileIterator = 0;
String fileName = "HP1-" + String(fileIterator) + ".txt"; //HP1 is the first heat pulse tube
File myFile;
String dataString;

/*********************************************************************
*********************************************************************/





/**********************************************************************
//                            setup()
**********************************************************************/
void setup(){

  //initialize serial communications
  Serial.begin(115200); 

  //initialize the signal pin as an input using internal pullup resistor
  //  HIGH means switch is not active
  //  LOW means the switch has been activated
  pinMode(switchPin, INPUT_PULLUP);

  //initialize relay control pin
  pinMode(relayPin, OUTPUT);

  //initialize RTDs
  sensor0.begin(MAX31865_3WIRE);  //using 3 wire RTDs
  sensor1.begin(MAX31865_3WIRE);  //using 3 wire RTDs

  //initialize LCD (columns, rows)
  lcd.begin(16, 2);

  //initialize load cell
  loadCell.begin(SDA, SCL);
  loadCell.set_scale(calibration_factor); //This value is obtained by using the SparkFun_HX711_Calibration sketch
  loadCell.tare(); //Assuming there is no weight on the scale at start up, reset the scale to 0


  //attempt to initialize SD card and display errors if microSD is not found
  if (!SD.begin(53)){
    Serial.println("SD Error");
    lcd.setCursor(0, 0);
    lcd.print("SD Error");
  }

  //if the SD card is found, proceed as planned
  else{
    Serial.println("Ready");
    lcd.setCursor(0, 0);
    lcd.print("Ready");
  }

  //check for existence of the file with the given name
  while (SD.exists(fileName)){ 
      fileIterator++;
      fileName = "HP1-" + String(fileIterator) + ".txt";
  }

  //everything is ready, set now as the start of the experiments
  startTime = millis();

}
/*********************************************************************
*********************************************************************/





/**********************************************************************
//                             loop()
**********************************************************************/
void loop() {
  

  //if nothing is happening, ensure that the heater is off
  if ((startLogging == 0) && (digitalRead(switchPin) == 1) && (doneLogging == 0)){
    
    digitalWrite(relayPin, LOW);
  
  }


  //if an experiment is not ongoing and the switch has been activated and an experiment has not finished
  if ((startLogging == 0) && (digitalRead(switchPin) == 0) && (doneLogging == 0)){

    //open the appropriately named file to write to
    myFile = SD.open(fileName, FILE_WRITE);

    //indicate that the file has been opened
    Serial.println(fileName + " Opened");
    lcd.setCursor(0, 0);
    lcd.print("Started");
    delay(75); //delay for display purposes
    lcd.setCursor(0, 0);
    lcd.print(clearString);
    lcd.setCursor(0, 0);

    //indicate that an experiment has started
    startLogging = 1;

    //set now as the current start of the most recent experiment
    startTime = millis();
    elapsedTime = 0;
      
    //read temperatures in degrees Celsius
    temp0 = sensor0.temperature(RNOMINAL, RREF);
    temp1 = sensor1.temperature(RNOMINAL, RREF);

    //read load cell
    compression = loadCell.get_units();

    //log data to file and display data on LCD
    dataString = String(temp0) + "," + String(temp1) + "," + String(elapsedTime) + "," + String(float(millis())) + "," + String(heaterStatus) + "," + String(compression);
    myFile.println(dataString);
    Serial.println(dataString);

    displayString = String(temp0) + ", " + String(temp1);
    lcd.setCursor(0, 0);
    lcd.print(displayString);
    lcd.setCursor(0, 1);
    lcd.print("Heater: Off");
    
  }


  //if an experiment is ongoing (has not ended)
  if ((startLogging == 1) && (doneLogging == 0)){
    
    elapsedTime = millis() - startTime;

    //if the heat pulse duration has not been met or exceeded
    if (elapsedTime < heaterOnTime){

      elapsedTime = millis() - startTime;

      //activate relay
      digitalWrite(relayPin, HIGH);

      //indicate that the heater is on
      heaterStatus = 1;

      //read temperatures in degrees Celsius
      temp0 = sensor0.temperature(RNOMINAL, RREF);
      temp1 = sensor1.temperature(RNOMINAL, RREF);
      
      //read load cell
      compression = loadCell.get_units();
      
      //log data to file and display data on LCD
      dataString = String(temp0) + "," + String(temp1) + "," + String(elapsedTime) + "," + String(float(millis())) + "," + String(heaterStatus) + "," + String(compression);
      displayString = String(temp0) + ", " + String(temp1);
      myFile.println(dataString);
      Serial.println(dataString);
      lcd.setCursor(0, 0);
      lcd.print(displayString);
      lcd.setCursor(0, 1);
      lcd.print(clearString);
      lcd.setCursor(0, 1);
      lcd.print("Heater: On"); 
     
    }

    //if the heat pulse duration has been met or exceeded
    else{

      //turn off heater
      digitalWrite(relayPin, LOW);
      heaterStatus = 0;

      //display that the heater has been turned off on the LCD
      lcd.setCursor(0, 1);
      lcd.print(clearString);
      lcd.setCursor(0, 1);
      lcd.print("Heater: Off");

      //read temperatures in degrees Celsius
      temp0 = sensor0.temperature(RNOMINAL, RREF);
      temp1 = sensor1.temperature(RNOMINAL, RREF);

      //read load cell
      compression = loadCell.get_units();

      //calculate time passed since current experiment began
      elapsedTime = millis() - startTime;

      //log data to file and display data on LCD
      dataString = String(temp0) + "," + String(temp1) + "," + String(elapsedTime) + "," + String(float(millis())) + "," + String(heaterStatus) + "," + String(compression);
      displayString = String(temp0) + ", " + String(temp1);
      myFile.println(dataString);
      Serial.println(dataString);
      lcd.setCursor(0, 0);
      lcd.print(displayString);
      lcd.setCursor(0, 1);
      lcd.print(clearString);
      lcd.setCursor(0, 1);
      lcd.print("Heater: Off"); 

      //break out of if statement
      doneLogging = 1;

    }

    
  }


  //if the switch is still activated but the heat pulse has ended
  if ((digitalRead(switchPin) == 0) && (doneLogging == 1)){

    //leave relay off
    digitalWrite(relayPin, LOW);
    heaterStatus = 0;

    //display that the heater has been turned off on the LCD
    lcd.setCursor(0, 1);
    lcd.print(clearString);
    lcd.setCursor(0, 1);
    lcd.print("Heater: Off");
  
    //read temperatures in degrees Celsius
    temp0 = sensor0.temperature(RNOMINAL, RREF);
    temp1 = sensor1.temperature(RNOMINAL, RREF);

    //calculate time passed since current experiment began
    elapsedTime = millis() - startTime;

    //log data to file and display data on LCD
    dataString = String(temp0) + "," + String(temp1) + "," + String(elapsedTime) + "," + String(float(millis())) + "," + String(heaterStatus) + "," + String(compression);
    displayString = String(temp0) + ", " + String(temp1);
    myFile.println(dataString);
    Serial.println(dataString);
    lcd.setCursor(0, 0);
    lcd.print(displayString);
    lcd.setCursor(0, 1);
    lcd.print(clearString);
    lcd.setCursor(0, 1);
    lcd.print("Heater: Off"); 
   
  }


  //if the switch is turned back off and the heat pulse has ended
  if ((digitalRead(switchPin) == 1) && (doneLogging == 1)){

    //leave relay off
    digitalWrite(relayPin, LOW);
    heaterStatus = 0;

    //close the file
    myFile.close();
    Serial.println(fileName + " Closed");
    lcd.setCursor(0, 0);
    lcd.print("Experiment ended");
    lcd.setCursor(0, 1);
    lcd.print("Heater: Off");

    //get ready for next file
    fileIterator++;

    //next file
    fileName = "HP1-" + String(fileIterator) + ".txt";

    //reset for next experiment
    startLogging = 0;
    doneLogging = 0;
    
  }
   
}
/*********************************************************************
*********************************************************************/
