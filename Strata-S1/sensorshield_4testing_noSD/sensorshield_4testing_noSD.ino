/*
Code to record data for the Strata-S1 experiment.

This code is intended just for ground-based testing of an individual tube

The Strata-S1 experiment consists of:
Tube A: Strata-sphere -- no sensors
Tube B: Load Cell only
Tube C: Load Cell, FSR1, FSR2
Tube D: Load Cell, FSR1, FSR2

All tubes also have an Entrapulator - code for controlling those is separate.

This code will take data the enitre time the experiment is powered.

Uses the HX711 load cell library

Modified from the flight code to be used for ground testing. 
Created 19-06-05
*/


////////////////////////////////////////////////////////////////////////////////////////
//       Digital I/O
////////////////////////////////////////////////////////////////////////////////////////
//#define  "X"                 SCL   // Not used.
//#define  "X"                 SDA   // Not used.
//#define  "X"                 AREF  // Not used.
//#define  "X"                 GND   // Not used.
#define  CLK                 13    // SDcard CLK
#define  MISO                12    // SDcard MISO
#define  MOSI                11    // SDcard MOSI
#define  LC_CLK               9    // Tube B Load Cell CLK                                    
#define  LC_DOUT             10    // Tube B Load Cell DATA OUT
#define  chipSelect           8    // SDcard chipSelect

//#define  LC_B_CLK            10    // Tube B Load Cell CLK                                    
//#define  LC_B_DOUT            9    // Tube B Load Cell DATA OUT
//#define  LC_D_CLK             7    // Tube D Load Cell CLK                     
//#define  LC_D_DOUT            6    // Tube D Load Cell DATA OUT
//#define  LC_C_CLK             5    // Tube C Load Cell CLK                                           
//#define  LC_C_DOUT            4    // Tube C Load Cell DATA OUT              
//#define  "X"                  3    // Not used.  
//#define  "X"                  2    // Not used.   
//#define  "X"                  1    // Not used.
//#define  "X"                  0    // Not used.

////////////////////////////////////////////////////////////////////////////////////////
//       Analog In 6
////////////////////////////////////////////////////////////////////////////////////////
// If using force sensors
#define FSR_1                 A2
#define FSR_2                 A5

const char PIN_FSR_ARR[] = {FSR_1, FSR_2};

//#define  FSR_B1               A0   // Tube B FSR 1 - not used
//#define  FSR_D2               A1   // Tube D FSR 2
//#define  FSR_C2               A2   // Tube C FSR 2
//#define  FSR_B2               A3   // Tube B FSR 2  - not used
//#define  FSR_D1               A4   // Tube D FSR 1                                
//#define  FSR_C1               A5   // Tube C FSR 1                              


////////////////////////////////////////////////////////////////////////////////////////
//       Command Routine Variables and Include files
////////////////////////////////////////////////////////////////////////////////////////

#include <HX711.h>
#include <SPI.h>
#include <SD.h>

// LOAD CELLS
// Need to calibrate the Load Cells before flight to get this value
const float calfactor_LC = 2140;   //This value is obtained using the SparkFun_HX711_Calibration sketch

HX711 loadcell;

// FSRs
// Measure the voltage at 5V and resistance of your 3.3k resistor, and enter
// their value's below:
const float VCC_C = 5.01;         // Measured voltage of 5V line into IC for wires connected to C
const float VCC_D = 4.98;         // Measured voltage of 5V line into IC for wires connected to D
//const float R_DIV_A1  = 9810.0;   // Measured resistance of 10k resistor going to pin A1
//const float R_DIV_A4  = 9800.0;   // Measured resistance of 10k resistor going to pin A4
const float R_DIV_A2  = 9840.0;   // Measured resistance of 10k resistor going to pin A2
const float R_DIV_A5  = 9850.0;   // Measured resistance of 10k resistor going to pin A5

// Set up file to save data
// TODO: Need to update to create a new file name each time (maybe with data or time?)
//File initFile;    // file to save initialization values
//File dataFile;    // file to save all data

void setup() {
// Open serial port
  Serial.begin(9600);

// ***Set up SD card***
  Serial.print("Initializing SD card...");

  if (!SD.begin(chipSelect)) {
    Serial.println("Card initialization failed!");
    // don't do anything more
//    while (1);
  }
  Serial.println("Card initialized.");

// ** Open a file to write the initilization values. **
// Note that only one file can be open at a time, so you have to close this one before opening another
//  initFile = SD.open("initvals.txt",  FILE_WRITE);
//  initFile.println("New data");   // TODO: add something to print time

// ***LOAD CELLS***
  Serial.println("Load Cell Setup");

// Set up load cell
  loadcell.begin(LC_DOUT, LC_CLK);
  loadcell.set_scale(calfactor_LC); //This value is obtained by using the SparkFun_HX711_Calibration sketch
  loadcell.tare(); //Scales the values to the fully extended state.
  Serial.print("Tare value  LC:");
  Serial.println(loadcell.get_units(), 1);
//  initFile.print("Tare value  LC:");
//  initFile.println(loadcell.get_units(), 1);


// ***FSRs***
// Set up FSRs 
  Serial.println("FSR Setup");

// Set up FSRs in Tube C
//  pinMode(FSR_1, INPUT);
//  pinMode(FSR_2, INPUT);
  // read average values of FSRs
 
// Close the initilization file on the SD card
//  initFile.close();

// write that we've restarted the file to the SDcard
// close the file
//  dataFile = SD.open("datalog.txt",  FILE_WRITE);
//  dataFile.println("New data");  // TODO: add something to print time
//  dataFile.println("LC,FSR_1,FSR_2");
//  dataFile.close();
  
}

void loop() {    

// ** Open the data file each time through the loop **
// Note that only one file can be open at a time, so you have to close this one before opening another.  
//  dataFile = SD.open("datalog.txt",  FILE_WRITE);  // TODO: add something to print time

// make a string in which we'll the data to log:
  String dataString = "";
// example dataString"valLC_B,valLC_C,valLC_D,valFSR_C1,valFSR_C2,valFSR_D1,valFSR_D2"
  
// Read data from all of the sensors and put into the data file
// *** LOAD CELLS *** 

  long valLC = loadcell.read();                 //scale.read() gets values, here returns a float
  Serial.println(valLC);
  Serial.println(valLC);
// Read data from the load cell and start the string:
// put into the datastring so that it can be written to the text file
//  dataString += String(valLC);
//  dataString += ",";

// ***FSRs***
// Read data from all FSRs and append to the string:
// note that this is just reading the raw values, will need to calibrate after
// note that A0 and A3 are unassigned, will just be 0s
//  for (int ii = 0; ii < 1; ii++) {
//    int sensor = analogRead(PIN_FSR_ARR[ii]);
//    dataString += String(sensor);
//    if (ii < 1) {
//      dataString += ",";
//    }
//  }

// if the file is available, write to it:
//  if (dataFile) {
//    dataFile.println(dataString);  // prints to the file
//    Serial.println(dataString);    // also print to serial port for testing
//    dataFile.close();
//  }
//  else {
//    Serial.println("error opening datalog.txt");
//  }

}
