#include "Arduino.h"
#include <avr/io.h>
#include <avr/interrupt.h>

#define PIN6 (1 << 6)
#define PIN5 (1 << 5)

unsigned int interval = 500;
unsigned long prevMillis;

void setup() {
  DDRD |= (PIN6|PIN5); // Set PIN6 and PIN5 as output
  TCNT0 = 0;
  TIMSK0 |= 0b110; // OCIEA = 1; OCIEB = 1
  OCR0A = 128; // Up to 255
  OCR0B = 128;
  TCCR0B = 0b00000011; // Set clk source to clk/64
  prevMillis = millis();
  sei();
}

ISR(TIMER0_COMPA_vect) {
}

ISR(TIMER0_COMPB_vect) {
}

void right_motor_forward(void) {
  TCCR0A = 0b10000001;
}

void right_motor_reverse(void) {
  TCCR0A = 0b00100001;
}

void loop() {
  right_motor_forward();
  while(millis() - prevMillis < interval);
  prevMillis = millis();

  right_motor_reverse();
  while(millis() - prevMillis < interval);
  prevMillis = millis();
}
