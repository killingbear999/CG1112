#define buttonPin 2
#define LEDPIN 12

static volatile int onOff = 0;

void switchISR() {
  onOff = 1 - onOff;
}

void setup() {
  attachInterrupt(digitalPinToInterrupt(buttonPin), switchISR, RISING);
  pinMode(LEDPIN, OUTPUT);
}

void loop() {
  if(onOff == 0) {
    digitalWrite(LEDPIN, LOW);
  } else {
    digitalWrite(LEDPIN, HIGH);
  }
}
