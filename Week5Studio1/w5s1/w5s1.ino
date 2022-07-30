#define REDPIN    11
#define GREENPIN  12
#define SWITCHPIN 2

#define LED_DELAY   100

// This variable decides which LED's turn it is to flash.
// 0 = green, 1 = red
static volatile int turn=0;

void setup() {
  // put your setup code here, to run once:

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
      digitalWrite(REDPIN, HIGH);
      delay(LED_DELAY);
      digitalWrite(REDPIN, LOW);
      delay(LED_DELAY);
    }

    counter++;
    delay(1000);
  }
}
void loop() {
  // put your main code here, to run repeatedly:

  if(turn == 0)
    flashGreen();

  if(turn == 1)
    flashRed();
}
