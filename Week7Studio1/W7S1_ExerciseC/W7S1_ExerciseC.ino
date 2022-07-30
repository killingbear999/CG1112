#include "Arduino.h"

unsigned int adcvalue, loval, hival; 

void setup() {
  // Clear Bit 0 (PRADC) to turn on power for the ADC module 
  PRR &= ~(1 << PRADC);

  //ADEN = 1, ADPS[2:0] = 111 (Prescale = 128), ADIE = 1
  ADCSRA |= ((1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0) | (1 << ADIE));

  //REFS[1:0] = 01 (AVcc as reference), MUX[2:0] = 000 (Channel 0) 
  ADMUX |= ((1 << REFS0));

  // Set PortB Pin 5 as output 
  DDRB |= (1 << DDB5);
  sei();

  // ADSC = 1 (Start Conversion)
  ADCSRA |= (1 << ADSC); 
}

void loop() {
  ledToggle();
  _delay_loop_2(adcvalue);
}

void ledToggle() {
  PORTB ^= (1 << PORTB5);
}

ISR(ADC_vect) {
  // Provide your code for the ISR
  loval = ADCL;
  hival = ADCH;
  adcvalue = (hival << 8) | loval;
  ADCSRA |= 0b01000000;
}
