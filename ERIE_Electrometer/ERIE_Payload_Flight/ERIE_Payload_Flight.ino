#define PIN_DI_TRIGGER             3 // Digital input from experiment trigger (active high)

#define PIN_DO_STEP_CP             4 // Digital output to stepper pulse pin (rising edge)
#define PIN_DO_STEP_DIR            5 // Digital output to stepper direction pin (high to open, low to close)
#define PIN_DO_STEP_ENABLE        A1 // Digital output to stepper enable pin (active high)

#define PIN_DO_12V_REG_ENABLE     A2 // Digital output to 12V regulator enable pin (active high) [LEDs/electrometer]
#define PIN_DO_HV_REG_ENABLE       6 // Digital output to high voltage regulator enable pin (active high)

#define PIN_DO_SD_CARD_MOSI       11 // Digital output  to microSD board pin 4 (SPI MOSI) [for reference, not used]
#define PIN_DI_SD_CARD_MISO       12 // Digital input from microSD board pin 3 (SPI MISO) [for reference, not used]
#define PIN_DO_SD_CARD_SCK        13 // Digital output  to microSD board pin 5 (SPI SCK)  [for reference, not used]
#define PIN_DO_SD_CARD_CS         A3 // Digital output  to microSD board pin 6 (SPI CS)

#define PIN_AI_EM_SIGNAL          A0 // Analog input from electrometer signal on pin 7 (0-5V, 2.5V zero) [default, not used]
#define PIN_DO_EM_RESET            7 // Digital output to electrometer reset on pin 6 (active high)
#define PIN_DO_EM_MUX_A            8 // Digital output to electrometer mux addr A on pin 3 (active high; LSB) [must match mask]
#define PIN_DO_EM_MUX_B            9 // Digital output to electrometer mux addr B on pin 4 (active high)      [must match mask]
#define PIN_DO_EM_MUX_C           10 // Digital output to electrometer mux addr C on pin 5 (active high; MSB) [must match mask]
#define PIN_DO_EM_MUX_MASK      0x07 // Base PORTB bit mask (do not change) (defaults to Arduino pins 8,  9, 10) [must match pin defs]
#define PIN_DO_EM_MUX_SHIFT        0 // Left shift from B00000111 (shift of 1 would give Arduino pins 9, 10, 11) [must match pin defs]
#define PIN_DO_EM_ADC              2 // Digital output for debugging electrometer ADC measurements [comment out to remove debug signal]

#define STEPPER_DUTY_CYCLE        50 // Duty cycle for stepper pulse waveform (0 - 100)
#define STEPPER_PULSES         21500 // Total number of stepper pulses in single direction (4x microstep)
#define MEASURE_DELAY           3000 // Minimum delay after trigger to take measurements before starting
#define TRIGGER_DELAY           1000 // Delay between two subsequent trigger state measurements
#define REGULATOR_DELAY          500 // Delay before/after changing state of regulator
#define SETTLE_DELAY           15000 // Delay after enabling high voltage prior to starting stepper
#define CORONA_DELAY           10000 // Delay between door cycles to observe corona in darkness
#define STEPPER_OPENED_DELAY    2000 // Delay after stepper has opened
#define STEPPER_CLOSED_DELAY    2000 // Delay after stepper has closed
#define STEPPER_INTERIM_DELAY   5000 // Delay before stepper opens during interim

#define STEPPER_MOD                5 // Defines number of Timer1 rollovers after which to pulse stepper
#define MEASURE_MOD              500 // Defines number of Timer1 rollovers after which to increment measurement channel
#define MEASURE_PAD              100 // Defines margin of Timer1 rollovers when ADC is not measuring (2*MEASURE_PAD < MEASURE_MOD)

#define STEPPER_DUTY_HIGH (STEPPER_MOD * STEPPER_DUTY_CYCLE) / 100

#include "Datalog.hpp"
#include "Stepper.hpp"
#include "Electrometer.hpp"

volatile uint8_t door_cycle_count = 0;
volatile unsigned int timer_pulse_count = 0;

void start_timer()
{

  // Clear all interrupts
  cli();

  // Reset Timer1 registers
  TCCR1A = 0x00;
  TCCR1B = 0x00;
  TCNT1  = 0x0000;

  // Load Timer1 compare match register
  // 16MHz/prescaler/2*period+1; for 500us period, load 4001
  // NOTE: may need to compensate for ISR computation time
  OCR1A = 4001;

  // Enable Timer1 clear timer on compare match (CTC) mode
  TCCR1B |= (1 << WGM12);

  // Set prescaler for timer with B001 for no prescaler
  // and B010:8, B011:64, B100:256, B101:1024
  TCCR1B |= (0 << CS12) | (0 << CS11) | (1 << CS10);

  // Enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);

  // Enable all interrupts
  sei();

}

void stop_timer()
{

  // Disable Timer1 interrupts
  TIMSK1 = 0x00;

}

typedef enum
{

  STATE_DELAY,
  STATE_INITIALIZE,
  STATE_COAST_TRIGGER_WAIT,
  STATE_COAST_TRIGGER_VERIFY,
  STATE_12V_REG_ENABLE,
  STATE_12V_REG_DISABLE,
  STATE_HV_REG_ENABLE,
  STATE_HV_REG_DISABLE,
  STATE_ELECTROMETER_ENABLE,
  STATE_ELECTROMETER_DISABLE,
  STATE_STEPPER_RUN_OPENED,
  STATE_STEPPER_RUN_CLOSED,
  STATE_STEPPER_RUN,
  STATE_CHUTE_TRIGGER_WAIT,
  STATE_CHUTE_TRIGGER_VERIFY,
  STATE_TERMINATE

} states_t;
states_t state = STATE_INITIALIZE;
states_t state_next = STATE_INITIALIZE;
unsigned long state_time = 0;
unsigned long state_delay = 0;

void state_transition(int new_state)
{

  state_transition(new_state, 0);

}

void state_transition(int new_state, unsigned long delay_time)
{

  datalog.print(F("Transitioning to state "));

  if(delay_time > 0)
  {

    state_delay = delay_time;
    state_next = new_state;
    new_state = STATE_DELAY;

  }

  switch(new_state)
  {

    case STATE_DELAY:
      datalog.print(F("DELAY ["));
      datalog.print(String(delay_time));
      datalog.println(F("]"));
      break;
    case STATE_INITIALIZE:              datalog.println(F("INITIALIZE"));              break;
    case STATE_12V_REG_ENABLE:          datalog.println(F("12V_REG_ENABLE"));          break;
    case STATE_ELECTROMETER_ENABLE:     datalog.println(F("ELECTROMETER_ENABLE"));     break;
    case STATE_COAST_TRIGGER_WAIT:      datalog.println(F("COAST_TRIGGER_WAIT"));      break;
    case STATE_COAST_TRIGGER_VERIFY:    datalog.println(F("COAST_TRIGGER_VERIFY"));    break;
    case STATE_HV_REG_ENABLE:           datalog.println(F("HV_REG_ENABLE"));           break;
    case STATE_STEPPER_RUN_OPENED:      datalog.println(F("STEPPER_RUN_OPENED"));      break;
    case STATE_STEPPER_RUN_CLOSED:      datalog.println(F("STEPPER_RUN_CLOSED"));      break;
    case STATE_STEPPER_RUN:             datalog.println(F("STEPPER_RUN"));             break;
    case STATE_HV_REG_DISABLE:          datalog.println(F("HV_REG_DISABLE"));          break;
    case STATE_CHUTE_TRIGGER_WAIT:      datalog.println(F("CHUTE_TRIGGER_WAIT"));      break;
    case STATE_CHUTE_TRIGGER_VERIFY:    datalog.println(F("CHUTE_TRIGGER_VERIFY"));    break;
    case STATE_ELECTROMETER_DISABLE:    datalog.println(F("ELECTROMETER_DISABLE"));    break;
    case STATE_12V_REG_DISABLE:         datalog.println(F("12V_REG_DISABLE"));         break;
    case STATE_TERMINATE:               datalog.println(F("TERMINATE"));               break;
    default:                            datalog.println(F("UNKNOWN"));                 break;

  }

  state = new_state;
  state_time = millis();

}

void print_stepper_status()
{

  datalog.print(F("Stepper executed "));
  datalog.print(String(stepper.get_pulse_count()));
  datalog.print(F(" steps over "));
  datalog.print(String(stepper.get_stop_time() - stepper.get_start_time()));
  datalog.print(F(" ms ("));
  datalog.print(String(STEPPER_DUTY_HIGH));
  datalog.print(F("/"));
  datalog.print(String(STEPPER_MOD));
  datalog.println(F(" duty)"));

}

void setup()
{

  Serial.begin(115200);
  datalog.set_stream(&Serial);
  Serial.println(F("\n\nStarting state machine\n\n"));

  pinMode(PIN_DI_TRIGGER, INPUT);

  pinMode(PIN_DO_STEP_CP,     OUTPUT);
  pinMode(PIN_DO_STEP_DIR,    OUTPUT);
  pinMode(PIN_DO_STEP_ENABLE, OUTPUT);

  digitalWrite(PIN_DO_STEP_CP,     LOW);
  digitalWrite(PIN_DO_STEP_DIR,    LOW);
  digitalWrite(PIN_DO_STEP_ENABLE, LOW);

  pinMode(PIN_DO_HV_REG_ENABLE,  OUTPUT);
  pinMode(PIN_DO_12V_REG_ENABLE, OUTPUT);

  digitalWrite(PIN_DO_HV_REG_ENABLE,  LOW);
  digitalWrite(PIN_DO_12V_REG_ENABLE, LOW);

  pinMode(PIN_DO_SD_CARD_MOSI, OUTPUT);
  pinMode(PIN_DI_SD_CARD_MISO, INPUT);
  pinMode(PIN_DO_SD_CARD_SCK,  OUTPUT);
  pinMode(PIN_DO_SD_CARD_CS,   OUTPUT);

  digitalWrite(PIN_DO_SD_CARD_MOSI, HIGH);
  digitalWrite(PIN_DO_SD_CARD_SCK,  HIGH);
  digitalWrite(PIN_DO_SD_CARD_CS,   HIGH);

  pinMode(PIN_DO_EM_RESET, OUTPUT);
  pinMode(PIN_DO_EM_MUX_A, OUTPUT);
  pinMode(PIN_DO_EM_MUX_B, OUTPUT);
  pinMode(PIN_DO_EM_MUX_C, OUTPUT);

  digitalWrite(PIN_DO_EM_RESET, LOW);
  digitalWrite(PIN_DO_EM_MUX_A, LOW);
  digitalWrite(PIN_DO_EM_MUX_B, LOW);
  digitalWrite(PIN_DO_EM_MUX_C, LOW);

  #ifdef PIN_DO_EM_ADC
    pinMode(PIN_DO_EM_ADC, OUTPUT);
    digitalWrite(PIN_DO_EM_ADC, LOW);
  #endif

  electrometer.adc_init();
  stepper.init(PIN_DO_STEP_ENABLE, PIN_DO_STEP_DIR, PIN_DO_STEP_CP);
  sdcard.start(PIN_DO_SD_CARD_CS);
  sdcard.create_file();
  datalog.set_sd_card_file(sdcard.get_file());

  state_transition(STATE_INITIALIZE);


}

void loop()
{

  switch(state)
  {

    case STATE_DELAY:
      if(millis() - state_time > state_delay)
      {

        state_transition(state_next);

      }
      break;

    case STATE_INITIALIZE:
      start_timer();
      state_transition(STATE_12V_REG_ENABLE);
      break;

    case STATE_12V_REG_ENABLE:
      digitalWrite(PIN_DO_12V_REG_ENABLE, HIGH);
      state_transition(STATE_ELECTROMETER_ENABLE, REGULATOR_DELAY);
      break;

    case STATE_ELECTROMETER_ENABLE:
      electrometer.adc_reset();
      electrometer.set_channel(0xFF);
      electrometer.set_enabled(true);
      electrometer.set_running(false);

      // If door already cycled once, start next door cycle
      if(door_cycle_count >= 1)
      {

        state_transition(STATE_STEPPER_RUN_OPENED, STEPPER_INTERIM_DELAY);

      }
      // otherwise wait for microgravity coast start trigger
      else
      {

        state_transition(STATE_COAST_TRIGGER_WAIT, MEASURE_DELAY);

      }
      break;

    case STATE_COAST_TRIGGER_WAIT:
      if(digitalRead(PIN_DI_TRIGGER) == HIGH)
      {

        state_transition(STATE_COAST_TRIGGER_VERIFY, TRIGGER_DELAY);

      }
      break;

    case STATE_COAST_TRIGGER_VERIFY:
      if(digitalRead(PIN_DI_TRIGGER) == HIGH)
      {

        state_transition(STATE_HV_REG_ENABLE, TRIGGER_DELAY);

      }
      else
      {

        state_transition(STATE_COAST_TRIGGER_WAIT);

      }
      break;

    case STATE_HV_REG_ENABLE:
      digitalWrite(PIN_DO_HV_REG_ENABLE, HIGH);
      state_transition(STATE_STEPPER_RUN_OPENED, SETTLE_DELAY);
      break;

    case STATE_STEPPER_RUN_OPENED:
      stepper.set_direction_pin_state(true);
      state_transition(STATE_STEPPER_RUN);
      break;

    case STATE_STEPPER_RUN_CLOSED:
      stepper.set_direction_pin_state(false);
      state_transition(STATE_STEPPER_RUN);
      break;

    case STATE_STEPPER_RUN:
      // If stepper is already running,
      if(stepper.get_enabled() || stepper.get_enable_pin_state())
      {

        // check if it has completed moving, then
        if(stepper.get_complete_flag())
        {

          // print out status, reset,
          print_stepper_status();
          stepper.set_enable_pin_state(false);
          stepper.set_complete_flag(false);

          // and, if opened, transition to closing
          if(stepper.get_direction_pin_state())
          {
            state_transition(STATE_STEPPER_RUN_CLOSED, STEPPER_OPENED_DELAY);
          }
          // but, if closed,
          else
          {
            // increment door cycle count
            door_cycle_count++;
            // and if already cycled twice, start shutting things down
            if(door_cycle_count >= 2)
            {
              state_transition(STATE_HV_REG_DISABLE, TRIGGER_DELAY);
            }
            // otherwise, perform actions that occur between door cycles
            else
            {
              state_transition(STATE_ELECTROMETER_DISABLE, STEPPER_CLOSED_DELAY);
            }
          }
        }
      }
      // If stepper has not been started yet
      else
      {
        // start stepper moving in opened direction
        datalog.println(F("Running stepper in opened direction"));
        stepper.set_enabled(true);
        stepper.set_enable_pin_state(true);
        stepper.start();
      }
      break;

    case STATE_HV_REG_DISABLE:
      digitalWrite(PIN_DO_HV_REG_ENABLE, LOW);
      state_transition(STATE_CHUTE_TRIGGER_WAIT, MEASURE_DELAY);
      break;

    case STATE_CHUTE_TRIGGER_WAIT:
      if(digitalRead(PIN_DI_TRIGGER) == HIGH)
      {

        state_transition(STATE_CHUTE_TRIGGER_VERIFY, TRIGGER_DELAY);

      }
      break;

    case STATE_CHUTE_TRIGGER_VERIFY:
      if(digitalRead(PIN_DI_TRIGGER) == HIGH)
      {

        state_transition(STATE_ELECTROMETER_DISABLE, TRIGGER_DELAY);

      }
      else
      {

        state_transition(STATE_CHUTE_TRIGGER_WAIT);

      }
      break;

    case STATE_ELECTROMETER_DISABLE:
      electrometer.set_enabled(false);
      state_transition(STATE_12V_REG_DISABLE, REGULATOR_DELAY);
      break;

    case STATE_12V_REG_DISABLE:
      digitalWrite(PIN_DO_12V_REG_ENABLE, LOW);

      // If door already cycled twice, continue shutting things down
      if(door_cycle_count >= 2)
      {

        state_transition(STATE_TERMINATE);

      }
      // Otherwise, start regulator again after observation delay
      else
      {

        state_transition(STATE_12V_REG_ENABLE, CORONA_DELAY);

      }
      break;

    case STATE_TERMINATE:
      stop_timer();
      sdcard.stop();
      break;

    default:
      state_transition(STATE_INITIALIZE);
      break;

  }

  if(electrometer.get_running() && electrometer.get_adc_data_ready())
  {

    datalog.print(String(electrometer.get_adc_time_diff()));
    datalog.print(F(", "));
    datalog.print(String(electrometer.get_channel_buf()));
    datalog.print(F(", "));
    datalog.print(String(electrometer.get_adc_time_buf()));
    datalog.print(F(", "));
    datalog.print(String(electrometer.get_adc_count_buf()));
    datalog.print(F(", "));
    datalog.print(String(electrometer.get_adc_samps_buf()));
    datalog.print(F(", "));
    datalog.print(String(electrometer.get_adc_mean_buf(), 3));
    datalog.print(F(", "));
    datalog.print(String(electrometer.get_adc_vari_buf()));
    datalog.print(F(", "));
    datalog.print(String(electrometer.get_adc_vari_buf()/float(electrometer.get_adc_samps_buf()-1), 3));

    if(electrometer.get_adc_data_error())
    {

      datalog.print(F(" [ERROR]"));
      electrometer.set_adc_data_error(false);

    }

    datalog.println(F(""));

    electrometer.set_adc_data_ready(false);

  }

}

// ADC interrupt
ISR(ADC_vect)
{

  // Make certain to read ADCL first as it locks the value and ADCH releases it
  electrometer.set_adc_value(ADCL | (ADCH << 8));

  // Accumulate the ADC values
  electrometer.set_adc_count(electrometer.get_adc_count() + electrometer.get_adc_value());
  electrometer.push_adc_value(electrometer.get_adc_value());

}

// Timer1 interrupt
ISR(TIMER1_COMPA_vect)
{

  timer_pulse_count = ++timer_pulse_count % (STEPPER_MOD*MEASURE_MOD);

  if(stepper.get_enabled())
  {

    switch(timer_pulse_count % STEPPER_MOD)
    {

      case 0:
        // Create rising edge for stepper
        stepper.set_pulse_pin_state(true);
        break;

      case STEPPER_DUTY_HIGH:
        // Create falling edge based on duty cycle
        stepper.set_pulse_pin_state(false);

        if(stepper.get_pulse_count() >= STEPPER_PULSES)
        {

          stepper.stop();

        }
        break;

    }

  }

  if((timer_pulse_count % MEASURE_MOD) == 0)
  {

    // Synchronize electrometer state changes
    electrometer.set_running(electrometer.get_enabled());

  }

  if(electrometer.get_running())
  {

    switch(timer_pulse_count % MEASURE_MOD)
    {

      case 0:
        electrometer.set_channel((electrometer.get_channel()+1) & PIN_DO_EM_MUX_MASK);
        PORTB = (PORTB & ~(PIN_DO_EM_MUX_MASK << PIN_DO_EM_MUX_SHIFT)) | (electrometer.get_channel() << PIN_DO_EM_MUX_SHIFT);
        break;

      // Clock cycle to enable ADC
      // Should be smaller than next case
      // Must be less than MEASURE_MOD
      case MEASURE_PAD:
        #ifdef PIN_DO_EM_ADC
          digitalWrite(PIN_DO_EM_ADC, HIGH);
        #endif
        electrometer.adc_start();
        break;

      // Clock cycle to disable ADC
      // Should be larger than previous case
      // Must be less than MEASURE_MOD
      case MEASURE_MOD-MEASURE_PAD:
        electrometer.adc_stop();
        #ifdef PIN_DO_EM_ADC
          digitalWrite(PIN_DO_EM_ADC, LOW);
        #endif
        break;

    }

  }

}
