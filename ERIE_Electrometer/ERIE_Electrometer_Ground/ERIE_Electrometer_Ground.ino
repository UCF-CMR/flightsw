byte channel = 0;

#define MUX_DELAY 103
#define RST_DELAY 500
#define RST_START 10000

// MUX_A_PIN  8 // LSB; Arduino pin  8; electrometer pin 3
// MUX_B_PIN  9 //      Arduino pin  9; electrometer pin 4
// MUX_C_PIN 10 // MSB; Arduino pin 10; electrometer pin 5
#define MUX_MASK  B00000111 // Base PORTB bit mask (do not change) (defaults to Arduino pins 8,  9, 10)
#define MUX_SHIFT 0         // Left shift from B00000111 (shift of 1 would give Arduino pins 9, 10, 11)

#define RESET_PIN 12 // Arduino pin 12; electrometer pin 6
#define SIGNL_PIN A0 // Arduino pin A0; electrometer pin 7

float analogReadScale()
{
  return 5.*analogRead(SIGNL_PIN)/1024.;
}

void switchChannel(byte c)
{
  // Only take the three least significant bits of the argument (range 0-7)
  c &= MUX_MASK;
  // Update the three bits of port B defined by MUX_MASK and MUX_SHIFT
  PORTB = (PORTB & ~(MUX_MASK << MUX_SHIFT)) | (c << MUX_SHIFT);
  // Record the current channel in the global variable
  channel = c;
  // Wait for multiplexer to settle
  delay(MUX_DELAY);
}

void resetChannel(byte c)
{
  if(c != channel) switchChannel(c);
  digitalWrite(RESET_PIN, HIGH);
  delay(RST_DELAY);
  digitalWrite(RESET_PIN, LOW);
}

float readChannelNTimes(byte c, byte n, unsigned int d)
{
  if(c != channel) switchChannel(c);
  float sum = 0.;
  for(byte i = 0; i < n; i++)
  {
    sum += readChannel(c);
    delay(d);
  }
  return sum/n;
}

float readChannel(byte c)
{
  if(c != channel) switchChannel(c);
  return analogReadScale();
}

void printTime()
{
  Serial.print(millis()/1000., 3);
}

void readAllChannels()
{
  printTime();
  for(byte c = 0; c < 8; c++)
  {
    Serial.print(", ");
    Serial.print(readChannel(c), 3);
  }
  Serial.println();
}

void readAllChannelsNTimes(byte n, unsigned int d)
{
  printTime();
  for(byte c = 0; c < 8; c++)
  {
    Serial.print(", ");
    Serial.print(readChannelNTimes(c, n, d), 3);
  }
  Serial.println();
}

void setup()
{
  Serial.begin(9600);
  // Set the direction of port B mux address lines to output
  DDRB |= (MUX_MASK << MUX_SHIFT);
  pinMode(RESET_PIN, OUTPUT);
  digitalWrite(RESET_PIN, LOW);
}

bool resetComplete = false;

void loop()
{
  if(!resetComplete and millis() > RST_START)
  {
    for(byte c = 0; c < 8; c++)
    {
      printTime();
      Serial.print(", ");
      Serial.print("RESET ");
      Serial.print(c);
      Serial.print(", ");
      if(c != channel) switchChannel(c);
      digitalWrite(RESET_PIN, HIGH);
      delay(RST_DELAY);
      Serial.print(readChannelNTimes(c, 10, 2), 3);
      Serial.print(", ");
      digitalWrite(RESET_PIN, LOW);
      delay(RST_DELAY);
      Serial.print(readChannelNTimes(c, 10, 2), 3);
      Serial.println();
    }
    resetComplete = true;
  }
  else
  {
    //readAllChannels();
    readAllChannelsNTimes(10, 2);
  }
}
