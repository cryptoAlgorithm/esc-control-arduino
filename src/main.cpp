#include <Arduino.h>

#include <Servo.h>

#define MAX_PERIOD 2000
#define MIN_PERIOD 1000

#define ESC_1_PIN  9
#define ESC_2_PIN  8
#define PSU_PIN    2

Servo esc1;
Servo esc2;

// PSU macros
#define PSU_ON() digitalWrite(PSU_PIN, 0); \
                 Serial.println("PSU ON"); \
                 delay(500) // PSU delay to wait for PSU to turn on
#define PSU_OFF() digitalWrite(PSU_PIN, 1); \
                 Serial.println("PSU OFF")

// Serial constants
#define SPEED_PROMPT F("Set interval (1000-2000us): ")

/**
 * Initialises ESCs with correct startup sequence
 * 
 * This calibrates the max and min period durations.
*/
void init_esc() {
  Serial.println("ESC init start");
  esc1.attach(ESC_1_PIN);
  esc2.attach(ESC_2_PIN);

  // Calibration sequence
  esc1.writeMicroseconds(MAX_PERIOD);
  esc2.writeMicroseconds(MAX_PERIOD);

  PSU_ON();
  Serial.println(F("Calibrate HIGH"));
  delay(2000); // ESC timeout
  Serial.println(F("Calibrate LOW"));
  esc1.writeMicroseconds(MIN_PERIOD);
  esc2.writeMicroseconds(MIN_PERIOD);

  Serial.println(F("ESC init done"));
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(PSU_PIN, OUTPUT);

  // Switch off PSU initially
  PSU_OFF();

  // Init serial
  Serial.begin(115200);
  Serial.println("Start!");

  // Set OC1A pins to outputs
  /*pinMode(9, OUTPUT); // OC1A = pin 9

  // Write timer1 config registers
  TCCR1A = _BV(COM1A0) | _BV(COM1B0) | _BV(WGM11) | _BV(WGM10);
  TCCR1B = _BV(CS11) | _BV(CS10);
  OCR1A = MAX_PERIOD;
  OCR1B = MIN_PERIOD;*/

  // Init ESC with startup sequence
  init_esc();

  // This line signals to hosts that the initialisation sequence has completed
  Serial.println(F("Ready"));

  Serial.print(SPEED_PROMPT);
}

void loop() {
  // Pull out all variables used in loop for better "memory management"
  int32_t rawRead;
  uint32_t interval;
  char motorID;
  while (Serial.available()) {
    rawRead = Serial.parseInt();
    if (rawRead < 0) {
      switch (rawRead) {
        case -1:
          // Emergency PSU off
          PSU_OFF();
          break;
        case -2:
          init_esc();
          break;
      }
      continue;
    }
    interval = constrain(rawRead, MIN_PERIOD, MAX_PERIOD);
    /* uint32_t interval = constrain(rawRead, MIN_PERIOD, MAX_PERIOD);
    Serial.print("Set interval to "); Serial.print(interval); Serial.println("");
    OCR1B = interval;*/
    esc1.writeMicroseconds(interval);
    esc2.writeMicroseconds(interval);
    Serial.print("Set interval to "); Serial.print(interval); Serial.println("us");
    // Clear the remaining data in the serial buffer
    while (Serial.available()) Serial.read();
    Serial.print(SPEED_PROMPT);
  }
}
