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
                 Serial.println(F("PSU ON")); \
                 delay(500) // PSU delay to wait for PSU to turn on
#define PSU_OFF() digitalWrite(PSU_PIN, 1); \
                 Serial.println(F("PSU OFF"))

// Serial macros
#define SERIAL_FLUSH() while (Serial.available()) Serial.read() // Clear the remaining data in the serial buffer
#define SPEED_PROMPT() SERIAL_FLUSH(); Serial.println(F("Set interval (1000-2000us): "))


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
  char act, next;

  // Command format: <TBSO>[speed]
  if (Serial.available()) {
    act = Serial.read();

    switch (act) {
      case 'T': // Top ESC
      case 'B': // Bottom ESC
        break;
      case 'S': // PSU [S]hutoff
        PSU_OFF();
        return;
      case 'O': // PSU [O]n
        // This will recalibrate range of ESCs
        // Might not be neccessary, but we'll do it anyways
        init_esc();
        return;
      default:
        Serial.println(F("Invalid format, expected motor ID"));
        return;
    }

    // Make sure next character in buffer is a valid number
    next = Serial.peek();
    if (next < '0' || next > '9') { // Invalid char
      Serial.println(F("Expected number after ESC ID"));
      return;
    }

    rawRead = Serial.parseInt();
    interval = constrain(rawRead, MIN_PERIOD, MAX_PERIOD);
    // Write us to ESC
    act == 'T' ? esc1.writeMicroseconds(interval) : esc2.writeMicroseconds(interval);
    // Print interval
    Serial.print(F("Set interval of ESC <")); Serial.print(act); Serial.print(F("> to "));
    Serial.print(interval); Serial.println(F("us"));

    SPEED_PROMPT();
  }
}
