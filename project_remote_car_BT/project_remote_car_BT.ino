/*
 Example sketch for the PS3 Bluetooth library - developed by Kristian Lauszus
 For more information visit my blog: http://blog.tkjelectronics.dk/ or
 send me an e-mail:  kristianl@tkjelectronics.com
 */
#define FEATURE_PS4
#define _DEBUG_LOG

#ifdef FEATURE_PS4
#include <PS4BT.h>
#else
#include <PS3BT.h>
#endif
#include <usbhub.h>

// Satisfy the IDE, which needs to see the include statment in the ino too.
#ifdef dobogusinclude
#include <spi4teensy3.h>
#endif
#include <SPI.h>

USB Usb;
//USBHub Hub1(&Usb);    //Some dongles have a hub inside

// You have to create the Bluetooth Dongle instance like so
BTD Btd(&Usb);

#ifdef FEATURE_PS4
/* You can create the instance of the PS4BT class in two ways */
// This will start an inquiry and then pair with the PS4 controller - you only have to do this once
// You will need to hold down the PS and Share button at the same time, 
// the PS4 controller will then start to blink rapidly indicating that it is in pairing mode
//PS4BT PS4(&Btd, PAIR);
// After that you can simply create the instance like so and then press the PS button on the device
PS4BT PS4(&Btd);
#else
/* You can create the instance of the class in two ways */
PS3BT PS3(&Btd, 0x00, 0x15, 0x83, 0x3D, 0x0A, 0x57); 
// This will also store the bluetooth address - this can be obtained from the dongle when running the sketch
#endif /* FEATURE_PS4 */

bool printAngle = false;
#ifdef FEATURE_PS4
bool printTouch = false;
uint8_t oldL2Value, oldR2Value = 0;
#endif /* FEATURE_PS4 */

const int PIN_UP      = 2;    // MSX up
const int PIN_DOWN    = 3;    // MSX down
const int PIN_LEFT    = 4;    // MSX left
const int PIN_RIGHT   = 5;    // MSX right
const int PIN_T1      = 6;    // MSX TR1
const int PIN_T2      = 7;    // MSX TR2
const int LED_0       = 13;

static bool ledOn = false;

volatile bool isUp, isDown, isLeft, isRight; 
volatile bool isA, isB, isC, isD, isE, isF;
volatile bool isStart, isSelect;

int gLSpeed = 0;
int gRSpeed = 0;

#define Dir1Pin_A   7         // 제어신호 1핀
#define Dir2Pin_A   8         // 제어신호 2핀
#define SpeedPin_A  5         // PWM제어를 위한 핀

#define Dir1Pin_B   2         // 제어신호 1핀
#define Dir2Pin_B   4         // 제어신호 2핀
#define SpeedPin_B  6         // PWM제어를 위한 핀

#define DEFUALT_SPEED   220     /* Default Motor Speed */
#define INNNER_SPEED    100     /* Inner Motor Speed When the device turn conner. */

typedef enum direction {
    DIR_FORWARD,
    DIR_REVERSE,
    DIR_LEFT,
    DIR_RIGHT,
    DIR_LEFT_FORWARD,
    DIR_RIGHT_FORWARD,
    DIR_LEFT_REVERSE,
    DIR_RIGHT_REVERSE,
    DIR_MAX
} _tDirection;

typedef struct  {
    direction dir;
    int leftSpeed;
    int rightSpeed;
} deviceStatus ;

static deviceStatus status;

#if 0
#define TOUCH_UP(x)  digitalWrite(PIN_UP, x)
#define TOUCH_DOWN(x)  digitalWrite(PIN_DOWN, x)
#define TOUCH_LEFT(x)  digitalWrite(PIN_LEFT, x)
#define TOUCH_RIGHT(x)  digitalWrite(PIN_RIGHT, x)
#define TOUCH_T1(x)  digitalWrite(PIN_T1, x)
#define TOUCH_T2(x)  digitalWrite(PIN_T2, x)
#else
#define TOUCH_UP(x) x
#define TOUCH_DOWN(x) x
#define TOUCH_LEFT(x) x
#define TOUCH_RIGHT(x) x
#define TOUCH_T1(x) x
#define TOUCH_T2(x) x
#endif

// void ledHandler(){
//   ledOn = !ledOn;
//   digitalWrite(LED_0, ledOn); // Led on, off, on, off...
// }

void motorDrive_dir(direction dir) 
{
    if( status.dir == dir ){
        return;
    }
  
    Serial.print(F("\r\nmotorDrive_dir: "));
    Serial.print(dir);
    //ledHandler();

    switch (dir){
        case DIR_FORWARD:
        case DIR_LEFT_FORWARD:
        case DIR_RIGHT_FORWARD:
            digitalWrite(Dir1Pin_A, LOW);
            digitalWrite(Dir2Pin_A, HIGH);
            digitalWrite(Dir1Pin_B, LOW);
            digitalWrite(Dir2Pin_B, HIGH); 
            break;
        case DIR_REVERSE:
        case DIR_LEFT_REVERSE:
        case DIR_RIGHT_REVERSE:
            digitalWrite(Dir1Pin_A, HIGH);
            digitalWrite(Dir2Pin_A, LOW);
            digitalWrite(Dir1Pin_B, HIGH);
            digitalWrite(Dir2Pin_B, LOW);
            break;
        case DIR_LEFT:
            digitalWrite(Dir1Pin_A, LOW);
            digitalWrite(Dir2Pin_A, HIGH);
            digitalWrite(Dir1Pin_B, HIGH);
            digitalWrite(Dir2Pin_B, LOW);
            break;
        case DIR_RIGHT:
            digitalWrite(Dir1Pin_A, HIGH);
            digitalWrite(Dir2Pin_A, LOW);
            digitalWrite(Dir1Pin_B, LOW);
            digitalWrite(Dir2Pin_B, HIGH);
            break;
        default:
            break;
    }
    status.dir = dir;
}

void motorDrive_control(int leftSpeed, int rightSpeed )
{
    if ( status.leftSpeed == leftSpeed && status.rightSpeed == rightSpeed ){
        return;
    }
    Serial.print(F("\r\nmotorDrive_control: "));
    Serial.print(leftSpeed);

    analogWrite(SpeedPin_A, leftSpeed);
    analogWrite(SpeedPin_B, rightSpeed);
    status.leftSpeed = leftSpeed;
    status.rightSpeed = rightSpeed;
}

void setup() {
    Serial.begin(115200);
#if !defined(__MIPSEL__)
    while (!Serial); // Wait for serial port to connect - used on Leonardo, Teensy and other boards with built-in USB CDC serial connection
#endif

    pinMode(Dir1Pin_A, OUTPUT);     // 제어 1번핀 출력모드 설정
    pinMode(Dir2Pin_A, OUTPUT);     // 제어 2번핀 출력모드 설정
    pinMode(Dir1Pin_B, OUTPUT);     // 제어 1번핀 출력모드 설정
    pinMode(Dir2Pin_B, OUTPUT);     // 제어 2번핀 출력모드 설정

    digitalWrite(Dir1Pin_A, HIGH);  //모터가 정방향으로 회전
    digitalWrite(Dir2Pin_A, HIGH);
    digitalWrite(Dir1Pin_B, HIGH);  //모터가 정방향으로 회전
    digitalWrite(Dir2Pin_B, HIGH);
    analogWrite(SpeedPin_A, 0);
    analogWrite(SpeedPin_B, 0);
  
    isUp = isDown = isLeft = isRight = isA = isB = isC = isD = isE = isF = false;

    status.dir = DIR_MAX;
    status.leftSpeed = status.rightSpeed = 0;

    if (Usb.Init() == -1) {
        Serial.print(F("\r\nOSC did not start"));
        while(1); //halt
    }
    //Timer3.attachInterrupt(ledHandler);
    //Timer3.start(5000); // Calls every 1000ms

    Serial.print(F("\r\nPS joypad Bluetooth Library Started"));
}

void loop() 
{
  Usb.Task();

#ifdef FEATURE_PS4
  if (PS4.connected()) {
    if (PS4.getAnalogHat(RightHatX) > 137 || PS4.getAnalogHat(RightHatX) < 117 || PS4.getAnalogHat(RightHatY) > 137 || PS4.getAnalogHat(RightHatY) < 117) {
        if (PS4.connected()) {}
            Serial.print(F("\r\nRightHatX: "));
            Serial.print(PS4.getAnalogHat(RightHatX));
            Serial.print(F("\tRightHatY: "));
            Serial.print(PS4.getAnalogHat(RightHatY));
        }
  }
#else
  if (PS3.PS3Connected || PS3.PS3NavigationConnected){
    if(PS3.getAnalogHat(RightHatX) > 145 || PS3.getAnalogHat(RightHatX) < 110 || PS3.getAnalogHat(RightHatY) > 145 || PS3.getAnalogHat(RightHatY) < 110) {
        if( PS3.PS3Connected) { // The Navigation controller only have one joystick
            Serial.print(F("\r\nRightHatX: "));
            Serial.print(PS3.getAnalogHat(RightHatX));
            Serial.print(F("\tRightHatY: "));
            Serial.print(PS3.getAnalogHat(RightHatY));
        }
    }
#endif

#ifdef FEATURE_PS4
    // Analog button values can be read from almost all buttons
    if (PS4.getAnalogButton(L2) ) {
        Serial.print(F("\r\nL2: "));
        Serial.print(PS4.getAnalogButton(L2));
    } else if (PS4.getButtonClick(PS)) {
        Serial.print(F("\r\nPS"));
        PS4.disconnect();
    } else {
        if( PS4.getAnalogButton(R2)) {
            Serial.print(F("\tR2: "));
            Serial.print(PS4.getAnalogButton(R2));
        }
 
        /* Direction Control */
        isUp    = TOUCH_UP( PS4.getAnalogHat(LeftHatY) < 110 || PS4.getButtonPress(UP) );
        isDown  = TOUCH_DOWN( PS4.getAnalogHat(LeftHatY) > 145 || PS4.getButtonPress(DOWN) );
        isLeft  = TOUCH_LEFT( PS4.getAnalogHat(LeftHatX) < 110 || PS4.getButtonPress(LEFT) );
        isRight = TOUCH_RIGHT( PS4.getAnalogHat(LeftHatX) > 145 || PS4.getButtonPress(RIGHT) );

        /* Trigger Button */
        isA = TOUCH_T1( PS4.getButtonPress(CROSS) );
        isB = TOUCH_T2( PS4.getButtonPress(CIRCLE) );
        isC = ( PS4.getButtonPress(SQUARE) );
        isD = ( PS4.getButtonPress(TRIANGLE) );

        /* Function Button */
        isE = ( PS4.getButtonPress(L1) );
        isF = ( PS4.getButtonPress(R1) );
        isStart = ( PS4.getButtonPress(START) );
        isSelect = ( PS4.getButtonPress(SELECT) );

#ifdef _DEBUG_LOG
        if(isA) Serial.print(F("\r\nCross"));
        if(isB) Serial.print(F("\r\nCircle"));
        if(isC) Serial.print(F("\r\nSquare"));
        if(isD) Serial.print(F("\r\nTraingle"));
        if(isE) Serial.print(F("\r\nL1"));
        if(isF) Serial.print(F("\r\nR1"));
#endif
        //Direction Control
        if( isUp && isLeft ){
            motorDrive_dir(DIR_LEFT_FORWARD);
            gLSpeed = INNNER_SPEED;
            gRSpeed = DEFUALT_SPEED + 30;
        } else if( isUp && isRight ){
            motorDrive_dir(DIR_RIGHT_FORWARD);
            gLSpeed = DEFUALT_SPEED + 30;
            gRSpeed = INNNER_SPEED;
        } else if( isDown && isLeft){
            motorDrive_dir(DIR_LEFT_REVERSE);
            gLSpeed = INNNER_SPEED;
            gRSpeed = DEFUALT_SPEED + 30;
        } else if( isDown && isRight){
            motorDrive_dir(DIR_RIGHT_REVERSE);
            gLSpeed = DEFUALT_SPEED + 30;
            gRSpeed = INNNER_SPEED;
        } else {
            if(isUp) {
                motorDrive_dir(DIR_FORWARD);
                gLSpeed = DEFUALT_SPEED;
                gRSpeed = DEFUALT_SPEED;
            } else if (isDown) {
                motorDrive_dir(DIR_REVERSE);
                gLSpeed = DEFUALT_SPEED;
                gRSpeed = DEFUALT_SPEED;
            } else if (isRight) {
                motorDrive_dir(DIR_RIGHT);
                gLSpeed = DEFUALT_SPEED;
                gRSpeed = DEFUALT_SPEED;
            } else if (isLeft) {
                motorDrive_dir(DIR_LEFT);
                gLSpeed = DEFUALT_SPEED;
                gRSpeed = DEFUALT_SPEED;
            } else {
                gLSpeed = gRSpeed = 0;
            }
        } 
        motorDrive_control(gLSpeed, gRSpeed);

#ifdef _DEBUG_LOG
        if(PS4.getButtonClick(L3)){
            Serial.print(F("\r\nL3"));
        }
        if(PS4.getButtonClick(R3)){
            Serial.print(F("\r\nR3"));
        }
#endif
        if (isStart) {
          printAngle = !printAngle;
        }
    }
 
#ifdef _DEBUG_LOG
    // Set this to 1 in order to see the angle of the controller
    if( printAngle ){
        Serial.print(F("\r\nPitch: "));
        Serial.print(PS4.getAngle(Pitch));
        Serial.print(F("\tRoll: "));
        Serial.print(PS4.getAngle(Roll));
    }
#endif

#else
    // Analog button values can be read from almost all buttons
    if (PS3.getAnalogButton(L2) ) {
        Serial.print(F("\r\nL2: "));
        Serial.print(PS3.getAnalogButton(L2));
    } else if (PS3.getButtonClick(PS)) {
        Serial.print(F("\r\nPS"));
        PS3.disconnect();
    } else {
        if( PS3.getAnalogButton(R2)) {
            Serial.print(F("\tR2: "));
            Serial.print(PS3.getAnalogButton(R2));
        }
 
        /* Direction Control */
        isUp    = TOUCH_UP( PS3.getAnalogHat(LeftHatY) < 110 || PS3.getButtonPress(UP) );
        isDown  = TOUCH_DOWN( PS3.getAnalogHat(LeftHatY) > 145 || PS3.getButtonPress(DOWN) );
        isLeft  = TOUCH_LEFT( PS3.getAnalogHat(LeftHatX) < 110 || PS3.getButtonPress(LEFT) );
        isRight = TOUCH_RIGHT( PS3.getAnalogHat(LeftHatX) > 145 || PS3.getButtonPress(RIGHT) );

        /* Trigger Button */
        isA = TOUCH_T1( PS3.getButtonPress(CROSS) );
        isB = TOUCH_T2( PS3.getButtonPress(CIRCLE) );
        isC = ( PS3.getButtonPress(SQUARE) );
        isD = ( PS3.getButtonPress(TRIANGLE) );

        /* Function Button */
        isE = ( PS3.getButtonPress(L1) );
        isF = ( PS3.getButtonPress(R1) );
        isStart = ( PS3.getButtonPress(START) );
        isSelect = ( PS3.getButtonPress(SELECT) );

#ifdef _DEBUG_LOG
        if(isA)
            Serial.print(F("\r\nCross"));
        if(isB)
            Serial.print(F("\r\nCircle"));
        if(isC)
            Serial.print(F("\r\nSquare"));
        if(isD)
            Serial.print(F("\r\nTraingle"));
        if(isE)
            Serial.print(F("\r\nL1"));
        if(isF)
            Serial.print(F("\r\nR1"));
#endif

        //Direction Control
        if( isUp && isLeft ){
            motorDrive_dir(DIR_LEFT_FORWARD);
            gLSpeed = INNNER_SPEED;
            gRSpeed = DEFUALT_SPEED + 30;
        } else if( isUp && isRight ){
            motorDrive_dir(DIR_RIGHT_FORWARD);
            gLSpeed = DEFUALT_SPEED + 30;
            gRSpeed = INNNER_SPEED;
        } else if( isDown && isLeft){
            motorDrive_dir(DIR_LEFT_REVERSE);
            gLSpeed = INNNER_SPEED;
            gRSpeed = DEFUALT_SPEED + 30;
        } else if( isDown && isRight){
            motorDrive_dir(DIR_RIGHT_REVERSE);
            gLSpeed = DEFUALT_SPEED + 30;
            gRSpeed = INNNER_SPEED;
        } else {
            if(isUp) {
                // Serial.print(F("\r\nUp"));
                // if (PS3.PS3Connected) {
                //   PS3.setLedOff();
                //   PS3.setLedOn(LED4);
                // }
                motorDrive_dir(DIR_FORWARD);
                gLSpeed = DEFUALT_SPEED;
                gRSpeed = DEFUALT_SPEED;
            } else if (isDown) {
                // Serial.print(F("\r\nDown"));
                // if (PS3.PS3Connected) {
                //   PS3.setLedOff();
                //   PS3.setLedOn(LED2);
                // }
                motorDrive_dir(DIR_REVERSE);
                gLSpeed = DEFUALT_SPEED;
                gRSpeed = DEFUALT_SPEED;
            } else if (isRight) {
                // Serial.print(F("\r\nRight"));
                // if (PS3.PS3Connected) {
                //   PS3.setLedOff();
                //   PS3.setLedOn(LED1);
                // }
                motorDrive_dir(DIR_RIGHT);
                gLSpeed = DEFUALT_SPEED;
                gRSpeed = DEFUALT_SPEED;
            } else if (isLeft) {
                // Serial.print(F("\r\nLeft"));
                // if (PS3.PS3Connected) {
                //   PS3.setLedOff();
                //   PS3.setLedOn(LED3);
                // }
                motorDrive_dir(DIR_LEFT);
                gLSpeed = DEFUALT_SPEED;
                gRSpeed = DEFUALT_SPEED;
            } else {
                gLSpeed = gRSpeed = 0;
            }
        } 
        motorDrive_control(gLSpeed, gRSpeed);

#ifdef _DEBUG_LOG
        if(PS3.getButtonClick(L3)){
            Serial.print(F("\r\nL3"));
        }
        if(PS3.getButtonClick(R3)){
            Serial.print(F("\r\nR3"));
        }
        if(isSelect) {
            Serial.print(F("\r\nSelect - "));
            PS3.printStatusString();
        }
#endif
        if (isStart) {
          printAngle = !printAngle;
        }
    }
 
#ifdef _DEBUG_LOG
    // Set this to 1 in order to see the angle of the controller
    if( printAngle ){
        Serial.print(F("\r\nPitch: "));
        Serial.print(PS3.getAngle(Pitch));
        Serial.print(F("\tRoll: "));
        Serial.print(PS3.getAngle(Roll));
    }
#endif
#endif

}
