/*
Demo for 4-Digit Display or 4-Digit Display(D4056B)
by Melab
Demo Function: Display the decimals on the digital tube.
http://www.dadimall.co.kr
http://www.daduino.co.kr
*/

#include "TM1637.h"
#define CLK 4//pins definitions for the module and can be changed to other ports       
#define DIO 2
TM1637 disp(CLK,DIO);
void setup()
{
  disp.set(BRIGHT_TYPICAL);//BRIGHT_TYPICAL = 2,BRIGHT_DARKEST = 0,BRIGHTEST = 7;
  disp.init(D4056A);//D4056A is the type of the module
}
void loop()
{
  disp.display(0.999);
  delay(1000);
  disp.display(999);
  delay(1000);
  disp.display(-0.99);
  delay(1000);
  disp.display(999.9);
  delay(1000);
  disp.display(9999);
  delay(1000);
  disp.display(-9.99);
  delay(1000);
  disp.display(0.9);
  delay(1000);
}

