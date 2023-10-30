#include <Arduino.h>
int temp=10;
int prog=0;

void setup() {
  Serial.begin(9600);
   Serial.print("prog.val=");
    Serial.print(prog);
    Serial.write(0xff);
    Serial.write(0xff);
    Serial.write(0xff);
}

void loop() 

{
  char c = Serial.read();

  if (c == 65) 
  {
    prog +=5;
    Serial.print("prog.val=");
    Serial.print(prog);
    Serial.write(0xff);
    Serial.write(0xff);
    Serial.write(0xff);
  }

  if (c == 42) 
  {
    temp +=10;
    Serial.print("temp.val=");
    Serial.print(temp);
    Serial.write(0xff);
    Serial.write(0xff);
    Serial.write(0xff);
  }
  
}