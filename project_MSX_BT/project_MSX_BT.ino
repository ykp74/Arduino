#include <DueTimer.h>

#define SUPPORT_PS4
#define SUPPORT_MVS
//#define SUPPORT_MSX
//#define _DEBUG

#ifdef SUPPORT_PS4
#include <PS4BT.h>
#endif
#include <usbhub.h>

// Satisfy the IDE, which needs to see the include statment in the ino too.
#ifdef dobogusinclude
# include <spi4teensy3.h>
# include <SPI.h>
#endif

// #include "TM1651.h"
// #define CLK 3 //pins definitions for TM1651 and can be changed to other ports       
// #define DIO 2 
// TM1651 Display(CLK,DIO);

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

// 상태 플래그
bool isConnected = false;
bool psButtonLastState = false;

#define PIN_IND_LED   41

#ifdef SUPPORT_MSX
const int PIN_UP      = 8;    // MSX up
const int PIN_DOWN    = 11;   // MSX down   //9-> 10
const int PIN_LEFT    = 4;    // MSX left
const int PIN_RIGHT   = 5;    // MSX right

const int PIN_T1      = 6;    // MSX TR1
const int PIN_T2      = 7;    // MSX TR2
#endif

#ifdef SUPPORT_MVS
const int PIN_UP      = 42;
const int PIN_DOWN    = 43;
const int PIN_LEFT    = 44;
const int PIN_RIGHT   = 45;

const int PIN_T1      = 48;
const int PIN_T2      = 49;
const int PIN_T3      = 50;
const int PIN_T4      = 51;
const int PIN_T5      = 52;
const int PIN_T6      = 53;

const int PIN_START   = 38; 
const int PIN_CREDIT  = 39;
#endif

volatile bool isUp, isDown, isLeft, isRight; 
volatile bool isA, isB, isC, isD, isE, isF;
volatile bool isStart, isSelect;
volatile bool pushUp, pushDown, pushLeft, pushRight, pushA, pushB, pushC, pushD, pushStart, pushSelect; 

int bat_level = 0;

void get_BatLevel_handler(void)
{
    int batLvl = PS4.getBatteryLevel();

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
#ifdef _DEBUG
    Serial.print("get_BatLevel_handler : ");
    Serial.println(batLvl);
#endif
}

void ind_led_handler(void)
{
    // Toggle LED
    digitalWrite( PIN_IND_LED, digitalRead( PIN_IND_LED ) ^ 1 );
}

void ind_led_eable(bool enable)
{
    static boolean isOn = false;

    if(enable == true){
        if(!isOn){
            isOn = true;
            digitalWrite( PIN_IND_LED, enable );
            Serial.println("Connect PS Pad!!");
        }
    } else {
        if(isOn){
            isOn = false;
            digitalWrite( PIN_IND_LED, enable );
        }
    }
}

void ps_button_handler(void)
{
    bool psButtonState = PS4.getButtonPress(PS);

    // PS 버튼이 눌린 상태에서만 처리 (토글 방식)
    if (psButtonState && !psButtonLastState) {
        if (isConnected) {
            // 연결 해제
            PS4.disconnect();
            Serial.println("PS4 컨트롤러 연결 해제 시도 중...");
        } else {
            // 연결 시도 (Bluetooth 페어링은 자동 처리됨)
            Serial.println("PS4 컨트롤러 연결 시도 중...");
        }
    }

    // 버튼 상태 업데이트
    psButtonLastState = psButtonState;
}

#ifdef _DEBUG
void debug_log(void)
{
    if(isUp){
        if(!pushUp){
            Serial.print(F("\r\nUp"));
            //Display.displayNum(0,1); 
            pushUp = true;
        }
    } else {
        pushUp = false;
    }

    if(isDown){
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

    if(isRight){
        if(!pushRight){
            Serial.print(F("\r\nRight"));
            //Display.displayNum(0,4); 
            pushRight = true;
        }
    } else {
        pushRight = false;
    }

    if(isA){
        if(!pushA){
            Serial.print(F("\r\nA Trigger key"));
            //PS4.setLedFlash(10, 10); // Set it to blink rapidly
            pushA = true;
        }
    } else {
        pushA = false;
    }

    if(isB){
        if(!pushB){
            Serial.print(F("\r\nB Trigger key"));
            //PS4.setLedFlash(10, 10); // Set it to blink rapidly
            pushB = true;
        }
    } else {
        pushB = false;
    }

    if(isC){
        if(!pushC){
            Serial.print(F("\r\nC Trigger key"));
            //PS4.setLedFlash(10, 10); // Set it to blink rapidly
            pushC = true;
        }
    } else {
        pushC = false;
    }
        
    if(isD){
        if(!pushD){
            Serial.print(F("\r\nD Trigger key"));
            //PS4.setLedFlash(10, 10); // Set it to blink rapidly
            pushD = true;
        }
    } else {
        pushD = false;
    }
  
    if(isStart){
        if(!pushStart){
            Serial.print(F("\r\nD nSTART key"));
            //PS4.setLedFlash(10, 10); // Set it to blink rapidly
            pushStart = true;
        }
    } else {
        pushStart = false;
    }

    if(isSelect){
        if(!pushSelect){
            Serial.print(F("\r\nD nSelect key"));
            //PS4.setLedFlash(10, 10); // Set it to blink rapidly
            pushSelect = true;
        }
    } else {
        pushSelect = false;
    }
}
#endif /* _DEBUG */

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
    // Display.displayClear();
    // Display.displaySet(2);//BRIGHT_TYPICAL = 2,BRIGHT_DARKEST = 0,BRIGHTEST = 7;
    // Display.displayNum(0,0);

    Serial.print(F("\r\nPS4 Bluetooth Library Started"));
    ind_led_eable(true);
    delay(1000); // 1000ms(1초) 대기
    ind_led_eable(false);
}

void loop()
{
    Usb.Task();

#ifdef SUPPORT_PS4
    if(PS4.connected()){ 
        if (!isConnected) {
          isConnected = true;
          //Connect PS4 Controller
          ind_led_eable(true);
        }

        // PS 버튼으로 연결 시도
        //ps_button_handler();

        isUp    = ( PS4.getAnalogHat(LeftHatY) < 50 || PS4.getButtonPress(UP) );
        isDown  = ( PS4.getAnalogHat(LeftHatY) > 200 || PS4.getButtonPress(DOWN) );
        isLeft  = ( PS4.getAnalogHat(LeftHatX) < 50 || PS4.getButtonPress(LEFT) );
        isRight = ( PS4.getAnalogHat(LeftHatX) > 200 || PS4.getButtonPress(RIGHT) );
        isA = ( PS4.getButtonPress(SQUARE));
        isB = ( PS4.getButtonPress(TRIANGLE));
        isC = ( PS4.getButtonPress(CROSS));
        isD = ( PS4.getButtonPress(CIRCLE));
        isE = ( PS4.getButtonPress(L1) );
        isF = ( PS4.getButtonPress(R1) );
        isStart  = ( PS4.getButtonPress(OPTIONS) );
        isSelect = ( PS4.getButtonPress(SHARE) );   
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
        debug_log();
#endif
    } else {
        //Disconnect PS4 Controller
        if (isConnected) {
            isConnected = false;
            Serial.println("PS4 컨트롤러 연결이 해제되었습니다!");
            ind_led_eable(false);
        }
    }  
}
