// Define for setting register bits
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif

// Define for clearing register bits
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |=  _BV(bit))
#endif

#include <SD.h>

#define CHIPSELECT A3

//#define LISTVOL

Sd2Card card;
SdVolume volume;
SdFile root;
File file;
char filename[10] = {0};

uint16_t fileindex = 0xFFFF;

volatile bool timerFlag = false;

byte channel = 0xFF;
unsigned long prevTime;
unsigned long currTime;

void setup() {
  Serial.begin(115200);
  while(!Serial);

  Serial.println();

  Serial.print("Initializing SD card ... ");
  
  if (!card.init(SPI_HALF_SPEED, CHIPSELECT)) {
    Serial.println("fail!");
    while (1);
  } else {
    Serial.println("done!");
  }

  Serial.println();
  Serial.print("Card type:         ");
  switch (card.type()) {
    case SD_CARD_TYPE_SD1:  Serial.println("SD1");     break;
    case SD_CARD_TYPE_SD2:  Serial.println("SD2");     break;
    case SD_CARD_TYPE_SDHC: Serial.println("SDHC");    break;
    default:                Serial.println("Unknown"); break;
  }

  if (!volume.init(card)) {
    Serial.println("Could not find FAT16/FAT32 partition.");
    Serial.println("Make sure you've formatted the card");
    while (1);
  }

  Serial.print("Clusters:          ");
  Serial.println(volume.clusterCount());
  Serial.print("Blocks x Cluster:  ");
  Serial.println(volume.blocksPerCluster());
  Serial.print("Total Blocks:      ");
  Serial.println(volume.blocksPerCluster() * volume.clusterCount());
  Serial.println();

  Serial.print("Volume type is:    FAT");
  Serial.println(volume.fatType(), DEC);
  uint32_t volumesize;
  volumesize  = volume.blocksPerCluster();
  volumesize *= volume.clusterCount();
  volumesize /= 2;
  Serial.print("Volume size (KB):  ");
  Serial.println(volumesize);
  Serial.print("Volume size (MB):  ");
  volumesize /= 1024;
  Serial.println(volumesize);
  Serial.print("Volume size (GB):  ");
  Serial.println((float)volumesize / 1024.0);
  Serial.println();

  #ifdef LISTVOL
    Serial.println("Volume file listing (name and size in bytes):");
    root.openRoot(volume);
    root.ls(LS_R | LS_SIZE);
    root.close();
    Serial.println();
  #endif

  Serial.print("Initializing SD card ... ");
  if(!SD.begin(CHIPSELECT))
  {
    Serial.println("fail!");
    while(1);
  }
  Serial.println("done!");
  Serial.println();

  do
  {
    fileindex++;
    sprintf(filename, "%05d.txt", fileindex);
  }
  while(SD.exists(filename));

  Serial.print("Creating file ");
  Serial.print(filename);
  Serial.println(" ...");
  file = SD.open(filename, O_WRITE | O_CREAT); // Faster than FILE_WRITE

  // Clear timer configuration registers
  TCCR1A = 0;
  TCCR1B = 0;

  // Set up timer in CTC mode with prescaler of 256
  sbi(TCCR1B, WGM12);
  sbi(TCCR1B, CS12);

  // Initialize timer counter and compare values
  TCNT1 = 0;
  //OCR1A = 62499; // 1 sec
  //OCR1A = 15624; // 250 msec
  OCR1A = 6249;  // 100 msec

  // Enable timer compare match interrupt
  sbi(TIMSK1, OCIE1B);

  sei();                  // Enable Global Interrupts

  prevTime = millis();

}

void loop()
{

  if(timerFlag)
  {
    timerFlag = false;
    currTime = millis();
    Serial.print(currTime);
    Serial.print(", ");
    Serial.println(currTime-prevTime);
    file.print(currTime);
    file.print(",");
    file.print(channel++ % 8);
    file.print(",");
    file.println(currTime*channel % 0xFFFF);
    prevTime = currTime;
  }
  else
  {
    if(millis() >= 10000)
    {
      // Clear interrupts
      cli();
      file.close();
    }
  }

}

// Timer 1 interrupt
ISR(TIMER1_COMPB_vect) {
  timerFlag = true;
}
