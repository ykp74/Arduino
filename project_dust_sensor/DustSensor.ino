/*********************************************************************************************************

먼지센서 키트 예제 프로그램

연결도 
* Dust Sensor
- VCC => VCC
- GND => GND
- AOUT => A0
- ILED => D7

* I2C LCD
- VCC => VCC
- GND => GND
- SDA => A4
- SCL => A5

*********************************************************************************************************/
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define        COV_RATIO                       0.2            //ug/mmm / mv
#define        NO_DUST_VOLTAGE                 400            //mv
#define        SYS_VOLTAGE                     5000           

// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 16, 2);  // I2C LCD 객체 선언

/*
I/O define
*/
const int iled = 7;                                            //drive the led of sensor
const int vout = A0;                                            //analog input

/*
variable
*/
float old_density, density, voltage;
int   adcvalue;

/*
private function
*/
 
int Filter(int m)
{
  static int flag_first = 0, _buff[10], sum;
  const int _buff_max = 10;
  int i;
  
  if(flag_first == 0)
  {
    flag_first = 1;

    for(i = 0, sum = 0; i < _buff_max; i++)
    {
      _buff[i] = m;
      sum += _buff[i];
    }
    return m;
  }
  else
  {
    sum -= _buff[0];
    for(i = 0; i < (_buff_max - 1); i++)
    {
      _buff[i] = _buff[i + 1];
    }
    _buff[9] = m;
    sum += _buff[9];
    
    i = sum / 10.0;
    return i;
  }
}


void setup(void)
{
  pinMode(iled, OUTPUT);
  digitalWrite(iled, LOW);                                     //iled default closed

  
  Serial.begin(9600);                                         //send and receive at 9600 baud
  Serial.print("*********************************** Eleparts ***********************************\n");

   // initialize the LCD
  lcd.begin(); // lcd를 사용을 시작합니다.

}

void loop(void)
{
  /*
  get adcvalue
  */
  digitalWrite(iled, HIGH);
  delayMicroseconds(280);
  adcvalue = analogRead(vout);
  digitalWrite(iled, LOW);
  
  adcvalue = Filter(adcvalue);
  
  /*
  covert voltage (mv)
  */
  voltage = (SYS_VOLTAGE / 1024.0) * adcvalue * 11;
  
  /*
  voltage to density
  */
  if(voltage >= NO_DUST_VOLTAGE)
  {
    voltage -= NO_DUST_VOLTAGE;
    
    density = voltage * COV_RATIO;
  }
  else
    density = 0;
    
  /*
  display the result
  */

  
  Serial.print("The current dust concentration is: ");
  Serial.print(density);
  Serial.print("ug/m3\n");  


 
 lcd.backlight(); // backlight를 On 시킵니다.
 lcd.setCursor(0,0);
 lcd.print("Dust Sensor"); // 화면에 Dust Sensor를 출력합니다.
 lcd.setCursor(0,1);
 lcd.print("PM2.5:");
 lcd.setCursor(11,1);
 lcd.print("ug/m3");
 lcd.setCursor(6,1);
 lcd.print(density);
 
 delay(5000);
 
 lcd.clear();
}























