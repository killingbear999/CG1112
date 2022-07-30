#include "Arduino.h"
#include <avr/io.h>
#include <avr/interrupt.h>

#define PIN7 (1 << 7)
#define PIN6 (1 << 6)
#define PIN5 (1 << 5)
#define PIN4 (1 << 4)
#define PIN3 (1 << 3)
#define PIN2 (1 << 2)
#define PIN1 (1 << 1)
#define PIN0 (1 << 0)

static volatile int change = 1;

void setup() {
  TCNT0 = 0; 
  TCCR0A = 0b10000001; // Set OCOM0A to 10 and WGM to 01
  //TCCR0A = 0b11000001; // Set OCOM0A to 11 and WGM to 01
  TIMSK0 |= 0b10; // Enable Interrupt for Output Compare Match
  OCR0A = 25; 
  TCCR0B = 0b00000011; // Set clk source to clk/64
  //TCCR0B = 0b00000101; // Set clk source to clk/1024
  sei();
}

ISR(TIMER0_COMPA_vect) {
  if(OCR0A==0) {
    change = 1;
  }
  if(OCR0A==255) {
    change = -1;
  }
  OCR0A = OCR0A + change; 
}

void loop() {

  // Set PORTD Pin 6 (Arduino Pin 6) as Output
  DDRD |= PIN6;

  while(1) {
  }
}
