#include "Datalog.hpp"

Datalog::Datalog()
{
  this->stream = NULL;
  this->stream_enabled = false;
  this->sd_card_file = NULL;
  this->sd_card_enabled = false;
}

void Datalog::set_stream(Stream* s)
{
  this->stream = s;
  this->stream_enabled = true;
}

void Datalog::set_sd_card_file(File* f)
{
  this->sd_card_file = f;
  this->sd_card_enabled = true;
}

void Datalog::print(String data)
{
  if(this->stream_enabled)
    this->stream->print(data);
  if(this->sd_card_enabled && this->sd_card_file)
  {
    this->sd_card_file->print(data);
    if(data.length() > 0 && data[data.length() - 1] == '\n')
      this->sd_card_file->flush();
  }
}

void Datalog::println(String data)
{
  if(this->stream_enabled)
    this->stream->println(data);
  if(this->sd_card_enabled && this->sd_card_file)
  {
    this->sd_card_file->println(data);
    this->sd_card_file->flush();
  }
}

void Datalog::print(char* data)
{
  if(this->stream_enabled)
    this->stream->print(data);
  if(this->sd_card_enabled && this->sd_card_file)
  {
    this->sd_card_file->print(data);
    if(strlen(data) > 0 && data[strlen(data)-1] == '\n')
      this->sd_card_file->flush();
  }
}

void Datalog::println(char* data)
{
  if(this->stream_enabled)
    this->stream->println(data);
  if(this->sd_card_enabled && this->sd_card_file)
  {
    this->sd_card_file->println(data);
    this->sd_card_file->flush();
  }
}

Datalog datalog;
