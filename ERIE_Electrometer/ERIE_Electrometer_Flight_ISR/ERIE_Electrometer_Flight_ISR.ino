// Define for setting register bits
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif

// Define for clearing register bits
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |=  _BV(bit))
#endif

#include <SD.h>

//#define DEBUG
#define PRINT
#define RESET
//#define SINGLE_CHANNEL 0
//#define LISTVOL

Sd2Card card;
SdVolume volume;
SdFile root;
File file;
char filename[10] = {0};

uint16_t fileindex = 0xFFFF;

volatile unsigned long adc_count = 0;
volatile unsigned long adc_samps = 0;
volatile unsigned long adc_value;

volatile float adc_mean_old, adc_mean_new;
volatile unsigned long adc_vari_old, adc_vari_new;

volatile bool timer_flag = false;

byte channel = 0xFF;

// NOTE: MUX_DELAY + ADC_DELAY MUST BE SHORTER THAN TIMER 1 CLOCK
//#define MUX_DELAY 103
#define MUX_DELAY 50     // Time to wait after switching mux address
#define ADC_DELAY 25     // Time over which to average ADC readings
#define RST_DELAY 500    // Time to wait after enabling reset signal before disabling reset signal
#define RST_START 10000  // Elapsed time from start of program to begin reset cycle
#define RST_COOLD 500    // Time to wait after disabling reset signal before changing mux address
#define RST_SPACE 500    // Time between channel resets to take data before next reset

// MUX_A_PIN  8 // LSB; Arduino pin  8; electrometer pin 3
// MUX_B_PIN  9 //      Arduino pin  9; electrometer pin 4
// MUX_C_PIN 10 // MSB; Arduino pin 10; electrometer pin 5
#define MUX_MASK  B00000111 // Base PORTB bit mask (do not change) (defaults to Arduino pins 8,  9, 10)
#define MUX_SHIFT 0         // Left shift from B00000111 (shift of 1 would give Arduino pins 9, 10, 11)

#define RESET_PIN 12 // Arduino pin 12; electrometer pin 6
#define SIGNL_PIN A0 // Arduino pin A0; electrometer pin 7

#define PIN_MOSI  11 // Arduino pin 11; SD board pin 4
#define PIN_MISO  12 // Arduino pin 12; SD board pin 3
#define PIN_SCK   13 // Arduino pin 13; SD board pin 5
#define PIN_CS    A3 // Arduino pin A3; SD board pin 6

bool sd_enable = true;

void switchChannel(byte c)
{
  // Only take the three least significant bits of the argument (range 0-7)
  c &= MUX_MASK;
  // Update the three bits of port B defined by MUX_MASK and MUX_SHIFT
  PORTB = (PORTB & ~(MUX_MASK << MUX_SHIFT)) | (c << MUX_SHIFT);
  // Record the current channel in the global variable
  channel = c;
}

void setup()
{
  
  Serial.begin(115200);
  while(!Serial);
  Serial.println();

  Serial.print("Initializing SD card ... ");
  
  if(!card.init(SPI_HALF_SPEED, PIN_CS))
  {
    Serial.println("fail!");
    sd_enable = false;
  }
  else
  {
    Serial.println("done!");
  }
  Serial.println();

  if(sd_enable)
  {
    Serial.println();
    Serial.print("Card type:         ");
    switch(card.type())
    {
      case SD_CARD_TYPE_SD1:  Serial.println("SD1");     break;
      case SD_CARD_TYPE_SD2:  Serial.println("SD2");     break;
      case SD_CARD_TYPE_SDHC: Serial.println("SDHC");    break;
      default:                Serial.println("Unknown"); break;
    }

    if(!volume.init(card))
    {
      Serial.println("Could not find FAT16/FAT32 partition.");
      Serial.println("Make sure you've formatted the card");
      sd_enable = false;
    }
  }

  if(sd_enable)
  {
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
  }

  #ifdef LISTVOL
    if(sd_enable)
    {
      Serial.println("Volume file listing (name and size in bytes):");
      root.openRoot(volume);
      root.ls(LS_R | LS_SIZE);
      root.close();
      Serial.println();
    }
  #endif

  if(sd_enable)
  {
    Serial.print("Initializing SD card ... ");
    if(!SD.begin(PIN_CS))
    {
      Serial.println("fail!");
      sd_enable = false;
    }
    else
    {
      Serial.println("done!");
    }
    Serial.println();
  }

  if(sd_enable)
  {
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
    Serial.println();
  }

  // Set the direction of port B mux address lines to output
  DDRB |= (MUX_MASK << MUX_SHIFT);
  pinMode(RESET_PIN, OUTPUT);
  digitalWrite(RESET_PIN, LOW);

  #ifdef SINGLE_CHANNEL
    channel = SINGLE_CHANNEL;
  #else
    channel = 0;
  #endif
  switchChannel(channel);

  sbi(ADMUX,  REFS0);     // Set ADC reference to AVCC
  sbi(ADCSRA, ADEN);      // Enable ADC
  sbi(ADCSRA, ADATE);     // Enable auto-triggering

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
  OCR1A = 7811;  // 125 msec
  //OCR1A = 6249;  // 100 msec

  // Enable timer compare match interrupt
  sbi(TIMSK1, OCIE1B);

  sei();                  // Enable Global Interrupts

  Serial.print("Taking data");
  #ifdef PRINT
    Serial.print(" and writing to serial port");
  #endif
  if(sd_enable)
  {
    Serial.print(" and writing to SD card");
  }
  Serial.println(" ...");
  Serial.println();

}

byte resetChannel = 0;
bool resetComplete[8] = {false};
bool resetEnabled = false;
bool resetCooldown = false;
unsigned long resetTime = RST_START;

#ifdef DEBUG
  unsigned long proc_prev  = micros();
#endif

void loop()
{

  if(timer_flag)
  {
    timer_flag = false;

    #ifdef DEBUG
      unsigned long proc_start = micros();
    #endif

    #ifndef SINGLE_CHANNEL
      if(!resetEnabled and !resetCooldown)
      {
        channel = (channel + 1) % 8;
        switchChannel(channel);
        
        // Wait for multiplexer to settle
        delay(MUX_DELAY);
      }
    #endif

    #ifdef RESET
      if(resetChannel == channel && !resetComplete[resetChannel])
      {
        if(!resetCooldown && !resetEnabled && millis() > resetTime)
        {
          digitalWrite(RESET_PIN, HIGH);
          resetEnabled = true;
          resetTime = millis();
          String buffer = String(channel) + String(", ") + millis() + String(", RESET START");
          #ifdef PRINT
            Serial.println(buffer);
          #endif
          file.println(buffer);
        }
        else if(!resetCooldown && resetEnabled && millis() > resetTime + RST_DELAY)
        {
          digitalWrite(RESET_PIN, LOW);
          resetEnabled = false;
          resetCooldown = true;
          resetTime = millis();
          String buffer = String(channel) + String(", ") + millis() + String(", RESET STOP");
          #ifdef PRINT
            Serial.println(buffer);
          #endif
          file.println(buffer);
        }
        else if(resetCooldown && millis() > resetTime + RST_COOLD)
        {
          resetCooldown = false;
          resetComplete[channel] = true;
          resetTime = millis() + RST_SPACE;
          resetChannel = (resetChannel + 1) % 8;
          String buffer = String(channel) + String(", ") + millis() + String(", RESET COOL");
          #ifdef PRINT
            Serial.println(buffer);
          #endif
          file.println(buffer);
        }
      }
    #endif

    adc_samps = 0;     // Re-initialize ADC sample counter
    adc_count = 0;     // Re-initialize ADC count accumulator
    adc_mean_old = 0.;
    adc_mean_new = 0.;
    adc_vari_old = 0;
    adc_vari_new = 0;

    sbi(ADCSRA, ADIE); // Enable ADC interrupts
    sbi(ADCSRA, ADSC); // Start ADC conversions
  
    delay(ADC_DELAY);

    cbi(ADCSRA, ADSC); // Stop ADC conversions
    cbi(ADCSRA, ADIE); // Disable ADC interrupts

    String buffer = String(channel) + String(", ") + millis() + String(", ") + adc_count + String(", ") + adc_samps + String(", ") + String(adc_mean_new, 3) + String(", ") + adc_vari_new + String(", ") + String(adc_vari_new/float(adc_samps-1), 3);
    #ifdef PRINT
      Serial.println(buffer);
    #endif
    file.println(buffer);

    file.flush();

    #ifdef DEBUG
      unsigned long proc_stop = micros();

      Serial.print(proc_start-proc_prev, DEC);
      Serial.print(", ");
      Serial.print(proc_start, DEC);
      Serial.print(", ");
      Serial.print(proc_stop, DEC);
      Serial.print(", ");
      Serial.println(proc_stop-proc_start, DEC);
      proc_prev = proc_start;
    #endif

  }

}

void push_adc_value(unsigned long adc_value)
{
  adc_samps++;

  // https://www.johndcook.com/blog/standard_deviation/
  // See Knuth TAOCP vol 2, 3rd edition, page 232
  if (adc_samps == 1)
  {
    adc_mean_old = adc_value;
    adc_mean_new = adc_value;
    adc_vari_old = 0.0;
  }
  else
  {
    adc_mean_new = adc_mean_old + (adc_value - adc_mean_old)/adc_samps;
    // NOTE: Approximations are being made here for the sake of speed!
    //       Casting means to integers will introduce rounding errors!
    adc_vari_new = adc_vari_old + (adc_value - int(adc_mean_old))*(adc_value - int(adc_mean_new));
    
    // set up for next iteration
    adc_mean_old = adc_mean_new;
    adc_vari_old = adc_vari_new;
  }
}

// ADC interrupt
ISR(ADC_vect)
{
  // Make certain to read ADCL first as it locks the value and ADCH releases it
  adc_value = ADCL | (ADCH << 8);

  // Accumulate the ADC values
  adc_count += adc_value;
  push_adc_value(adc_value);
}

// Timer 1 interrupt
ISR(TIMER1_COMPB_vect) {
  timer_flag = true;
}
