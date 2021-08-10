#define PIN_DO_STEP_CP          4    // Digital output to stepper pulse pin (rising edge)
#define PIN_DO_STEP_DIR         5    // Digital output to stepper direction pin (high to open, low to close)
#define PIN_DO_STEP_ENABLE     A1    // Digital output to stepper enable pin (active high)
#define STEPPER_PULSES       3000    // Total number of stepper pulses in single direction
#define STEPPER_DELAY        5000    // Delay time in microseconds between directions

volatile unsigned int timer_pulse_count = 0;

volatile unsigned long stepper_start_time = 0;
volatile unsigned int stepper_pulse_current = 0;
volatile bool stepper_pulse_state = false;
volatile bool stepper_complete_flag = false;
volatile bool stepper_enable = false;

#define STEPPER_MOD 1

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
  TCNT1  = 0x0000;

  // Load Timer1 compare match register
  // 16MHz/prescaler/2*period+1; for 5ms period, load 40001
  // NOTE: may need to compensate for ISR computation time
  OCR1A = 40093;

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

void setup()
{

  Serial.begin(115200);

  pinMode(PIN_DO_STEP_CP,     OUTPUT);
  pinMode(PIN_DO_STEP_DIR,    OUTPUT);
  pinMode(PIN_DO_STEP_ENABLE, OUTPUT);

  digitalWrite(PIN_DO_STEP_CP,     LOW);
  digitalWrite(PIN_DO_STEP_DIR,    LOW);
  digitalWrite(PIN_DO_STEP_ENABLE, LOW);

  start_timer();

  digitalWrite(PIN_DO_STEP_ENABLE, HIGH);
  Serial.println();
  Serial.println("Running stepper in opened direction ...");
  digitalWrite(PIN_DO_STEP_DIR, HIGH);
  start_stepper();
  while(!stepper_complete_flag);
  digitalWrite(PIN_DO_STEP_ENABLE, LOW);

  delay(STEPPER_DELAY);

  digitalWrite(PIN_DO_STEP_ENABLE, HIGH);
  Serial.println();
  Serial.println("Running stepper in closed direction ...");
  digitalWrite(PIN_DO_STEP_DIR, LOW);
  start_stepper();
  while(!stepper_complete_flag);
  digitalWrite(PIN_DO_STEP_ENABLE, LOW);

  stop_timer();

}

void loop()
{

}

// Timer1 interrupt
ISR(TIMER1_COMPA_vect)
{

  timer_pulse_count = ++timer_pulse_count % STEPPER_MOD;

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
        if(stepper_pulse_current >= STEPPER_PULSES)
          stop_stepper();
      }
    }
  }

}
