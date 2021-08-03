#define PIN_DI_TRIGGER         3    // Digital input to trigger experiment (active high)
#define PIN_DO_STEP_CP         4    // Digital output for stepper pulse (rising edge)
#define PIN_DO_STEP_DIR        5    // Digital output for stepper direction (high to open, low to close)
#define PIN_DO_LED_EM_ENABLE  A1    // Digital output to enable LEDs and electrometer (active high)
#define PIN_DO_5V_REG_ENABLE  A2    // Digital output to enable 5V regulator (active high)

// TODO: Edit this for full opening - 6750
#define DOOR_MOVE_STEPS 3050

// Pulse stepper by integer number of steps
// NOTE: Sum both delays to get the period of the waveform
void move_stepper(unsigned int steps)
{
  for(unsigned int i = 0; i < steps; i++)
  {
    digitalWrite(PIN_DO_STEP_CP, HIGH);
    delay(3);
    digitalWrite(PIN_DO_STEP_CP, LOW);
    delay(2);
  }
}

// Flash LEDs with time delay in milliseconds
// NOTE: Electrometer will be power cycled as well
void flash_leds(unsigned int time)
{
  digitalWrite(PIN_DO_LED_EM_ENABLE, LOW);
  delay(time);
  digitalWrite(PIN_DO_LED_EM_ENABLE, HIGH);
}

void setup()
{
  pinMode(PIN_DO_5V_REG_ENABLE, OUTPUT); digitalWrite(PIN_DO_5V_REG_ENABLE, LOW);
  pinMode(PIN_DO_LED_EM_ENABLE, OUTPUT); digitalWrite(PIN_DO_LED_EM_ENABLE, HIGH);
  pinMode(PIN_DO_STEP_CP,       OUTPUT); digitalWrite(PIN_DO_STEP_CP,       LOW);
  pinMode(PIN_DO_STEP_DIR,      OUTPUT); digitalWrite(PIN_DO_STEP_DIR,      LOW);
  pinMode(PIN_DI_TRIGGER,       INPUT);

  // TODO: Adjust this delay for flight
  delay(2000);
}

void loop()
{

  // Only start experiment when trigger signal is high
  if(digitalRead(PIN_DI_TRIGGER) == HIGH)
  {

    delay(1000);

    // Debounce the trigger input by checking a second time
    if(digitalRead(PIN_DI_TRIGGER) == HIGH)
    {

      delay(1000);

      // Flash lights to signal experiment start
      flash_leds(1500);

      // Enable microstep driver
      digitalWrite(PIN_DO_5V_REG_ENABLE, HIGH);

      // Wait 3 seconds
      delay(3000);

      // Open the tray door
      digitalWrite(PIN_DO_STEP_DIR, HIGH);
      move_stepper(DOOR_MOVE_STEPS);

      // Stay open 5 seconds
      delay(5000);

      // Close the tray door
      digitalWrite(PIN_DO_STEP_DIR, LOW);
      move_stepper(DOOR_MOVE_STEPS);

      // Wait 5 seconds
      delay(5000);

      // Disable microstep driver
      digitalWrite(PIN_DO_5V_REG_ENABLE, LOW);

      // Flash lights to signal experiment end
      flash_leds(1500);

      // TODO: Adjust this delay for flight
      //delay(3600000);
      delay(20000);

    }

  }

}
