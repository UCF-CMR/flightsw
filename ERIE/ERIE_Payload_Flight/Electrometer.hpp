#ifndef ELECTROMETER_HPP
#define ELECTROMETER_HPP

#include <Arduino.h>

class Electrometer
{
  private:
    volatile bool enabled = false;
    volatile bool running = false;
    volatile uint8_t channel = 0xFF;
    volatile uint8_t channel_buf = 0xFF;

    volatile unsigned long adc_count = 0, adc_count_buf = 0;
    volatile unsigned long adc_samps = 0, adc_samps_buf = 0;
    volatile unsigned long adc_value = 0, adc_value_buf = 0;

    volatile float adc_mean_old = 0., adc_mean_new = 0., adc_mean_buf = 0.;

    volatile unsigned long adc_vari_old = 0, adc_vari_new = 0, adc_vari_buf = 0;
    volatile unsigned long adc_time_old = 0, adc_time_new = 0, adc_time_buf = 0;

    volatile bool adc_data_ready = false;
    volatile bool adc_data_error = false;

  public:
    void adc_reset();
    void adc_init();
    void adc_start();
    void adc_stop();

    void push_adc_value(unsigned long adc_value);

    volatile bool get_enabled();
    void set_enabled(bool enabled);

    volatile bool get_running();
    void set_running(bool running);

    volatile unsigned long get_adc_count();
    volatile unsigned long get_adc_count_buf();
    void set_adc_count(unsigned long adc_count);

    volatile unsigned long get_adc_samps();
    volatile unsigned long get_adc_samps_buf();
    void set_adc_samps(unsigned long adc_samps);

    volatile unsigned long get_adc_value();
    volatile unsigned long get_adc_value_buf();
    void set_adc_value(unsigned long adc_value);

    volatile float get_adc_mean_buf();
    volatile unsigned long get_adc_vari_buf();
    volatile unsigned long get_adc_time_buf();
    volatile unsigned long get_adc_time_diff();

    volatile bool get_adc_data_ready();
    void set_adc_data_ready(bool adc_data_ready);

    volatile bool get_adc_data_error();
    void set_adc_data_error(bool adc_data_error);

    volatile uint8_t get_channel();
    volatile uint8_t get_channel_buf();
    void set_channel(uint8_t channel);
};

extern Electrometer electrometer;

#endif
