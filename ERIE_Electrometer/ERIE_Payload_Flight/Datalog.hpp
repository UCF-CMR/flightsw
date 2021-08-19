#include <Stream.h>
#include "SDCard.hpp"

class Datalog
{
  private:
    Stream* stream;
    bool stream_enabled;
    File* sd_card_file;
    bool sd_card_enabled;
  public:
    Datalog(void);
    void set_stream(Stream* s);
    void set_sd_card_file(File* f);
    void print(String data);
    void println(String data);
    void print(char* data);
    void println(char* data);
};

extern Datalog datalog;
