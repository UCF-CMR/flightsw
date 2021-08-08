// Define for setting register bits
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif

// Define for clearing register bits
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |=  _BV(bit))
#endif

#define PIN_DO_STEP_CP          4    // Digital output to stepper pulse pin (rising edge)
#define PIN_DO_STEP_DIR         5    // Digital output to stepper direction pin (high to open, low to close)

#define PIN_DO_EM_MUX_A         8    // Digital output to electrometer mux addr A on pin 3 (active high; LSB)
#define PIN_DO_EM_MUX_B         9    // Digital output to electrometer mux addr B on pin 4 (active high)
#define PIN_DO_EM_MUX_C        10    // Digital output to electrometer mux addr C on pin 5 (active high; MSB)

#define PIN_DO_EM_ADC           2

#define STEPPER_PULSES_TOTAL 3000    // Total number of stepper pulses in single direction
#define STEPPER_DELAY        5000    // Delay time in microseconds between directions

#define MEASURE_DELAY        3000    // Delay between measurements and stepper operation

unsigned int timer_pulse_count = 0;

unsigned int stepper_start_time = 0;
volatile unsigned int stepper_pulse_current = 0;
volatile bool stepper_pulse_state = false;
volatile bool stepper_complete_flag = false;
volatile bool stepper_enable = false;

#define STEPPER_MOD 1
#define MEASURE_MOD 50

volatile bool electrometer_enable = false;
volatile uint8_t electrometer_channel = B000;
volatile uint8_t electrometer_channel_buf = B000;

volatile unsigned long adc_count = 0;
volatile unsigned long adc_samps = 0;
volatile unsigned long adc_value = 0;

volatile unsigned long adc_count_buf = 0;
volatile unsigned long adc_samps_buf = 0;
volatile unsigned long adc_value_buf = 0;

volatile float adc_mean_old = 0.;
volatile float adc_mean_new = 0.;
volatile float adc_mean_buf = 0.;

volatile unsigned long adc_vari_old = 0;
volatile unsigned long adc_vari_new = 0;
volatile unsigned long adc_vari_buf = 0;

volatile unsigned long adc_time_old = 0;
volatile unsigned long adc_time_new = 0;
volatile unsigned long adc_time_buf = 0;

volatile bool adc_data_ready = false;
volatile bool adc_data_error = false;

void update_stepper_pulse()
{

  // Output current stepper state to pulse pin
  digitalWrite(PIN_DO_STEP_CP, stepper_pulse_state ? HIGH : LOW);

}

void reset_stepper_pulse()
{

  // Initialize volatile variables
  stepper_pulse_current = 0;
  stepper_pulse_state = false;

  // Update stepper pulse state
  update_stepper_pulse();

}

void start_timer()
{

  // Clear all interrupts
  cli();

  // Reset Timer1 registers
  TCCR1A = 0x00;
  TCCR1B = 0x00;
  TCNT1 = 0x0000;

  // Load Timer1 compare match register
  // 16MHz/prescaler/2*period+1; for 5ms period, load 40001
  // NOTE: may need to compensate for ISR computation time
  OCR1A = 40093;

  // Enable Timer1 clear timer on compare match (CTC) mode
  TCCR1B |= (1 << WGM12);

  // Set prescaler for timer with B001 for no prescaler and
  // B010:8, B011:64, B100:64, B100:256, B101:1024
  TCCR1B |= (0 << CS12) | (0 << CS11) | (1 << CS10);

  // Enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);

  // Enable all interrupts
  sei();

}

void start_stepper()
{

  // Reset stepper pulse state
  reset_stepper_pulse();

  // Indicate stepper is not finished
  stepper_complete_flag = false;

  // Update stepper start time
  stepper_start_time = millis();

  stepper_enable = true;

}

void stop_stepper()
{

  stepper_enable = false;

  // Display stepper status
  Serial.print("Stepper executed ");
  Serial.print(stepper_pulse_current);
  Serial.print(" steps over ");
  Serial.print(millis() - stepper_start_time);
  Serial.println(" ms");

  // Reset stepper pulse state
  reset_stepper_pulse();

  // Indicate stepper is finished
  stepper_complete_flag = true;

}

void stop_timer()
{

  // Disable Timer1 interrupts
  TIMSK1 = 0x00;

}

typedef enum
{
  STATE_INITIALIZE,
  STATE_ELECTROMETER_START,
  STATE_ELECTROMETER_STOP,
  STATE_STEPPER_RUN_OPENED,
  STATE_STEPPER_RUN_CLOSED,
  STATE_IDLE,
  STATE_TERMINATE
} states_t;

typedef enum
{
  DOOR_OPENING,
  DOOR_OPENED,
  DOOR_CLOSING,
  DOOR_CLOSED
} door_states_t;

states_t state = STATE_INITIALIZE;
unsigned long state_time = 0;

door_states_t door_state = DOOR_CLOSED;
unsigned long door_state_time = 0;

void state_transition(int new_state)
{
  Serial.print("Transitioning to state ");
  switch(new_state)
  {
    case STATE_INITIALIZE:         Serial.println("INITIALIZE");         break;
    case STATE_ELECTROMETER_START: Serial.println("ELECTROMETER_START"); break;
    case STATE_ELECTROMETER_STOP:  Serial.println("ELECTROMETER_STOP");  break;
    case STATE_STEPPER_RUN_OPENED: Serial.println("STEPPER_RUN_OPENED"); break;
    case STATE_STEPPER_RUN_CLOSED: Serial.println("STEPPER_RUN_CLOSED"); break;
    case STATE_IDLE:               Serial.println("IDLE");               break;
    case STATE_TERMINATE:          Serial.println("TERMINATE");          break;
    default:                       Serial.println("UNKNOWN");            break;
  }
  state = new_state;
  state_time = millis();
}

void door_state_transition(int new_state)
{
  Serial.print("Transitioning to door state ");
  switch(new_state)
  {
    case DOOR_OPENING: Serial.println("OPENING"); break;
    case DOOR_OPENED:  Serial.println("OPENED");  break;
    case DOOR_CLOSING: Serial.println("CLOSING"); break;
    case DOOR_CLOSED:  Serial.println("CLOSED");  break;
    default:           Serial.println("UNKNOWN"); break;
  }
  door_state = new_state;
  door_state_time = millis();
}

void setup()
{

  Serial.begin(115200);
  Serial.println("Starting state machine");

  pinMode(PIN_DO_STEP_CP,  OUTPUT);
  pinMode(PIN_DO_STEP_DIR, OUTPUT);

  digitalWrite(PIN_DO_STEP_CP,  LOW);
  digitalWrite(PIN_DO_STEP_DIR, LOW);

  pinMode(PIN_DO_EM_MUX_A, OUTPUT);
  pinMode(PIN_DO_EM_MUX_B, OUTPUT);
  pinMode(PIN_DO_EM_MUX_C, OUTPUT);
  pinMode(PIN_DO_EM_ADC,   OUTPUT);

  digitalWrite(PIN_DO_EM_MUX_A, LOW);
  digitalWrite(PIN_DO_EM_MUX_B, LOW);
  digitalWrite(PIN_DO_EM_MUX_C, LOW);
  digitalWrite(PIN_DO_EM_ADC,   LOW);

  sbi(ADMUX,  REFS0);     // Set ADC reference to AVCC
  sbi(ADCSRA, ADEN);      // Enable ADC
  sbi(ADCSRA, ADATE);     // Enable auto-triggering

  state_transition(STATE_INITIALIZE);

}

void loop()
{

  switch(state)
  {

    case STATE_INITIALIZE:
      start_timer();
      door_state_transition(DOOR_CLOSED);
      state_transition(STATE_ELECTROMETER_START);
      break;

    case STATE_ELECTROMETER_START:
      electrometer_enable = true;
      if(millis() - door_state_time > MEASURE_DELAY)
      {
        state_transition(STATE_STEPPER_RUN_OPENED);
      }
      break;


    case STATE_ELECTROMETER_STOP:
      electrometer_enable = false;
      state_transition(STATE_IDLE);
      break;

    case STATE_STEPPER_RUN_OPENED:
      Serial.println("Running stepper in opened direction");
      digitalWrite(PIN_DO_STEP_DIR, HIGH);
      start_stepper();
      door_state_transition(DOOR_OPENING);
      state_transition(STATE_IDLE);
      break;

    case STATE_STEPPER_RUN_CLOSED:
      Serial.println("Running stepper in closed direction");
      digitalWrite(PIN_DO_STEP_DIR, LOW);
      start_stepper();
      door_state_transition(DOOR_CLOSING);
      state_transition(STATE_IDLE);
      break;

    case STATE_IDLE:
      switch(door_state)
      {
        case DOOR_OPENING:
          if(stepper_complete_flag)
          {
            door_state_transition(DOOR_OPENED);
            stepper_complete_flag = false;
          }
          break;
        case DOOR_CLOSING:
          if(stepper_complete_flag)
          {
            door_state_transition(DOOR_CLOSED);
            stepper_complete_flag = false;
          }
          break;
        case DOOR_OPENED:
          if(millis() - door_state_time > STEPPER_DELAY)
          {
            state_transition(STATE_STEPPER_RUN_CLOSED);
          }
          break;
        case DOOR_CLOSED:
          if(electrometer_enable && (millis() - door_state_time > MEASURE_DELAY))
          {
            state_transition(STATE_ELECTROMETER_STOP);
          }
          break;
        default:
          break;
      }
      break;

    case STATE_TERMINATE:
      electrometer_enable = false;
      stop_timer();
      break;

    default:
      state_transition(STATE_INITIALIZE);
      break;
  }

  if(electrometer_enable && adc_data_ready)
  {

    String buffer = String(adc_time_new - adc_time_old) + String(", ");
    buffer += String(electrometer_channel_buf) + String(", ") + String(adc_time_buf) + String(", ") + adc_count_buf + String(", ") + adc_samps_buf + String(", ") + String(adc_mean_buf, 3) + String(", ") + adc_vari_buf + String(", ") + String(adc_vari_buf/float(adc_samps_buf-1), 3);
    if(adc_data_error)
    {
      buffer += String(" [ERROR]");
      adc_data_error = false;
    }
    Serial.println(buffer);
    adc_data_ready = false;
  }

}

void push_adc_value(unsigned long adc_value)
{
  adc_samps++;

  // https://www.johndcook.com/blog/standard_deviation/
  // See Knuth TAOCP vol 2, 3rd edition, page 232
  if (adc_samps == 1)
  {
    adc_mean_old = adc_value;
    adc_mean_new = adc_value;
    adc_vari_old = 0.0;
  }
  else
  {
    adc_mean_new = adc_mean_old + (adc_value - adc_mean_old)/adc_samps;
    // NOTE: Approximations are being made here for the sake of speed!
    //       Casting means to integers will introduce rounding errors!
    adc_vari_new = adc_vari_old + (adc_value - int(adc_mean_old))*(adc_value - int(adc_mean_new));

    // set up for next iteration
    adc_mean_old = adc_mean_new;
    adc_vari_old = adc_vari_new;
  }
}

// ADC interrupt
ISR(ADC_vect)
{
  // Make certain to read ADCL first as it locks the value and ADCH releases it
  adc_value = ADCL | (ADCH << 8);

  // Accumulate the ADC values
  adc_count += adc_value;
  push_adc_value(adc_value);
}

// Timer1 interrupt
ISR(TIMER1_COMPA_vect)
{

  timer_pulse_count = ++timer_pulse_count % (STEPPER_MOD*MEASURE_MOD);

  if(stepper_enable)
  {
    if((timer_pulse_count % STEPPER_MOD) == 0)
    {
      // Invert pulse output pin
      stepper_pulse_state ^= true;
      update_stepper_pulse();

      // Only count rising edges as valid steps
      if(stepper_pulse_state)
      {
        stepper_pulse_current++;
        if(stepper_pulse_current >= STEPPER_PULSES_TOTAL)
          stop_stepper();
      }
    }
  }

  if(electrometer_enable)
  {
    switch(timer_pulse_count % MEASURE_MOD)
    {
      case 0:
        electrometer_channel = ++electrometer_channel & B00000111;
        PORTB = (PORTB & ~B00000111) | electrometer_channel;
        break;

      // Clock cycle to enable ADC
      // Should be smaller than next case
      // Must be less than MEASURE_MOD
      case 10:
        digitalWrite(PIN_DO_EM_ADC, HIGH);

        adc_samps = 0;     // Re-initialize ADC sample counter
        adc_count = 0;     // Re-initialize ADC count accumulator
        adc_mean_old = 0.;
        adc_mean_new = 0.;
        adc_vari_old = 0;
        adc_vari_new = 0;

        sbi(ADCSRA, ADIE); // Enable ADC interrupts
        sbi(ADCSRA, ADSC); // Start ADC conversions

        break;

      // Clock cycle to disable ADC
      // Should be larger than previous case
      // Must be less than MEASURE_MOD
      case 40:
        cbi(ADCSRA, ADSC); // Stop ADC conversions
        cbi(ADCSRA, ADIE); // Disable ADC interrupts

        if(adc_data_ready)
        {
          adc_data_error = true;
        }

        adc_time_old = adc_time_new;
        adc_time_new = micros();
        adc_time_buf = millis();

        electrometer_channel_buf = electrometer_channel;

        adc_samps_buf = adc_samps;
        adc_count_buf = adc_count;

        adc_mean_buf = adc_mean_new;
        adc_vari_buf = adc_vari_new;

        adc_data_ready = true;

        digitalWrite(PIN_DO_EM_ADC, LOW);

        break;
    }

  }
}
