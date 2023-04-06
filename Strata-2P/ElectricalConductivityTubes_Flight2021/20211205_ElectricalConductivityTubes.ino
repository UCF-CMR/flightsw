/* Program Name: 20211129_ElectricalConductivity_NoLoadCell.ino
 *  Author: Jared M. Long-Fox
 *  For: Center for Microgravity Research
 *  Project: Strata-2P (PI - A. Dove)
 *  Date Created: 29 November 2021
 *  Date Last Modified: 29 November 2021
 *  Purpose: Control electrical conductivity tubes and collect 
 *           data in the Strata-2P parabolic flight experiment
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
#include <Adafruit_INA260.h>
#include "HX711.h"

/**********************************************************************
//                            Assign Pins 
**********************************************************************/

// i2c pins on Arduino Uno
// 10K pull up resistors already on the Adafruit INA260 boards
const int sda = SDA;
const int scl = SCL;
const int loadCell_sda = A1;
const int loadCell_scl = A0;

//signal switch
const int switchPin = A2;

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
//                     Set up INA260 power sensors 
**********************************************************************/

//Set up high- and low-side INA260 power sensors
Adafruit_INA260 highSensor = Adafruit_INA260();
Adafruit_INA260 lowSensor = Adafruit_INA260();

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

float compression; //load cell readings


//data logging status tracking variables
bool startLogging = 0; //0 means that data logging has not started, 1 means data logging has started
bool doneLogging = 0; //0 means that data logging has not stopped, 1 means data logging has stopped

//file writing variables
int fileIterator = 0;
String fileName = "EC-" + String(fileIterator) + ".txt"; //EC is the electrical conductivity tubes
File myFile;
String dataString;

//variables to hold input and output voltages
float Vi; //input voltage
float Vo; //output voltage 


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


  // check if high side sensor is working
  if (!highSensor.begin(0x40)) {
    Serial.println("Couldn't find highSensor");
  }

  // check if low side sensor is working
  if (!lowSensor.begin(0x41)) {
    Serial.println("Couldn't find lowSensor");
  }


  //initialize load cell
  loadCell.begin(A1, A0);
  loadCell.set_scale(calibration_factor); //This value is obtained by using the SparkFun_HX711_Calibration sketch
  loadCell.tare(); //Assuming there is no weight on the scale at start up, reset the scale to 0


  //attempt to initialize SD card and display errors if mu is not found
  if (!SD.begin(8)){
    Serial.println("SD Error");
  }

  //if the SD card is found, proceed as planned
  else{
    Serial.println("Ready");
  }

  //check for existence of the file with the given name
  while (SD.exists(fileName)){ 
      fileIterator++;
      fileName = "EC-" + String(fileIterator) + ".txt";
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

  

  //if an experiment is not ongoing and the switch has been activated and an experiment has not finished
  if ((startLogging == 0) && (digitalRead(switchPin) == 0) && (doneLogging == 0)){

    //open the appropriately named file to write to
    myFile = SD.open(fileName, FILE_WRITE);

    //indicate that the file has been opened
    Serial.println(fileName + " Opened");

    //indicate that an experiment has started
    startLogging = 1;

    //set now as the current start of the most recent experiment
    startTime = millis();
    elapsedTime = 0;

    //read power sensors
    Vi = highSensor.readBusVoltage(); //input voltage in mV
    Vo = lowSensor.readBusVoltage(); //output voltage in mV

    //read load cell
    compression = loadCell.get_units();

    //log data to file
    dataString = String(Vi) + "," + String(Vo) + "," + String(elapsedTime) + "," + String(float(millis())) + "," + String(compression);
    myFile.println(dataString);
    Serial.println(dataString);
    
  }



  if ((startLogging == 1) && (doneLogging == 0) && (digitalRead(switchPin) == 0)){

    //calculate time passed since current experiment began
    elapsedTime = millis() - startTime;

    //read power sensors
    Vi = highSensor.readBusVoltage(); //`input voltage in mV
    
    //read load cell
    compression = loadCell.get_units();

    //log data to file
    dataString = String(Vi) + "," + String(Vo) + "," + String(elapsedTime) + "," + String(float(millis())) + "," + String(compression);
    myFile.println(dataString);
    Serial.println(dataString);
    
  }



  //if the switch is flipped off during data collection
  if ((digitalRead(switchPin) == 1) && (startLogging == 1)){
    //indicate that the experiment has ended
    doneLogging = 1;
  }



  if ((startLogging == 1) && (doneLogging == 1) && (digitalRead(switchPin) == 1)){

    //since the experiment has concluded, close the file and prepare for the next experiment
    myFile.close();
    Serial.println(fileName + " Closed");
    fileIterator++;

    //next file
    fileName = "EC-" + String(fileIterator) + ".txt";
    
    startLogging = 0;
    doneLogging = 0;
    
  }

   
}
/*********************************************************************
*********************************************************************/
