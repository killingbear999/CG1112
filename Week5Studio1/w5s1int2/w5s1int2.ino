#include <avr/io.h>
#include <avr/interrupt.h>

static volatile int onOff = 0;
 
/* Our setup routine */
void setup() {
 // Set the INT0 interrupt to respond on the RISING edge.
 // We need to set bits 0 and 1
 // in EICRA to 1
 EICRA |= 0b00000011;
 
 // The green LED is on Pin 12, which is on PB4. Set it to OUTPUT
 DDRB |= 0b00010000;
 
 // Activate INT0
 EIMSK |= 0b00000001;
 
 // Ensure that interrupts are turned on.
 sei();
 } 

/* Here we declare the Interrupt Service Routine (ISR) for INT0_vect */
ISR(INT0_vect) {
  onOff = 1 - onOff;
}
 
/* Our setup routine */
void loop() {
 // Switch off the LED if onOff is 0, otherwise switch it on.
 if(onOff==0) {
  PORTB &=0b11101111;
 } else {
  PORTB |= 0b00010000;
 }
}
 
