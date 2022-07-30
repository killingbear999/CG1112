/*
 * w6s2p2.c
 *
 * Created: 2/2/2018 6:01:01 AM
 * Author : dcstanc
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define LED_DELAY 100
#define PIN3 (1 << 3)
#define PIN2 (1 << 2)

// The turn variable decides whose turn it is to go now.
static volatile int turn=0;

// The _timerTicks variable maintains a count of how many 100us ticks have passed by.
static unsigned long _timerTicks = 0;

/*// Our INT0 ISR just flips the turn variable
ISR(INT0_vect)
{
  // Modify this ISR to do switch debouncing
  turn = 1 - turn;
}*/

// Uncomment and properly declare the ISR below for timer 1


ISR(TIMER1_COMPA_vect)
{
  _timerTicks++;
  if(_timerTicks==5) {
    turn = 1 - turn;
    _timerTicks = 0;
  }
  // Implement the rest of the code to switch between LEDs every 5 seconds 
  
}



// Flash the green LED
void flashGreen()
{
  int count=1;
  int i;

  while(turn==0)
  {
    for(i=0; i<count && turn==0; i++)
    {
      // Switch green LED at pin 12 on. Pin 12 is PB4
      PORTB |= 0b00010000;
    
      // Delay 250ms
      _delay_ms(LED_DELAY);
    
      PORTB &= 0b11101111;
      _delay_ms(LED_DELAY);
    }
    
    _delay_ms(1000);
    count++;
  } 
}

void flashRed()
{
  int count=1;
  int i;

  while(turn==1)
  {
    for(i=0; i<count && turn==1; i++)
    {
      // Switch red LED at pin 11 on. Pin 11 is PB4
      PORTB |= 0b00001000;
      
      // Delay 250ms
      _delay_ms(LED_DELAY);
      
      PORTB &= 0b11110111;
      _delay_ms(LED_DELAY);
    }
    
    _delay_ms(1000);
    count++;
  }
}

void setup()
{
  // Clear interrupts for safety while setting up
  cli();
  
  /*// Set up EICRA
  // For falling edge on INT0, Bits 1 and 0 should be 10
  EICRA |= 0b00000011;*/
  
  // Set pins 11 and 12 to output. Pin 11 is PB3, pin 12 is PB4
  DDRB |= 0b00011000;

  // Set up timer 1
  setupTimer();

  // Activate INT0
  //EIMSK |= 0b00000001; 
  
  // Start timer 1
  startTimer();

  // Ensure that interrupts are enabled 
  sei();
}

// Timer set up function. 
void setupTimer()
{
  // Set timer 1 to produce 1s (1000000us) ticks 
  // But do no start the timer here.
  cli();
  TCCR1A = 0b00000000;
  TCCR1B = 0;
  TIMSK1 |= 0b10;
  
  TCNT1 = 0;
  OCR1A = 62500; // V value
  sei();
}

// Timer start function
void startTimer()
{
  // Start timer 1 here.
  TCCR1B |= PIN3 | PIN2;
  //TCCR1B = 0b00001100;
  sei();
  
}

void loop()
{
    if(turn==0)
      flashGreen();
    else
      flashRed();
  
}
