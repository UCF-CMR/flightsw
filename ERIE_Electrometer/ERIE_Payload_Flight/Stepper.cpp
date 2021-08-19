#include "Stepper.hpp"

Stepper::Stepper()
{

}

void Stepper::update_pulse_pin_state()
{

  // Output current stepper state to pulse pin
  digitalWrite(this->stepper_pin_cp, this->stepper_pulse_pin_state ? HIGH : LOW);

}

void Stepper::reset_pulse_pin_state()
{

  // Initialize volatile variables
  this->stepper_pulse_count = 0;
  this->stepper_pulse_pin_state = false;

  // Update stepper pulse state
  this->update_pulse_pin_state();

}

void Stepper::set_pulse_pin_state(bool state)
{

  // Update stepper pulse state if needed
  if(this->stepper_pulse_pin_state != state)
  {

    // Only count rising edges as valid steps
    if(!this->stepper_pulse_pin_state && state)
      this->stepper_pulse_count++;

    // Set stepper pulse state
    this->stepper_pulse_pin_state = state;

    // Update stepper pulse state
    this->update_pulse_pin_state();

  }

}

void Stepper::toggle_pulse_pin_state()
{

  // Toggle stepper pulse state
  this->stepper_pulse_pin_state ^= true;

  this->set_pulse_pin_state(this->stepper_pulse_pin_state);

}

void Stepper::set_direction_pin_state(bool dir)
{

  this->stepper_direction_pin_state = dir;
  digitalWrite(this->stepper_pin_dir, dir ? HIGH : LOW);

}

bool Stepper::get_direction_pin_state() { return this->stepper_direction_pin_state; }

void Stepper::set_enable_pin_state(bool en)
{

  this->stepper_enable_pin_state = en;
  digitalWrite(this->stepper_pin_en, en ? HIGH : LOW);

}

bool Stepper::get_enable_pin_state() { return this->stepper_enable_pin_state; }

void Stepper::set_enabled(bool en) { this->stepper_enabled = en; }
volatile bool Stepper::get_enabled() {return this->stepper_enabled; }

void Stepper::set_complete_flag(bool flag) { this->stepper_complete_flag = flag; }
volatile bool Stepper::get_complete_flag() { return this->stepper_complete_flag; }

volatile unsigned long Stepper::get_pulse_count()
{

  return this->stepper_pulse_count;

}

volatile unsigned long Stepper::get_start_time()
{

  return this->stepper_start_time;

}

volatile unsigned long Stepper::get_stop_time()
{

  return this->stepper_stop_time;

}

void Stepper::init(uint8_t pin_en, uint8_t pin_dir, uint8_t pin_cp)
{

  // Define stepper enable, direction, and pulse pins
  this->stepper_pin_en  = pin_en;
  this->stepper_pin_dir = pin_dir;
  this->stepper_pin_cp  = pin_cp;

}

void Stepper::start()
{

  // Indicate stepper is not finished
  this->set_complete_flag(false);

  // Reset stepper pulse state
  this->reset_pulse_pin_state();

  // Enable stepper
  this->set_enabled(true);

  // Update stepper start time
  this->stepper_start_time = millis();

}

void Stepper::stop()
{

  // Update stepper stop time
  this->stepper_stop_time = millis();

  // Disable stepper
  this->set_enabled(false);

  // Drive pulse line low, but do not clear count
  this->set_pulse_pin_state(false);

  // Indicate stepper is finished
  this->set_complete_flag(true);

}

Stepper stepper;
