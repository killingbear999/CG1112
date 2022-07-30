//Exercise A
/*#include "Arduino.h"
void setup() {
  // setting DDRB as output
  DDRB = B00110000; // Set PB5 & PB4 as output
}
void loop() {
  // toggle both LED's one at a time
  PORTB = B00100000; //Green LED: ON, Red LED: OFF 
  delay(1000);
  PORTB = B00010000; //Green LED: OFF, Red LED: ON 
  delay(1000);
}*/

//Exercise B&C
#include "Arduino.h"
//#define SW_MASK 0xF0
#define PIN3  (1 << 3)
#define PIN2  (1 << 2)
#define PIN7  (1 << 7)
#define PIN6  (1 << 6)
#define PIN5  (1 << 5)
#define PIN4  (1 << 4)
void setup() {
  // set as output
  DDRB = (PIN5 | PIN4 | PIN3 | PIN2);
  //set as input
  DDRD = ~(PIN7 | PIN6 | PIN5 | PIN4);
}
void loop() {
  /*// toggle both LED's one at a time
  PORTB = PIN5; //Green LED: ON, Red LED: OFF 
  delay(1000);
  PORTB = PIN4; //Green LED: OFF, Red LED: ON 
  delay(1000);*/
  //PORTB = ((PIND & SW_MASK) >> 2);
  if((PIND & (PIN4 | PIN5 | PIN6 | PIN7)) == 0xF0) {
    PORTB |= PIN2;
    delay(100);
    PORTB &= ~PIN2;
    delay(100);
    PORTB |= PIN3;
    delay(100);
    PORTB &= ~PIN3;
    delay(100);
    PORTB |= PIN4;
    delay(100);
    PORTB &= ~PIN4;
    delay(100);
    PORTB |= PIN5;
    delay(100);
    PORTB &= ~PIN5;
    delay(100);
  } else if ((PIND & (PIN4 | PIN5 | PIN6 | PIN7)) == 0x00) {
    PORTB |= PIN5;
    delay(100);
    PORTB &= ~PIN5;
    delay(100);
    PORTB |= PIN4;
    delay(100);
    PORTB &= ~PIN4;
    delay(100);
    PORTB |= PIN3;
    delay(100);
    PORTB &= ~PIN3;
    delay(100);
    PORTB |= PIN2;
    delay(100);
    PORTB &= ~PIN2;
    delay(100);
  } else {
    
    if(PIND & PIN7) {
      PORTB |= PIN5;
    } else {
      PORTB &= ~PIN5;
    } 
    
    if(PIND & PIN6) {
      PORTB |= PIN4;
    } else {
      PORTB &= ~PIN4;
    } 
  
    if(PIND & PIN5) {
      PORTB |= PIN3;
    } else {
      PORTB &= ~PIN3;
    } 
  
    if(PIND & PIN4) {
      PORTB |= PIN2;
    } else {
      PORTB &= ~PIN2;
    }
  }
}
