#include <LiquidCrystal.h>

#define MESURE_PIN  A4
#define LED_POWER   13

#define COV_RATIO   0.2      //ug/mmm / mv
#define NO_DUST_VOLTAGE 400  //mv
#define SYS_VOLTAGE 5000        

float density, voltage;
int adcvalue;

// set the LCD address to 0x3F for a 16 chars and 2 line display
LiquidCrystal lcd(4, 6, 9, 10, 11, 12); 
char sLcdBuffer[16];

int Filter(int m){
  static int flag_first = 0, _buff[10], sum;
  const int _buff_max = 10;
  int i;
  
  if(flag_first == 0){
    flag_first = 1;

    for(i = 0, sum = 0; i < _buff_max; i++){
      _buff[i] = m;
      sum += _buff[i];
    }
    return m;
  } else {
    sum -= _buff[0];
    for(i = 0; i < (_buff_max - 1); i++){
      _buff[i] = _buff[i + 1];
    }
    _buff[9] = m;
    sum += _buff[9];
    
    i = sum / 10.0;
    return i;
  }
}

void setup() {
  Serial.begin(115200);
  
  //LCD Display
  lcd.begin(16,2);
  lcd.setCursor(0, 0);
  lcd.print("==KSND PROJECT==");
  lcd.setCursor(0, 1);
  lcd.print("System SW2 ykp74");
  delay(1000);
  lcd.clear();
  pinMode(LED_POWER,OUTPUT);
  digitalWrite(LED_POWER, LOW);  
}

void loop() {
  digitalWrite(LED_POWER, HIGH);
  delayMicroseconds(280);
  adcvalue = analogRead( MESURE_PIN);
  digitalWrite(LED_POWER, LOW);
  
  adcvalue = Filter(adcvalue);

  voltage = (SYS_VOLTAGE / 1024.0) * adcvalue * 11;

  if(voltage >= NO_DUST_VOLTAGE){
    voltage -= NO_DUST_VOLTAGE;
    density = voltage * COV_RATIO;
  } else {
    density = 0;
  }

  //memset( sLcdBuffer,0,sizeof(char)*16);
  //sprintf( sTemp," %4d", density);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Concentration");
  lcd.setCursor(0, 1);
  lcd.print("ug/m3 : ");
  lcd.setCursor(8, 1);
  lcd.print(density);

  delay(1000);
}
