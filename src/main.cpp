#include <Arduino.h>

#include <Servo.h>

#define MAX_PERIOD 2000
#define MIN_PERIOD 1000

#define ESC_1_PIN  2
#define ESC_2_PIN  3
#define PSU_PIN    4


Servo esc1;
Servo esc2;

// PSU macros
#define PSU_ON()  { digitalWrite(PSU_PIN, 0); \
                    Serial.println(F("PSU ON")); \
                    delay(500); \
                    psuOn = true; \
                  } // PSU delay to wait for PSU to turn on
#define PSU_OFF() { esc1.writeMicroseconds(MIN_PERIOD); \
                    esc2.writeMicroseconds(MIN_PERIOD); \
                    digitalWrite(PSU_PIN, 1); \
                    Serial.println(F("PSU OFF")); \
                    psuOn = false; \
                  }

// Serial macros
#define SERIAL_FLUSH() while (Serial.available()) Serial.read() // Clear the remaining data in the serial buffer
#define SPEED_PROMPT() SERIAL_FLUSH(); Serial.print(F("Set interval (1000-2000us): "))


bool psuOn = false;

/**
 * Initialises ESCs with correct startup sequence
 * 
 * This calibrates the max and min period durations.
*/
void init_esc() {
  Serial.println(F("ESC init start"));
  esc1.attach(ESC_1_PIN);
  esc2.attach(ESC_2_PIN);

  // Calibration sequence
  esc1.writeMicroseconds(MAX_PERIOD);
  esc2.writeMicroseconds(MAX_PERIOD);

  PSU_ON();
  Serial.println(F("Calibrate max duty"));
  delay(2000); // ESC timeout
  Serial.println(F("Calibrate min duty"));
  esc1.writeMicroseconds(MIN_PERIOD);
  esc2.writeMicroseconds(MIN_PERIOD);
  delay(2500);

  Serial.println(F("ESC init done"));
}


void setup() {
  // Init serial
  Serial.begin(115200);
  Serial.println(F("Start"));

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(PSU_PIN, OUTPUT);

  // Switch off PSU initially
  PSU_OFF();

  // Init ESC with startup sequence
  init_esc();

  // This line signals to hosts that the initialisation sequence has completed
  Serial.println(F("Ready"));

  SPEED_PROMPT();
}


void loop() {
  // Pull out all variables used in loop for better "memory management"
  uint32_t rawRead, interval;
  char act;

  // Command format: <TBSO>[speed]
  if (Serial.available()) {
    act = Serial.read();

    switch (act) {
      case 'T': // Top ESC
      case 'B': // Bottom ESC
      case 'P': // PSU actions
        break;
      case 'S': // Stop all motors
        esc1.writeMicroseconds(MIN_PERIOD);
        esc2.writeMicroseconds(MIN_PERIOD);
        Serial.println(F("Stopped all ESCs"));
        goto prompt;
      default:
        Serial.println(F("Invalid format, expected motor ID"));
        goto prompt;
    }

    // Make sure next character in buffer is a valid number
    // FIXME: peek() call always returns 0xff, find out why
    /* next = Serial.peek();
    if (next < '0' || next > '9') { // Invalid char
      Serial.print(F("Expected number after command, got ")); Serial.println(next);
      goto prompt;
    } */

    rawRead = Serial.parseInt(SKIP_NONE);
    if (act == 'P') {
      if (rawRead == 0) PSU_OFF() else init_esc();
    } else {
      interval = constrain(rawRead, MIN_PERIOD, MAX_PERIOD);
      // Turn on the PSU if it isn't already
      if (!psuOn) init_esc();
      // Write us to ESC
      act == 'T' ? esc1.writeMicroseconds(interval) : esc2.writeMicroseconds(interval);
      // Print interval
      Serial.print(F("Set interval of ESC <")); Serial.print(act); Serial.print(F("> to "));
      Serial.print(interval); Serial.println(F("us"));
    }

    goto prompt;
  }

  return;
  prompt:
    SPEED_PROMPT();
    return;
}
