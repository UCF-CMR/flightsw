#define PIN_DO_STEP_CP          4    // Digital output to stepper pulse pin (rising edge)
#define PIN_DO_STEP_DIR         5    // Digital output to stepper direction pin (high to open, low to close)

#define PIN_DO_EM_MUX_A         8    // Digital output to electrometer mux addr A on pin 3 (active high; LSB)
#define PIN_DO_EM_MUX_B         9    // Digital output to electrometer mux addr B on pin 4 (active high)
#define PIN_DO_EM_MUX_C        10    // Digital output to electrometer mux addr C on pin 5 (active high; MSB)

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

volatile bool electrometer_enable = false;
volatile bool electrometer_mux_state[3] = {false, false, false};

// Clock divider relative to stepper frequency
#define MUX_C_TIMER_MOD 50
#define MUX_B_TIMER_MOD (2*MUX_C_TIMER_MOD)
#define MUX_A_TIMER_MOD (4*MUX_C_TIMER_MOD)

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

  // Update stepper start time
  stepper_start_time = millis();

}

void start_timer()
{

  // Clear all interrupts
  cli();

  // Reset Timer2 registers
  TCCR2A = 0x00;
  TCCR2B = 0x00;
  TCNT2  = 0x00;

  // Load Timer2 compare match register
  // 16MHz/prescaler/2*period-1; for 5ms period, load 155.25
  // NOTE: may need to compensate for ISR computation time
  OCR2A = 155;

  // Enable Timer2 clear timer on compare match (CTC) mode
  TCCR2A |= (1 << WGM21);

  // Set prescaler for timer with B001 for no prescaler and
  // B010:8, B011:32, B100:64, B101:128, B110:256, B111:1024
  TCCR2B |= (1 << CS22) | (1 << CS21) | (0 << CS20);

  // Enable timer compare interrupt
  TIMSK2 |= (1 << OCIE2A);

  // Enable all interrupts
  sei();

}

void start_stepper()
{

  // Reset stepper pulse state
  reset_stepper_pulse();

  // Indicate stepper is not finished
  stepper_complete_flag = false;

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

  // Disable Timer2 interrupts
  TIMSK2 = 0x00;

}

void setup()
{

  Serial.begin(115200);

  pinMode(PIN_DO_STEP_CP,  OUTPUT);
  pinMode(PIN_DO_STEP_DIR, OUTPUT);

  digitalWrite(PIN_DO_STEP_CP,  LOW);
  digitalWrite(PIN_DO_STEP_DIR, LOW);

  pinMode(PIN_DO_EM_MUX_A, OUTPUT);
  pinMode(PIN_DO_EM_MUX_B, OUTPUT);
  pinMode(PIN_DO_EM_MUX_C, OUTPUT);

  digitalWrite(PIN_DO_EM_MUX_A, LOW);
  digitalWrite(PIN_DO_EM_MUX_B, LOW);
  digitalWrite(PIN_DO_EM_MUX_C, LOW);

  start_timer();
  electrometer_enable = true;

  delay(MEASURE_DELAY);

  Serial.println();
  Serial.println("Running stepper in opened direction ...");
  digitalWrite(PIN_DO_STEP_DIR, HIGH);
  start_stepper();
  while(!stepper_complete_flag);

  delay(STEPPER_DELAY);

  Serial.println();
  Serial.println("Running stepper in closed direction ...");
  digitalWrite(PIN_DO_STEP_DIR, LOW);
  start_stepper();
  while(!stepper_complete_flag);

  delay(MEASURE_DELAY);

  electrometer_enable = false;
  stop_timer();

}

void loop()
{

}

ISR(TIMER2_COMPA_vect)
{

  timer_pulse_count++;

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
    if((timer_pulse_count % MUX_C_TIMER_MOD) == 0)
      electrometer_mux_state[0] ^= true;
    if((timer_pulse_count % MUX_B_TIMER_MOD) == 0)
      electrometer_mux_state[1] ^= true;
    if((timer_pulse_count % MUX_A_TIMER_MOD) == 0)
      electrometer_mux_state[2] ^= true;
    PORTB = (PORTB & ~B00000111) | electrometer_mux_state[0] << 2 | electrometer_mux_state[1] << 1 | electrometer_mux_state[2];
  }
}
