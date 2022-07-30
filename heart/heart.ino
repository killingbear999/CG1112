#define numLEDs 7

int arrLEDs[] = {2, 5, 7, 8, 6, 3, 4};
void setup() {
  int i;
  for(i=0;i<=numLEDs;i=i+1) {
    pinMode(arrLEDs[i], OUTPUT);
  }
  /*for(i=0;i<=numLEDs;i=i+1) {
    pinMode(arrWhite[i], OUTPUT);
  }*/
}

void loop() {
  int i,value;
  for(i=0;i<=numLEDs;i=i+1) {
    digitalWrite(arrLEDs[i], HIGH);
    delay(500);
    /*digitalWrite(arrLEDs[i], LOW);
    delay(1000);*/
  }
  /*for(i=0;i<=numLEDs;i=i+1) {
    digitalWrite(arrWhite[i], HIGH);
  }*/
  /*for(i=0;i<=numLEDs;i=i+1) {
    value = analogRead(arrWhite[i]);
    analogWrite(arrWhite[i], 10);
    delay(100);
  }*/
  analogWrite(9, 10);
  analogWrite(10, 10);
  analogWrite(11, 10);
 
}
