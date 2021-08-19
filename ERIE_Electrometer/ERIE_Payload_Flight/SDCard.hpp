#include <SD.h>

class SDCard
{
  private:
    uint8_t pin = 0xFF;
    unsigned int fileindex = 0xFFFF;
    char filename[10] = {0};
    bool enabled = false;
    File file;
  public:
    SDCard(void);
    void start(uint8_t pin);
    void create_file(void);
    void stop(void);
    bool is_enabled(void);
};

extern SDCard sdcard;
