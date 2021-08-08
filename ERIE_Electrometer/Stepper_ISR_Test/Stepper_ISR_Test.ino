#define PIN_DO_5V_REG_ENABLE   A2    // Digital output to enable 5V regulator (active high) [stepper]

#define PIN_DO_STEP_CP          4    // Digital output to stepper pulse pin (rising edge)
#define PIN_DO_STEP_DIR         5    // Digital output to stepper direction pin (high to open, low to close)

#define STEPPER_PULSES       3000    // Total number of stepper pulses in single direction
#define STEPPER_DELAY        5000    // Delay time in microseconds between directions

unsigned int timer_pulse_count = 0;

unsigned int stepper_start_time = 0;
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

  pinMode(PIN_DO_5V_REG_ENABLE, OUTPUT);
  pinMode(PIN_DO_STEP_CP,       OUTPUT);
  pinMode(PIN_DO_STEP_DIR,      OUTPUT);

  digitalWrite(PIN_DO_5V_REG_ENABLE, LOW);
  digitalWrite(PIN_DO_STEP_CP,       LOW);
  digitalWrite(PIN_DO_STEP_DIR,      LOW);

  start_timer();

  digitalWrite(PIN_DO_5V_REG_ENABLE, HIGH);

  delay(STEPPER_DELAY);

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

  delay(STEPPER_DELAY);

  digitalWrite(PIN_DO_5V_REG_ENABLE, LOW);

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
        if(stepper_pulse_current >= STEPPER_PULSES)
          stop_stepper();
      }
    }
  }

}
