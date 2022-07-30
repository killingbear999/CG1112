#define REDPIN    11
#define GREENPIN  12
#define SWITCHPIN 2

#define LED_DELAY   100

// This variable decides which LED's turn it is to flash.
// 0 = green, 1 = red
static volatile int turn=0;

void setup() {
  pinMode(REDPIN, OUTPUT);
  pinMode(GREENPIN, OUTPUT);
  pinMode(SWITCHPIN, INPUT);
}

void flashGreen()
{
  int counter=1;

  while(turn==0)
  {
    for(int i=0; i<counter; i++)
    {
      // When the swith is pressed, the function switchControl() 
      // will be called to change the variable turn value 
      if(digitalRead(SWITCHPIN)==HIGH) {
        switchControl();
        }
      digitalWrite(GREENPIN, HIGH);
      delay(LED_DELAY);
      digitalWrite(GREENPIN, LOW);
      delay(LED_DELAY);
    }

    counter++;
    delay(1000);
  }
}

void flashRed()
{
  int counter=1;
  
  while(turn==1)
  {
    for(int i=0; i<counter; i++)
    {
       // When the swith is pressed, the function switchControl() 
      // will be called to change the variable turn value 
      if(digitalRead(SWITCHPIN)==HIGH) {
        switchControl();
        }
      digitalWrite(REDPIN, HIGH);
      delay(LED_DELAY);
      digitalWrite(REDPIN, LOW);
      delay(LED_DELAY);
    }

    counter++;
    delay(1000);
  }
}

// It is to change the turn value from 0 to 1 or from 1 to 0
void switchControl() {
  turn = 1 - turn;
}

void loop() {
  if(turn == 0) {
    flashGreen();
  }

  if(turn == 1) {
    flashRed();
  }
}
