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
    this->sd_card_file->print(data);
}

void Datalog::println(String data)
{
  if(this->stream_enabled)
    this->stream->println(data);
  if(this->sd_card_enabled && this->sd_card_file)
    this->sd_card_file->println(data);
}

void Datalog::print(char* data)
{
  if(this->stream_enabled)
    this->stream->print(data);
  if(this->sd_card_enabled && this->sd_card_file)
    this->sd_card_file->print(data);
}

void Datalog::println(char* data)
{
  if(this->stream_enabled)
    this->stream->println(data);
  if(this->sd_card_enabled && this->sd_card_file)
    this->sd_card_file->println(data);
}

Datalog datalog;
