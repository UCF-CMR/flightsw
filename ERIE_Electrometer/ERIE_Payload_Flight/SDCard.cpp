#include "SDCard.hpp"

SDCard::SDCard()
{
  this->pin = 0xFF;
  this->fileindex = 0xFFFF;
  this->enabled = false;
}

void SDCard::start(uint8_t p)
{

  this->pin = p;
  Serial.print(millis());
  Serial.print(F(": Initializing SD card ... "));
  this->enabled = SD.begin(this->pin);
  Serial.println(this->enabled ? F("done!") : F("fail!"));

  if(this->enabled)
  {

    do
    {

      this->fileindex++;
      sprintf(this->filename, "%05d.txt", this->fileindex);

    }
    while(SD.exists(this->filename));

    Serial.print(millis());
    Serial.print(F(": Next available file is "));
    Serial.println(this->filename);

  }

}

void SDCard::create_file()
{

  if(this->enabled)
  {

    Serial.print(millis());
    Serial.print(F(": Creating file "));
    Serial.print(this->filename);
    Serial.print(" ... ");
    this->file = SD.open(this->filename, O_WRITE | O_CREAT); // Faster than FILE_WRITE
    this->enabled = (bool)this->file;
    Serial.println(this->enabled ? F("done!") : F("fail!"));

  }

}

File* SDCard::get_file()
{
  return &this->file;
}

void SDCard::stop()
{

  if(this->file)
  {

    this->file.close();
    this->enabled = false;

  }

}

bool SDCard::is_enabled()
{

  return this->enabled;

}

SDCard sdcard;
