#include <stdint.h>


 int16_t x=12, y=13;

 void setup(){

  Serial.begin(9600);
  delay(5000);
 }

 void loop(){
  x++;
  y++;
  float servo_x = 90, servo_y=90;

  Serial.write((char*)&x, 2);
  Serial.write((char*)&y, 2);
  Serial.write((char*)&servo_x, 4);
  Serial.write((char*)&servo_y, 4);

  delay(500);
 }

