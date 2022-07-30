#include "Arduino.h"

unsigned int adcvalue, loval, hival; 

void setup() {
  // Clear Bit 0 (PRADC) to turn on power for the ADC module 
  PRR &= ~(1 << PRADC);

  //ADEN = 1, ADPS[2:0] = 111 (Prescale = 128)
  ADCSRA |= ((1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0));

  //REFS[1:0] = 01 (AVcc as reference), MUX[2:0] = 000 (Channel 0) 
  ADMUX |= ((1 << REFS0));

  // Set PortB Pin 5 as output
  DDRB |= (1 << DDB5); 
  Serial.begin(9600);
}

void loop() {
  // ADSC = 1 (Start Conversion) 
  ADCSRA |= (1 << ADSC);

  /*Wait for ADSC to go change to '0' to indicate that conversion is complete*/
  while(ADCSRA & (1 << ADSC));
  
  loval = ADCL;
  hival = ADCH;
  adcvalue = (hival << 8) | loval;
  Serial.println(adcvalue);
  
  ledToggle();
  _delay_loop_2(adcvalue);
}

void ledToggle() {
  PORTB ^= (1 << PORTB5);
}
