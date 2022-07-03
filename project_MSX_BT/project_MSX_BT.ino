#include <DueTimer.h>

#define SUPPORT_PS4
#define SUPPORT_MVS
//#define SUPPORT_MSX
#define _DEBUG

#ifdef SUPPORT_PS4
#include <PS4BT.h>
#endif
#include <usbhub.h>

// Satisfy the IDE, which needs to see the include statment in the ino too.
#ifdef dobogusinclude
# include <spi4teensy3.h>
# include <SPI.h>
#endif

#include "TM1651.h"
#define CLK 3 //pins definitions for TM1651 and can be changed to other ports       
#define DIO 2 
TM1651 Display(CLK,DIO);

//Connected to Device: 28:C1:3C:D1:6F:52
// Do not use pin 9, 10

USB Usb;
//USBHub Hub1(&Usb); // Some dongles have a hub inside
BTD Btd(&Usb); // You have to create the Bluetooth Dongle instance like so

#ifdef SUPPORT_PS4
/* You can create the instance of the PS4BT class in two ways */
// This will start an inquiry and then pair with the PS4 controller - you only have to do this once
// You will need to hold down the PS and Share button at the same time, the PS4 controller will then start to blink rapidly indicating that it is in pairing mode
PS4BT PS4(&Btd, PAIR);
// After that you can simply create the instance like so and then press the PS button on the device
//PS4BT PS4(&Btd);
#endif

const int PIN_IND_LED  = 29;

#ifdef SUPPORT_MSX
const int PIN_UP      = 8;    // MSX up
const int PIN_DOWN    = 11;   // MSX down   //9-> 10
const int PIN_LEFT    = 4;    // MSX left
const int PIN_RIGHT   = 5;    // MSX right

const int PIN_T1      = 6;    // MSX TR1
const int PIN_T2      = 7;    // MSX TR2
#endif

#ifdef SUPPORT_MVS
const int PIN_UP      = 30;
const int PIN_DOWN    = 31;
const int PIN_LEFT    = 32;
const int PIN_RIGHT   = 33;

const int PIN_T1      = 36;
const int PIN_T2      = 37;
const int PIN_T3      = 38;
const int PIN_T4      = 39;
const int PIN_T5      = 40;
const int PIN_T6      = 41;

const int PIN_START   = 26; 
const int PIN_CREDIT  = 27;
#endif

volatile bool isUp, isDown, isLeft, isRight; 
volatile bool isA, isB, isC, isD, isE, isF;
volatile bool isStart, isSelect;

volatile bool pushUp, pushDown, pushLeft, pushRight; 

int bat_level = 0;
bool isConnect = false;

void get_BatLevel_handler(void){
  int batLvl =  PS4.getBatteryLevel();

  if( bat_level != batLvl){
    bat_level = batLvl;
    
    if( bat_level > 8 ){
      PS4.setLed(Blue);
    } else if( bat_level > 6 ){
      PS4.setLed(Green);
    } else if( bat_level > 4 ){
      PS4.setLed(Yellow);
    } else {
      PS4.setLed(Red);
    } 
  }
  Serial.print("get_BatLevel_handler : ");
  Serial.println(batLvl);
}

void ind_led_handler(void){
    // Toggle LED
    digitalWrite( PIN_IND_LED, digitalRead( PIN_IND_LED ) ^ 1 );
}
void ind_led_eable(bool enable){
    digitalWrite( PIN_IND_LED, enable );
}

void setup()
{
  Serial.begin(115200);

#if !defined(__MIPSEL__)
  while (!Serial); // Wait for serial port to connect - used on Leonardo, Teensy and other boards with built-in USB CDC serial connection
#endif

  if (Usb.Init() == -1) {
    Serial.print(F("\r\nOSC did not start"));
    while (1); // Halt
  }
  isConnect = false;
  pinMode( PIN_IND_LED, OUTPUT);
  
  // usage define
  pinMode( PIN_UP,     INPUT);
  pinMode( PIN_DOWN,   INPUT);
  pinMode( PIN_LEFT,   INPUT);
  pinMode( PIN_RIGHT,  INPUT);
  pinMode( PIN_T1,     INPUT);
  pinMode( PIN_T2,     INPUT);
#ifdef SUPPORT_MVS
  pinMode( PIN_T3,     INPUT);
  pinMode( PIN_T4,     INPUT);
  pinMode( PIN_T5,     INPUT);
  pinMode( PIN_T6,     INPUT);
  pinMode( PIN_START,  INPUT);
  pinMode( PIN_CREDIT, INPUT);
#endif
  digitalWrite( PIN_IND_LED, LOW);

  // initialize
  digitalWrite(PIN_UP,    LOW);
  digitalWrite(PIN_DOWN,  LOW);
  digitalWrite(PIN_LEFT,  LOW);
  digitalWrite(PIN_RIGHT, LOW);
  digitalWrite(PIN_T1,    LOW);
  digitalWrite(PIN_T2,    LOW);
#ifdef SUPPORT_MVS
  digitalWrite(PIN_T3,    LOW);
  digitalWrite(PIN_T4,    LOW);
  digitalWrite(PIN_T5,    LOW);
  digitalWrite(PIN_T6,    LOW);
  digitalWrite(PIN_START, LOW);
  digitalWrite(PIN_CREDIT,LOW);
#endif

  isUp = isDown = isLeft = isRight = false;
  isA = isB = isC = isD = isE = isF = false;

  Timer3.attachInterrupt(get_BatLevel_handler).start(5000000);  // Calls every 5000ms
  //Timer5.attachInterrupt(ind_led_handler).start(500000);  // Calls every 1s
  Display.displayClear();
  Display.displaySet(2);//BRIGHT_TYPICAL = 2,BRIGHT_DARKEST = 0,BRIGHTEST = 7;
  Display.displayNum(0,0);
  Serial.print(F("\r\nPS4 Bluetooth Library Started"));
}

void loop()
{
  Usb.Task();

#ifdef SUPPORT_PS4
  if(!PS4.connected())
#endif
  {
    isConnect = false;
    ind_led_eable(false);
    return;
  } else {
    isConnect = true;
    ind_led_eable(true);
  }

#ifdef SUPPORT_PS4
  isUp    = ( PS4.getAnalogHat(LeftHatY) < 50 || PS4.getButtonPress(UP) );
  isDown  = ( PS4.getAnalogHat(LeftHatY) > 200 || PS4.getButtonPress(DOWN) );
  isLeft  = ( PS4.getAnalogHat(LeftHatX) < 50 || PS4.getButtonPress(LEFT) );
  isRight = ( PS4.getAnalogHat(LeftHatX) > 200 || PS4.getButtonPress(RIGHT) );
  isA = ( PS4.getButtonPress(CROSS) );
  isB = ( PS4.getButtonPress(CIRCLE) );
  isC = ( PS4.getButtonPress(SQUARE) );
  isD = ( PS4.getButtonPress(TRIANGLE) );
  isE = ( PS4.getButtonPress(L1) );
  isF = ( PS4.getButtonPress(R1) );
  isStart  = ( PS4.getButtonPress(OPTIONS) );
  isSelect = ( PS4.getButtonPress(SHARE) );
  
  if(PS4.getButtonClick(PS)){
    //Serial.println(F("\r\nPS"));
    Serial.println("Disconnect PS4 Pad!!");
    PS4.disconnect();
  }
#endif

  pinMode( PIN_UP, isUp ? OUTPUT : INPUT);  //INPUT : floating
  pinMode( PIN_DOWN, isDown ? OUTPUT : INPUT);
  pinMode( PIN_LEFT, isLeft ? OUTPUT : INPUT);
  pinMode( PIN_RIGHT, isRight ? OUTPUT : INPUT);
  pinMode( PIN_T1, (isA) ? OUTPUT : INPUT);
  pinMode( PIN_T2, (isB) ? OUTPUT : INPUT);
#ifdef SUPPORT_MVS
  pinMode( PIN_T3, (isC) ? OUTPUT : INPUT);
  pinMode( PIN_T4, (isD) ? OUTPUT : INPUT);
  pinMode( PIN_T5, (isE) ? OUTPUT : INPUT);
  pinMode( PIN_T6, (isF) ? OUTPUT : INPUT);
  pinMode( PIN_START, isStart ? OUTPUT : INPUT);
  pinMode( PIN_CREDIT, isSelect ? OUTPUT : INPUT);
#endif

#ifdef _DEBUG
  if (isUp){
    if(!pushUp){
      Serial.print(F("\r\nUp"));
      //Display.displayNum(0,1); 
      pushUp = true;
    }
  } else {
    pushUp = false;
  }

  if (isDown){
    if(!pushDown){
      Serial.print(F("\r\nDown"));
      //Display.displayNum(0,2); 
      pushDown = true;
    }
  } else {
    pushDown = false;
  }

  if(isLeft){
    if(!pushLeft){
      Serial.print(F("\r\nLeft"));
      //Display.displayNum(0,3); 
       pushLeft =true;
    }
  } else {
    pushLeft = false;
  }

  if (isRight){
    if(!pushRight){
      Serial.print(F("\r\nRight"));
      //Display.displayNum(0,4); 
      pushRight = true;
    }
  } else {
    pushRight = false;
  }

//  if (isA){
//    Serial.print(F("\r\nAkey"));
//    //PS4.setLedFlash(10, 10); // Set it to blink rapidly
//  }
//  if (isB){
//    Serial.print(F("\r\nBkey"));
//    //PS4.setLedFlash(0, 0); // Turn off blinking
//  }
//  if (isStart){
//    Serial.print(F("\r\nSTART key"));
//    //PS4.setLedFlash(10, 10); // Set it to blink rapidly
//  }
//  if (isSelect){
//    Serial.print(F("\r\nSelect key"));
//    //PS4.setLedFlash(0, 0); // Turn off blinking
//  }
#endif
}
