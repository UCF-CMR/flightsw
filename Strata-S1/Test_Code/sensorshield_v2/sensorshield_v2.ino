/*
Code to record data for the Strata-S1 experiment.

Full code will read from all sensors in the experiment tubes. 

In the current iteration, the Strata-S1 experiment consists of:
Tube A: Strata-sphere -- no sensors
Tube B: Load Cell only
Tube C: Load Cell, FSR1, FSR2
Tube D: Load Cell, FSR1, FSR2

All tubes also have an Entrapulator - code for controlling those is separate.

This code will take data the enitre time the experiment is powered, so should run the whole flight.

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
#define  LC_B_CLK            10    // Tube B Load Cell CLK                                    
#define  LC_B_DOUT            9    // Tube B Load Cell DATA OUT
#define  chipSelect           8    // SDcard chipSelect

#define  LC_D_CLK             7    // Tube D Load Cell CLK                     
#define  LC_D_DOUT            6    // Tube D Load Cell DATA OUT
#define  LC_C_CLK             5    // Tube C Load Cell CLK                                           
#define  LC_C_DOUT            4    // Tube C Load Cell DATA OUT              
//#define  "X"                  3    // Not used.  
//#define  "X"                  2    // Not used.   
//#define  "X"                  1    // Not used.
//#define  "X"                  0    // Not used.

////////////////////////////////////////////////////////////////////////////////////////
//       Analog In 6
////////////////////////////////////////////////////////////////////////////////////////
//#define  FSR_B1               A0   // Tube B FSR 1 data - not used
#define  FSR_D2               A1   // Tube D FSR 2
#define  FSR_C2               A2   // Tube C FSR 2
//#define  FSR_C2               A3   // Tube C FSR 2  - not used
#define  FSR_D1               A4   // Tube D FSR 1                                
#define  FSR_C1               A5   // Tube C FSR 1                              


////////////////////////////////////////////////////////////////////////////////////////
//       Command Routine Variables and Include files
////////////////////////////////////////////////////////////////////////////////////////

#include <HX711.h>
#include <SPI.h>
#include <SD.h>

// LOAD CELLS
// Need to calibrate the Load Cells before flight to get this value
#define calfactor_LC_B -7050.0 //This value is obtained using the SparkFun_HX711_Calibration sketch
#define calfactor_LC_C -7050.0 //This value is obtained using the SparkFun_HX711_Calibration sketch
#define calfactor_LC_D -7050.0 //This value is obtained using the SparkFun_HX711_Calibration sketch

HX711 loadcell_B;
HX711 loadcell_C;
HX711 loadcell_D;

// FSRs
// Measure the voltage at 5V and resistance of your 3.3k resistor, and enter
// their value's below:
const float VCC = 4.98; // Measured voltage of Ardunio 5V line
const float R_DIV = 3230.0; // Measured resistance of 3.3k resistor

// Set up file to save data
File initFile;
File dataFile;

void setup() {
// Open serial port
  Serial.begin(9600);
//  while(!Serial){
     ; // wait for serial port to connect. Needed for native USB port only
//  }

// ***Set up SD card***
  Serial.print("Initializing SD card...");

  if (!SD.begin(chipSelect)) {
    Serial.println("Card initialization failed!");
    // don't do anything more
    while (1);
  }
  Serial.println("Card initialized.");

// ** Open a file to write the initilization values. **
// Note that only one file can be open at a time, so you have to close this one before opening another.  Serial.println(dataString);
  initFile = SD.open("initial.txt",  FILE_WRITE);  

// ***LOAD CELLS***
  Serial.println("Load Cell Setup");

// Set up load cell in Tube B  
  loadcell_B.begin(LC_B_DOUT, LC_B_CLK);
  loadcell_B.set_scale(calfactor_LC_B); //This value is obtained by using the SparkFun_HX711_Calibration sketch
  loadcell_B.tare(); //Scales the values to the fully compressed state.
  Serial.print("Tare value  LC B:");
  Serial.println(loadcell_B.get_units(), 1);
  initFile.print("Tare value  LC B:");
  initFile.println(loadcell_B.get_units(), 1);

// Set up load cell in Tube C  
  loadcell_C.begin(LC_C_DOUT, LC_C_CLK);
  loadcell_C.set_scale(calfactor_LC_C); //This value is obtained by using the SparkFun_HX711_Calibration sketch
  loadcell_C.tare(); //Scales the values to the fully compressed state.
  Serial.print("Tare value  LC C:");
  Serial.println(loadcell_C.get_units(), 1);
  initFile.print("Tare value  LC C:");
  initFile.println(loadcell_C.get_units(), 1);


// Set up load cell in Tube B  
  loadcell_D.begin(LC_D_DOUT, LC_D_CLK);
  loadcell_D.set_scale(calfactor_LC_D); //This value is obtained by using the SparkFun_HX711_Calibration sketch
  loadcell_D.tare(); //Scales the values to the fully compressed state.
  Serial.print("Tare value LC D:");
  Serial.println(loadcell_D.get_units(), 1);
  initFile.print("Tare value  LC D:");
  initFile.println(loadcell_D.get_units(), 1);


// ***FSRs***
// Set up FSRs in Tube B - not used
  Serial.println("FSR Setup");

//  pinMode(FSR_B1, INPUT);
//  pinMode(FSR_B2, INPUT);

// Set up FSRs in Tube C
  pinMode(FSR_C1, INPUT);
  pinMode(FSR_C2, INPUT);
  
// Set up FSRs in Tube D
  pinMode(FSR_D1, INPUT);
  pinMode(FSR_D2, INPUT);

// Close the initilization file on the SD card
  initFile.close();

// write that we've restarted the file to the SDcard
  dataFile = SD.open("datalog.txt",  FILE_WRITE);
  dataFile.println("New data");
  dataFile.pringln("LC_B,LC_C,LC_D,FSR_B1,FSR_C1,FSR_D1,FSR_B2,FSR_C2,FSR_D2");
  dataFile.close();
  
}

void loop() {    

// ** Open the data file each time through the loop **
// Note that only one file can be open at a time, so you have to close this one before opening another.  
  dataFile = SD.open("datalog.txt",  FILE_WRITE);  


// make a string in which we'll the data to log:
  String dataString = "";
// example dataString"valLC_B,valLC_C,valLC_D,valFSR_C1,valFSR_C2,valFSR_D1,valFSR_D2"
  
// Read data from all of the sensors and put into the data file
// --> --> Need to make this save the load cell data to the SD Card
// *** LOAD CELLS *** 
  long valLC_B = loadcell_B.read();                 //scale.read() gets values, here returns a float
  long valLC_C = loadcell_C.read();                 //scale.read() gets values, here returns a float
  long valLC_D = loadcell_D.read();                 //scale.read() gets values, here returns a float

// put into the datastring so that it can be written to the text file
  dataString += String(valLC_B);
  dataString += ",";
  dataString += String(valLC_C);
  dataString += ",";
  dataString += String(valLC_D);
  dataString += ",";

// ***FSRs***
// Read data from all FSRs and append to the string:
// note that this is just reading the raw values, will need to calibrate after
// note that A0 and A3 are unassigned, will just be 0s
  for (int analogPin = 0; analogPin < 6; analogPin++) {
    int sensor = analogRead(analogPin);
    dataString += String(sensor);
    if (analogPin < 5) {
      dataString += ",";
    }
  }

// if the file is available, write to it:
  if (dataFile) {
    dataFile.println(dataString);
    Serial.println(dataString);    // also print to serial port for testing
    dataFile.close();
  }
  else {
    Serial.println("error opening datalog.txt");
  }

}
