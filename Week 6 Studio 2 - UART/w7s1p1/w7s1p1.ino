#include <avr/io.h>
#include <avr/interrupt.h>


// Masks for pins 12 and 11
#define PIN12MASK   0b00010000
#define PIN11MASK   0b00001000
#define BAUDRATE 9600

static volatile char flashWhich=1;
static volatile char buttonVal=1;


void sendData(const char data)
{
  // Poll UDRE bit in UCSR0A until it is 1
  // Send data
  while((UCSR0A & 0b00100000) == 0);
  UDR0 = data;
}

char recvData()
{
  // If RXC0 bit is 0, return 0
  // Otherwise return contents of UDR0 - '0'
  while((UCSR0A & 0b10000000) == 0 );
  unsigned char data = UDR0 - '0';
  return data;
}

void setupSerial()
{
  // Set up for 9600 8N1 for testing with Serial Monitor
  
  // zero everything in UCSR0A, espy U2X0 and MPCM0 to ensure we are not in
  // double-speed mode and that we are also not in multiprocessor mode, which 
  // will discard frames without addresses
  UCSR0A = 0;

  // running in asynchronous mode so bits 7 and 6 are 00;
  // we don't want oarity, so bits 5 and 4 are 00;
  // we want 1 stop bit, so bit 3 is 0;
  // we want 8 bits, so UCSZ02/UCSZ01/UCSZ00 are 0b011. UCSZ01and UCSZ00 are bits 2 and 1 on this register and should be 11;
  // UCSZ02 is set to 0 in UCSR0B below;
  // bit 0 (UCPOL0) is always 0;
  //UCSR0C = 0b00000110;
  
  // this code sets up USART0 for 9066 bps, 8N1 configuration
  /*unsigned int b;
  b = (unsigned int) round(F_CPU/(16.0*BAUDRATE)) - 1;
  UBRR0H = (unsigned char) (b >> 8);
  UBRR0L = (unsigned char) b;*/
  UBRR0L = 103;
  UBRR0H = 0;
  
  // Change to 9600 7E1 when communicating with another
  // Arduino
  UCSR0C = 0b00100100;
}

void startSerial()
{
  // Start the transmitter and receiver, but disable
  // all interrupts.
  UCSR0B = 0b00011000;
  
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
  sendData('0'+buttonVal);
}

ISR(INT1_vect)
{
  buttonVal=2;
  sendData('0'+buttonVal);
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

  char data = recvData();

  if(data != 0)
    flashWhich = data;
    
  if(flashWhich == 1)
    flashRed();
  else
    flashGreen();
}
