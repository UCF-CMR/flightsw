#include "Electrometer.hpp"

void Electrometer::adc_reset()
{

  this->adc_count = 0; this->adc_count_buf = 0;
  this->adc_samps = 0; this->adc_samps_buf = 0;
  this->adc_value = 0; this->adc_value_buf = 0;

  this->adc_mean_old = 0.; this->adc_mean_new = 0.; this->adc_mean_buf = 0.;
  this->adc_vari_old = 0 ; this->adc_vari_new = 0 ; this->adc_vari_buf = 0 ;
  this->adc_time_old = 0 ; this->adc_time_new = 0 ; this->adc_time_buf = 0 ;

  this->set_adc_data_ready(false);
  this->set_adc_data_error(false);

}

void Electrometer::adc_init()
{

  // Reset ADC registers
  ADMUX  = 0x00;
  ADCSRA = 0x00;

  // Set ADC reference to AVCC and default to A0
  ADMUX  |= (1 << REFS0);

  // Enable ADC and auto-triggering
  ADCSRA |= (1 << ADEN) | (1 << ADATE);

  // Set ADC prescaler with B000: 2, B001: 2, B010: 4, B011:  8
  //                        B100:16, B101:32, B110:64, B111:128
  ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);

}

void Electrometer::adc_start()
{

  // Re-initialize ADC variables
  this->adc_samps = 0;
  this->adc_count = 0;
  this->adc_mean_old = 0.;
  this->adc_mean_new = 0.;
  this->adc_vari_old = 0;
  this->adc_vari_new = 0;

  // Enable ADC interrupts and start ADC conversions
  ADCSRA |=  (1 << ADIE) |  (1 << ADSC);

}

void Electrometer::adc_stop()
{

  // Stop ADC conversions and disable ADC interrupts
  ADCSRA &= ~(1 << ADSC) & ~(1 << ADIE);

  if(this->adc_data_ready) this->set_adc_data_error(true);

  this->channel_buf = this->channel;

  this->adc_time_old = this->adc_time_new;
  this->adc_time_new = micros();
  this->adc_time_buf = millis();

  this->adc_samps_buf = this->adc_samps;
  this->adc_count_buf = this->adc_count;

  this->adc_mean_buf = this->adc_mean_new;
  this->adc_vari_buf = this->adc_vari_new;

  this->set_adc_data_ready(true);

}

void Electrometer::push_adc_value(unsigned long adc_value)
{

  this->adc_samps++;

  // https://www.johndcook.com/blog/standard_deviation/
  // See Knuth TAOCP vol 2, 3rd edition, page 232
  if(this->adc_samps == 1)
  {

    this->adc_mean_old = this->adc_value;
    this->adc_mean_new = this->adc_value;
    this->adc_vari_old = 0.0;

  }
  else
  {

    this->adc_mean_new = this->adc_mean_old + (this->adc_value - this->adc_mean_old)/this->adc_samps;
    // NOTE: Approximations are being made here for the sake of speed!
    //       Casting means to integers will introduce rounding errors!
    this->adc_vari_new = this->adc_vari_old + (this->adc_value - int(this->adc_mean_old))*(this->adc_value - int(this->adc_mean_new));

    // set up for next iteration
    this->adc_mean_old = this->adc_mean_new;
    this->adc_vari_old = this->adc_vari_new;

  }

}

volatile bool Electrometer::get_enabled() { return this->enabled; }
void Electrometer::set_enabled(bool enabled) { this->enabled = enabled; }

volatile bool Electrometer::get_running() { return this->running; }
void Electrometer::set_running(bool running) { this->running = running; }

volatile unsigned long Electrometer::get_adc_count() { return this->adc_count; }
volatile unsigned long Electrometer::get_adc_count_buf() { return this->adc_count_buf; }
void Electrometer::set_adc_count(unsigned long adc_count) { this->adc_count = adc_count; }

volatile unsigned long Electrometer::get_adc_samps() { return this->adc_samps; }
volatile unsigned long Electrometer::get_adc_samps_buf() { return this->adc_samps_buf; }
void Electrometer::set_adc_samps(unsigned long adc_samps) { this->adc_samps = adc_samps; }

volatile unsigned long Electrometer::get_adc_value() { return this->adc_value; }
volatile unsigned long Electrometer::get_adc_value_buf() { return this->adc_value_buf; }
void Electrometer::set_adc_value(unsigned long adc_value) { this->adc_value = adc_value; }

volatile float Electrometer::get_adc_mean_buf() { return this->adc_mean_buf; }
volatile unsigned long Electrometer::get_adc_vari_buf() { return this->adc_vari_buf; }
volatile unsigned long Electrometer::get_adc_time_buf() { return this->adc_time_buf; }
volatile unsigned long Electrometer::get_adc_time_diff() { return this->adc_time_old ? this->adc_time_new - this->adc_time_old : 0; }

volatile bool Electrometer::get_adc_data_ready() { return this->adc_data_ready; }
void Electrometer::set_adc_data_ready(bool adc_data_ready) { this->adc_data_ready = adc_data_ready; }

volatile bool Electrometer::get_adc_data_error() { return this->adc_data_error; }
void Electrometer::set_adc_data_error(bool adc_data_error) { this->adc_data_error = adc_data_error; }

volatile uint8_t Electrometer::get_channel() { return this->channel; }
volatile uint8_t Electrometer::get_channel_buf() { return this->channel_buf; }
void Electrometer::set_channel(uint8_t channel) { this->channel = channel; }

Electrometer electrometer;
