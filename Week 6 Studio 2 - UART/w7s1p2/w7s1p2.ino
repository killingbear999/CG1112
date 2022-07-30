#include <avr/io.h>
#include <avr/interrupt.h>

/*#ifndef F_CPU
#define F_CPU   16000000UL
#endif
*/
#include <util/delay.h>

// Masks for pins 12 and 11
#define PIN12MASK   0b00010000
#define PIN11MASK   0b00001000

// UDRIE mask. Use this to enable/disable
// the UDRE interrupt
#define UDRIEMASK   0b00100000

static volatile char flashWhich=1;
static volatile char buttonVal=1;

char dataRecv, dataSend;


ISR(USART_RX_vect)
{
  // Write received data to dataRecv
  dataRecv = UDR0;
  
}

ISR(USART_UDRE_vect)
{
  // Write dataSend to UDR0
  // Disable UDRE interrupt
  UDR0 = dataSend; 
  UCSR0B &= 0b11011111;
}


void sendData(const char data)
{
  // Copy data to be sent to dataSend
  dataSend = buttonVal+'0';

  
  // Enable UDRE interrupt below
  UCSR0B |= 0b00100000;
  
}

char recvData()
{
  return dataRecv - '0';
}

void setupSerial()
{
  // Set up the 9600 8N1 when using
  // Serial Monitor to test
  UCSR0A = 0;
  UCSR0C = 0b00000110;
  UBRR0L = 103;
  UBRR0H = 0;
  

  // Change to 9600 7E1 when
  // communicating between Arduinos.
  //UCSR0C = 0b00100100;
  
}

void startSerial()
{
  // Start the serial port.
  // Enable RXC interrupt, but NOT UDRIE
  // Remember to enable the receiver
  // and transmitter
  UCSR0B = 0b10011000;
}

// Enable external interrupts 0 and 1
void setupEINT()
{
  // Configure INT0 and INT1 for rising edge triggered.
  // Remember to enable INT0 and INT1 interrupts.
  EICRA = 0b00001111;
  EIMSK = 0b00000011;
}


// ISRs for external interrupts
ISR(INT0_vect)
{
  buttonVal=1;  
  sendData(buttonVal);
}

ISR(INT1_vect)
{
  buttonVal=2;
  sendData(buttonVal);
}

// Red is on pin 12
void flashRed()
{
    PORTB |= PIN12MASK;
    delay(100);
    PORTB &= ~PIN12MASK;
    delay(500);
}

// Green is on pin 11
void flashGreen()
{
    PORTB |= PIN11MASK;
    delay(100);
    PORTB &= ~PIN11MASK;
    delay(500);
}

void setup() {

  cli();
  // put your setup code here, to run once:

  DDRB |= (PIN11MASK | PIN12MASK);
  setupEINT();
  setupSerial();
  startSerial();
  sei();
}

void loop() {
  // put your main code here, to run repeatedly:

  flashWhich = recvData();
   
  if(flashWhich == 1)
    flashRed();
  else
    flashGreen();
}
