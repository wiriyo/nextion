void setup() {
  pinMode(18,OUTPUT);
  pinMode(19,OUTPUT);
  pinMode(2,OUTPUT);

}

void loop() {
  digitalWrite(18,1);
  digitalWrite(19,1);
  digitalWrite(2,1);
  delay(1000);
  digitalWrite(18,0);
  digitalWrite(19,0);
  digitalWrite(2,0);
  delay(1000);
}
