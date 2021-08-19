#ifndef STEPPER_HPP
#define STEPPER_HPP

#include <Arduino.h>

class Stepper
{
  private:
    volatile unsigned long stepper_start_time = 0;
    volatile unsigned long stepper_stop_time = 0;
    volatile unsigned long stepper_pulse_count = 0;
    volatile bool stepper_complete_flag = false;
    volatile bool stepper_enabled = false;
    bool stepper_pulse_pin_state = false;
    bool stepper_direction_pin_state = false;
    bool stepper_enable_pin_state = false;
    uint8_t stepper_pin_cp;
    uint8_t stepper_pin_dir;
    uint8_t stepper_pin_en;
  public:
    Stepper();
    void update_pulse_pin_state();
    void reset_pulse_pin_state();
    void set_pulse_pin_state(bool state);
    void toggle_pulse_pin_state();
    void set_direction_pin_state(bool dir);
    bool get_direction_pin_state();
    void set_enable_pin_state(bool en);
    bool get_enable_pin_state();
    void set_enabled(bool en);
    volatile bool get_enabled();
    void set_complete_flag(bool flag);
    volatile bool get_complete_flag();
    volatile unsigned long get_pulse_count();
    volatile unsigned long get_start_time();
    volatile unsigned long get_stop_time();
    void init(uint8_t pin_en, uint8_t pin_dir, uint8_t pin_cp);
    void start();
    void stop();
};

extern Stepper stepper;

#endif
