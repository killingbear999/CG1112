#include "serialize.h"
#include "packet.h"
#include "constants.h"
#include <math.h>
#include <avr/sleep.h>

#define PRR_TWI_MASK 0b10000000
#define PRR_SPI_MASK 0b00000100
#define ADCSRA_ADC_MASK 0b10000000
#define PRR_ADC_MASK 0b00000001
#define PRR_TIMER2_MASK 0b01000000
#define PRR_TIMER0_MASK 0b00100000
#define PRR_TIMER1_MASK 0b00001000
#define SMCR_SLEEP_ENABLE_MASK 0b00000001
#define SMCR_IDLE_MODE_MASK 0b11110001

typedef enum
{
  STOP = 0,
  FORWARD = 1,
  BACKWARD = 2,
  LEFT = 3,
  RIGHT = 4
} TDirection;
volatile TDirection dir = STOP;
/*

   Alex's configuration constants
*/

// Number of ticks per revolution from the
// wheel encoder.
#define COUNTS_PER_REV      190

// Wheel circumference in cm.
// We will use this to calculate forward/backward distance traveled
// by taking revs * WHEEL_CIRC
#define WHEEL_CIRC          20

// ALEX's length and breadth in cm
#define ALEX_LENGTH         20
#define ALEX_BREADTH        13


// ALEX's diagonal, we compute and store this once
// since it is expensive to compute and really doesn't change
float AlexDiagonal = 0.0;

// ALEX's turning circumference, calculated once

float AlexCirc = 0.0;

// Motor control pins. You need to adjust these till
// Alex moves in the correct direction
#define LF                  6  // Left forward pin
#define LR                 5  // Left reverse pin
#define RF                  10// Right forward pin
#define RR                  11// Right reverse pin

/*
      Alex's State Variables
*/

// Store the ticks from Alex's left and
// right encoders.
volatile unsigned long leftForwardTicks;
volatile unsigned long rightForwardTicks;
volatile unsigned long leftReverseTicks;
volatile unsigned long rightReverseTicks;

volatile unsigned long distanceLeft;
volatile unsigned long distanceRight;

// Store the revolutions on Alex's left
// and right wheels
volatile unsigned long leftForwardTicksTurns;
volatile unsigned long rightForwardTicksTurns;
volatile unsigned long leftReverseTicksTurns;
volatile unsigned long rightReverseTicksTurns;

// Forward and backward distance traveled
volatile unsigned long forwardDist;
volatile unsigned long reverseDist;

// Variable to keep track of whether we have mwasoved a commanded distance
unsigned long deltaDist;
unsigned long newDist;

// Variables to keep track of our turning angle
unsigned long deltaTicks;
unsigned long targetTicks;

// Variables for bare-metal coding of PWM
unsigned int interval = 500;
unsigned long prevMillis;

// Variables for bare-metal coding of serialisation
char dataRecv, dataSend;

/*

   Alex Communication Routines.

*/

TResult readPacket(TPacket *packet)
{
  // Reads in data from the serial port and
  // deserializes it.Returns deserialized
  // data in "packet".

  char buffer[PACKET_SIZE];
  int len;

  len = readSerial(buffer);

  if (len == 0)
    return PACKET_INCOMPLETE;
  else
    return deserialize(buffer, len, packet);

}

void sendStatus()
{
  // Implement code to send back a packet containing key
  // information like leftTicks, rightTicks, leftRevs, rightRevs
  // forwardDist and reverseDist
  // Use the params array to store this information, and set the
  // packetType and command files accordingly, then use sendResponse
  // to send out the packet. See sendMessage on how to use sendResponse.
  //
  TPacket statusPacket;
  statusPacket.packetType = PACKET_TYPE_RESPONSE;
  statusPacket.command = RESP_STATUS;
  statusPacket.params[0] = leftForwardTicks;
  statusPacket.params[1] = rightForwardTicks;
  statusPacket.params[2] = leftReverseTicks;
  statusPacket.params[3] = rightReverseTicks;
  statusPacket.params[4] = leftForwardTicksTurns;
  statusPacket.params[5] = rightForwardTicksTurns;
  statusPacket.params[6] = leftReverseTicksTurns;
  statusPacket.params[7] = rightReverseTicksTurns;
  statusPacket.params[8] = forwardDist;
  statusPacket.params[9] = reverseDist;
  sendResponse(&statusPacket);
}

void sendMessage(const char *message)
{
  // Sends text messages back to the Pi. Useful
  // for debugging.

  TPacket messagePacket;
  messagePacket.packetType = PACKET_TYPE_MESSAGE;
  strncpy(messagePacket.data, message, MAX_STR_LEN);
  sendResponse(&messagePacket);
}

void sendBadPacket()
{
  // Tell the Pi that it sent us a packet with a bad
  // magic number.

  TPacket badPacket;
  badPacket.packetType = PACKET_TYPE_ERROR;
  badPacket.command = RESP_BAD_PACKET;
  sendResponse(&badPacket);

}

void sendBadChecksum()
{
  // Tell the Pi that it sent us a packet with a bad
  // checksum.

  TPacket badChecksum;
  badChecksum.packetType = PACKET_TYPE_ERROR;
  badChecksum.command = RESP_BAD_CHECKSUM;
  sendResponse(&badChecksum);
}

void sendBadCommand()
{
  // Tell the Pi that we don't understand its
  // command sent to us.

  TPacket badCommand;
  badCommand.packetType = PACKET_TYPE_ERROR;
  badCommand.command = RESP_BAD_COMMAND;
  sendResponse(&badCommand);

}

void sendBadResponse()
{
  TPacket badResponse;
  badResponse.packetType = PACKET_TYPE_ERROR;
  badResponse.command = RESP_BAD_RESPONSE;
  sendResponse(&badResponse);
}

void sendOK()
{
  TPacket okPacket;
  okPacket.packetType = PACKET_TYPE_RESPONSE;
  okPacket.command = RESP_OK;
  sendResponse(&okPacket);
}

void sendResponse(TPacket *packet)
{
  // Takes a packet, serializes it then sends it out
  // over the serial port.
  char buffer[PACKET_SIZE];
  int len;

  len = serialize(buffer, packet, sizeof(TPacket));
  writeSerial(buffer, len);
}


/*
   Setup and start codes for external interrupts and
   pullup resistors.

*/
// Enable pull up resistors on pins 2 and 3
void enablePullups()
{
  // Use bare-metal to enable the pull-up resistors on pins
  // 2 and 3. These are pins PD2 and PD3 respectively.
  // We set bits 2 and 3 in DDRD to 0 to make them inputs.
  DDRD &= 0b11110011;
  PORTD |= 0b00001100;
  //Right = 180
  //Left = 195

}

// Functions to be called by INT0 and INT1 ISRs.
void leftISR()
{
  if (dir == FORWARD) {
    leftForwardTicks ++;
    forwardDist = -5;
    forwardDist = (unsigned long) ((float) leftForwardTicks / COUNTS_PER_REV * WHEEL_CIRC);
  } else if (dir == BACKWARD) {
    leftReverseTicks ++;
    reverseDist = (unsigned long) ((float) leftReverseTicks / COUNTS_PER_REV * WHEEL_CIRC);
  } else if (dir == LEFT) {
    leftReverseTicksTurns++;
  } else if (dir == RIGHT) {
    leftForwardTicksTurns++;
  }
}
void rightISR()
{
  if (dir == FORWARD) {
    rightForwardTicks++;
  } else if (dir == BACKWARD) {
    rightReverseTicks++;
  } else if (dir == RIGHT) {
    rightReverseTicksTurns++;
  } else if (dir == LEFT) {
    rightForwardTicksTurns++;
  }
}



// Set up the external interrupt pins INT0 and INT1
// for falling edge triggered. Use bare-metal.
void setupEINT()
{
  // Use bare-metal to configure pins 2 and 3 to be
  // falling edge triggered. Remember to enable
  // the INT0 and INT1 interrupts.
  cli();

  EICRA |= 0b00001010; // falling edge INT0, INT1

  EIMSK |= 0b00000011; // enable INT0, INT1

  sei();
}

// Implement the external interrupt ISRs below.
// INT0 ISR should call leftISR while INT1 ISR
// should call rightISR.

ISR(INT0_vect)
{
  leftISR();
}

ISR(INT1_vect)
{
  rightISR();
}


// Implement INT0 and INT1 ISRs above.

/*
   Setup and start codes for serial communications

*/
// Set up the serial connection.
void setupSerial()
{
  // Bare-metal programming for setupSerial()
  /*// Using 9600 7E1
    UCSR0A = 0;
    UBRR0L = 103;
    UBRR0H = 0;
    UCSR0C = 0b00100100;*/
  Serial.begin(9600);
}

// Start the serial connection.
void startSerial()
{
  // Bare-metal programming for startSerial()
  /*// Start the serial port.
    // Enable RXC interrupt, but NOT UDRIE
    // Remember to enable the receiver
    // and transmitter
    UCSR0B = 0b10011000;*/
}

// Read the serial port. Returns the read character in
// ch if available. Also returns TRUE if ch is valid.
// Bare-metal pragramming for receiving data
/*ISR(USART_RX_vect)
  {
  // Write received data to dataRecv
  dataRecv = UDR0;
  }
  char recvData()
  {
  return dataRecv - '0';
  }*/
int readSerial(char *buffer)
{
  int count = 0;

  while (Serial.available())
    buffer[count++] = Serial.read();

  return count;
}

// Write to the serial port.
// Bare-metal programming for sending data
/*ISR(USART_UDRE_vect)
  {
  // Write dataSend to UDR0
  // Disable UDRE interrupt
  UDR0 = dataSend;
  UCSR0B &= 0b11011111;
  }
  void sendData(const char data)
  {
  // Copy data to be sent to dataSend
  dataSend = buttonVal + '0';
  // Enable UDRE interrupt below
  UCSR0B |= 0b00100000;
  }*/
void writeSerial(const char *buffer, int len)
{
  Serial.write(buffer, len);
}

/*
   Alex's motor drivers.

*/

// Set up Alex's motors.
void setupMotors()
{
  /* Our motor set up is:
        A1IN - Pin 5, PD5, OC0B
        A2IN - Pin 6, PD6, OC0A
        B1IN - Pin 10, PB2, OC1B
        B2In - pIN 11, PB3, OC2A
  */
  // Bare-metal progtamming for setupMotors()
  /*cli();

    // Set up output pins
    DDRB |= ((1 << DDB2) | (1 << DDB3));
    DDRD |= ((1 << DDD5) | (1 << DDD6));

    // Timer0
    TCNT0 = 0;
    TIMSK0 |= 0b110;
    TCCR0B = 0b00000011;

    // Timer2
    TCNT2 = 0;
    TIMSK2 |= 0b110;
    TCCR2B = 0b00000011;

    prevMillis = millis();
    sei();*/
}

// Bare-metal programming for calling ISR
/*ISR(TIMER0_COMPA_vect) {
  }

  ISR(TIMER0_COMPB_vect) {
  }

  ISR(TIMER2_COMPB_vect) {
  }

  ISR(TIMER2_COMPA_vect) {
  }*/

// Start the PWM for Alex's motors.
void startMotors()
{
  // Bare-metal programming for startMotors()
  /*while (millis() - prevMillis < interval);
    prevMillis = millis();*/
}


// Convert percentages to PWM values
int pwmVal(float speed)
{
  if (speed < 0.0)
    speed = 0;

  if (speed > 100.0)
    speed = 100.0;

  return (int) ((speed / 100.0) * 255.0);
}

// Move Alex forward "dist" cm at speed "speed".
// "speed" is expressed as a percentage. E.g. 50 is
// move forward at half speed.
// Specifying a distance of 0 means Alex will
// continue moving forward indefinitely.
void reverse(float dist, float speed)
{
  //code to tell us haw far to move
  if (dist > 0) {
    deltaDist = dist;
  } else {
    deltaDist = 9999999;
  }

  newDist = forwardDist + deltaDist;

  dir = FORWARD;
  int val = pwmVal(speed);

  // Bare-metal programming for moving forward
  /*TCCR0A = 0b10000001;
    TCCR2A = 0b00100001;
    OCR0A = val; //LF
    OCR2B = val; //RF
    OCR0B = 0; //LR
    OCR2A = 0; //RR*/

  // LF = Left forward pin, LR = Left reverse pin
  // RF = Right forward pin, RR = Right reverse pin
  analogWrite(LF, val);
  analogWrite(RF, val / 1.25);
  analogWrite(LR, 0);
  analogWrite(RR, 0);
}

// Reverse Alex "dist" cm at speed "speed".
// "speed" is expressed as a percentage. E.g. 50 is
// reverse at half speed.
// Specifying a distance of 0 means Alex will
// continue reversing indefinitely.
void forward(float dist, float speed)
{
  //code to tell us haw far to move
  if (dist > 0) {
    deltaDist = dist;
  } else {
    deltaDist = 9999999;
  }

  newDist = reverseDist + deltaDist;

  dir = BACKWARD;
  int val = pwmVal(speed);

  // Bare-metal programming for moving reverse
  /*TCCR0A = 0b00100001;
    TCCR2A = 0b10000001;
    OCR0A = 0; //LF
    OCR2B = 0; //RF
    OCR0B = val; //LR
    OCR2A = val; //RR*/

  // LF = Left forward pin, LR = Left reverse pin
  // RF = Right forward pin, RR = Right reverse pin
  analogWrite(LR, val);
  analogWrite(RR, val * 0.8);
  analogWrite(LF, 0);
  analogWrite(RF, 0);
}

// New function to estimate number of wheel ticks
// needed to turn an angle
unsigned long computeDeltaTicks(float ang) {
  // We will assume that angular distance moved = linear distance moved in one wheel
  // revolution. This is probably incorrect but simplfies calculation.
  // # of wheel revs to make one full 360 turn is AlexCirc/WHEEL_CIRC
  // This is for 360 degrees. For ang degrees it will be (ang*AlexCirc)/(360*WHEEL_CIRC)
  // To convert to ticks, we multiply by COUNTS_PER_REV
  unsigned long ticks = (unsigned long) ((ang * AlexCirc * COUNTS_PER_REV) / (360 * WHEEL_CIRC));
  return ticks;
}

// Turn Alex left "ang" degrees at speed "speed".
// "speed" is expressed as a percentage. E.g. 50 is
// turn left at half speed.
// Specifying an angle of 0 degrees will cause Alex to
// turn left indefinitely.
void right(float ang, float speed)
{
  ang = ang / 4.5;
  if (ang == 0) {
    deltaTicks = 99999999;
  } else {
    deltaTicks = computeDeltaTicks(ang);
  }

  targetTicks = leftReverseTicksTurns + deltaTicks;

  dir = LEFT;
  int val = pwmVal(speed);

  // Bare-metal programming for moving rightward
  /*TCCR0A = 0b00100001;
    TCCR2A = 0b00100001;
    OCR0A = 0; //LF
    OCR2B = val; //RF
    OCR0B = val; //LR
    OCR2A = 0; //RR*/

  // To turn left we reverse the left wheel and move
  // the right wheel forward.
  analogWrite(LR, val);
  analogWrite(RF, val);
  analogWrite(LF, 0);
  analogWrite(RR, 0);
}

// Turn Alex right "ang" degrees at speed "speed".
// "speed" is expressed as a percentage. E.g. 50 is
// turn left at half speed.
// Specifying an angle of 0 degrees will cause Alex to
// turn right indefinitely.
void left(float ang, float speed)
{
  ang = ang / 3.6;
  if (ang == 0) {
    deltaTicks = 99999999;
  } else {
    deltaTicks = computeDeltaTicks(ang);
  }

  targetTicks = rightReverseTicksTurns + deltaTicks;

  dir = RIGHT;
  int val = pwmVal(speed);

  // Bare-metal programming for moving leftward
  /*TCCR0A = 0b10000001;
    TCCR2A = 0b10000001;
    OCR0A = val; //LF
    OCR2B = 0; //RF
    OCR0B = 0; //LR
    OCR2A = val; //RR*/

  // To turn right we reverse the right wheel and move
  // the left wheel forward.
  analogWrite(RR, val);
  analogWrite(LF, val);
  analogWrite(LR, 0);
  analogWrite(RF, 0);
}

// Stop Alex.
void stop()
{
  dir = STOP;

  // Bare-metal programming for stop()
  /*TCCR0A = 0b00000001;
    TCCR2A = 0b00000001;
    OCR0A = 0; //LF
    OCR2B = 0; //RF
    OCR0B = 0; //LR
    OCR2A = 0; //RR*/

  analogWrite(LF, 0);
  analogWrite(LR, 0);
  analogWrite(RF, 0);
  analogWrite(RR, 0);
}

/*
   Alex's setup and run codes

*/

// Clears all our counters
void clearCounters()
{
  leftForwardTicks = 0;
  rightForwardTicks = 0;
  leftReverseTicks = 0;
  rightReverseTicks = 0;
  leftForwardTicksTurns = 0;
  rightForwardTicksTurns = 0;
  leftReverseTicksTurns = 0;
  rightReverseTicksTurns = 0;
  forwardDist = 0;
  reverseDist = 0;
}

// Clears one particular counter
void clearOneCounter(int which)
{
  /*switch(which)
    {
    case 0:
      clearCounters();
      break;

    case 1:
      leftTicks=0;
      break;

    case 2:
      rightTicks=0;
      break;

    case 3:
      leftRevs=0;
      break;

    case 4:
      rightRevs=0;
      break;

    case 5:
      forwardDist=0;
      break;

    case 6:
      reverseDist=0;
      break;
    }*/
  clearCounters();
}
// Intialize Vincet's internal states

void initializeState()
{
  clearCounters();
}

void handleCommand(TPacket *command)
{
  switch (command->command)
  {
    // For movement commands, param[0] = distance, param[1] = speed.
    case COMMAND_FORWARD:
      sendOK();
      forward((float) command->params[0], (float) command->params[1]);
      break;
    case COMMAND_REVERSE:
      sendOK();
      reverse((float) command->params[0], (float) command->params[1]);
      break;

    case COMMAND_TURN_LEFT:
      sendOK();
      left((float) command->params[0], (float) command->params[1]);
      break;

    case COMMAND_TURN_RIGHT:
      sendOK();
      right((float) command->params[0], (float) command->params[1]);
      break;

    case COMMAND_STOP:
      sendOK();
      break;

    case COMMAND_GET_STATS:
      sendStatus();
      break;

    case COMMAND_CLEAR_STATS:
      sendOK();
      clearOneCounter(command->params[0]);
      break;
    /*
       Implement code for other commands here.

    */

    default:
      sendBadCommand();
  }
}

void waitForHello()
{
  int exit = 0;

  while (!exit)
  {
    TPacket hello;
    TResult result;

    do
    {
      result = readPacket(&hello);
    } while (result == PACKET_INCOMPLETE);

    if (result == PACKET_OK)
    {
      if (hello.packetType == PACKET_TYPE_HELLO)
      {


        sendOK();
        exit = 1;
      }
      else
        sendBadResponse();
    }
    else if (result == PACKET_BAD)
    {
      sendBadPacket();
    }
    else if (result == PACKET_CHECKSUM_BAD)
      sendBadChecksum();
  } // !exit
}

// turning off the Watchdog Timer
void WDT_off(void)
{
  // Clear WDRF in MCUSR
  MCUSR &= ~(1 << WDRF);

  // Write logical one to WDCE and WDE
  // Keep old prescaler setting to prevent unintentioanl time-out
  WDTCSR |= (1 << WDCE) | (1 << WDE);

  // turn off WDT
  WDTCSR = 0x00;
}

void setupPowerSaving()
{
  // Turn off the Watchdog Timer
  WDT_off();

  // Modify PRR to shut down TWI
  // Modify PRR to shut down SPI
  PRR |= PRR_TWI_MASK;
  PRR |= PRR_SPI_MASK;

  // Modify ADCSRA to disable ADC,
  // then modify PRR to shut down ADC
  ADCSRA |= ADCSRA_ADC_MASK;
  PRR |= PRR_ADC_MASK;

  // Set the SMCR to choose the IDLE sleep mode
  // Do not set the Sleep Enable (SE) bit yet
  SMCR |= SMCR_IDLE_MODE_MASK;

  // Set Port B Pin 5 as output pin, then write a logic LOW
  // to it so that the LED tied to Arduino's Pin 13 is OFF.
  DDRB |= (5 << 1);
  PORTB &= ~(5 << 1);
}

void putArduinoToIdle()
{
  // Modify PRR to shut down TIMER 0, 1, and 2
  PRR |= (PRR_TIMER2_MASK | PRR_TIMER0_MASK | PRR_TIMER1_MASK);

  // Modify SE bit in SMCR to enable (i.e., allow) sleep
  SMCR |= SMCR_SLEEP_ENABLE_MASK;
  // The following function puts ATmega328P’s MCU into sleep;
  // it wakes up from sleep when USART serial data arrives
  sleep_cpu();

  // Modify SE bit in SMCR to disable (i.e., disallow) sleep
  SMCR &= ~(SMCR_SLEEP_ENABLE_MASK);

  // Modify PRR to power up TIMER 0, 1, and 2
  PRR &= ~(PRR_TIMER2_MASK | PRR_TIMER0_MASK | PRR_TIMER1_MASK);
}

void setup() {
  // put your setup code here, to run once:

  AlexDiagonal = sqrt((ALEX_LENGTH * ALEX_LENGTH) + (ALEX_BREADTH * ALEX_BREADTH));
  AlexCirc = PI * AlexDiagonal;

  cli();
  setupEINT();
  setupSerial();
  setupPowerSaving();
  startSerial();
  setupMotors();
  startMotors();
  enablePullups();
  initializeState();
  sei();
}

void handlePacket(TPacket *packet)
{
  switch (packet->packetType)
  {
    case PACKET_TYPE_COMMAND:
      handleCommand(packet);
      break;

    case PACKET_TYPE_RESPONSE:
      break;

    case PACKET_TYPE_ERROR:
      break;

    case PACKET_TYPE_MESSAGE:
      break;

    case PACKET_TYPE_HELLO:
      break;
  }
}

void loop() {

  // Uncomment the code below for Step 2 of Activity 3 in Week 8 Studio 2

  //forward(0, 100);

  // Uncomment the code below for Week 9 Studio 2


  // put your main code here, to run repeatedly:
  TPacket recvPacket; // This holds commands from the Pi

  TResult result = readPacket(&recvPacket);

  if (result == PACKET_OK) {
    handlePacket(&recvPacket);
  } else if (result == PACKET_BAD) {
    sendBadPacket();
  } else if (result == PACKET_CHECKSUM_BAD) {
    sendBadChecksum();
  }

  if (deltaDist > 0) {
    if (dir == FORWARD) {
      if (forwardDist >= newDist) {
        deltaDist = 0;
        newDist = 0;
        stop();
      }
    } else if (dir == BACKWARD) {
      if (reverseDist >= newDist) {
        deltaDist = 0;
        newDist = 0;
        stop();
      }
    } else if (dir == STOP) {
      deltaDist = 0;
      newDist = 0;
      stop();
    }
  }

  if (deltaTicks > 0) {
    if (dir == LEFT) {
      if (leftReverseTicksTurns >= targetTicks) {
        deltaTicks = 0;
        targetTicks = 0;
        stop();
      }
    } else if (dir == RIGHT) {
      if (rightReverseTicksTurns >= targetTicks) {
        deltaTicks = 0;
        targetTicks = 0;
        stop();
      }
    } else if (dir == STOP) {
      deltaTicks = 0;
      targetTicks = 0;
      stop();
    }
  }

  if (dir == STOP)
  {
    putArduinoToIdle();
  }
}
