// Keyboard library Mouse library
//#include <Mouse.h>
#include <Keyboard.h>
#include <TimerOne.h>
#include <Bounce2.h>

//definition
//#define _DEBUG
#define _KEY3
#define FEATURE_BOUNCE2

#define VERSION         "KSND Multi Key Shield FW V1.7 (2026/10/30)"
#define PUSH(x)         digitalRead(x)==LOW 
#define RELEASE(x)      digitalRead(x)==HIGH 
#define DEBOUNCE_TIME_MS    50UL
#define STANDBY_BLINK_MS    500UL

#ifdef _DEBUG
#define DEBUG_PRINT(...) Serial.println(__VA_ARGS__)
#else
#define DEBUG_PRINT(...)
#endif

#define KEY_0   8
#define KEY_1   9
#define KEY_2   10
#define KEY_3   11
#define KEY_4   11  //Share

// Indicator
#define LED1    6
#define LED_STANDBY  13

unsigned long last_time_ms = 0; 
unsigned long current_time_ms;
unsigned long standby_last_toggle_ms = 0;
bool is_first_push = false;

//Key_0 Review comment
const char key0_sen0[] = "I Checked the completion of the basic test from the person in charge.";
const char key0_sen1[] = "It seems to be no problem when checking the change list.";
const char key0_sen2[] = "Approved.";

//Key_1 Review Feedback comment
const char key1_sen0[] = "flqbgkduTtmqslek."; //reviewed for hangle

//Key_2 primary Password
const char key2_sen0[] = "00153Pykk*";  //password_1 대문자 추가
//const char key2_sen0[] = "00153pyk*";  //password_1

#ifdef _KEY3
//Key_3 secondary Password
const char key3_sen0[] = "Yykp01533*";  //password_2 대문자 추가
//const char key3_sen0[] = "yykp0153*";  //password_2
#else
//Key_4 Third Password
const char key4_sen0[] = "Yykp01533.";  //password_3
#endif /* _KEY3 */

#ifdef FEATURE_BOUNCE2
const uint8_t BUTTON_COUNT = 4;
int8_t activeButton = -1;   // -1: 처리 중인 버튼 없음
bool versionPrinted = false;

const uint8_t buttonPins[BUTTON_COUNT] = {
    KEY_0,
    KEY_1,
    KEY_2,
#ifdef _KEY3
    KEY_3
#else
    KEY_4
#endif
};

Bounce2::Button buttons[BUTTON_COUNT];
#endif

enum key_state {
    KEY_NONE,
    KEY_PUSH_0,
    KEY_RELEASE_0,
    KEY_PUSH_1,
    KEY_RELEASE_1,
    KEY_PUSH_2,
    KEY_RELEASE_2,
#ifdef _KEY3
    KEY_PUSH_3,
    KEY_RELEASE_3,
#else
    KEY_PUSH_4,
    KEY_RELEASE_4,
#endif /* _KEY3 */
    KEY_MAX
};

struct _keyboard {
    key_state state = KEY_NONE;
    boolean isPushed = false;
} key;

//void updateStandbyLed(void)
//{
//    unsigned long now = millis();
//
//    if ((now - standby_last_toggle_ms) >= STANDBY_BLINK_MS) {
//        standby_last_toggle_ms = now;
//        digitalWrite(LED_STANDBY, !digitalRead(LED_STANDBY));
//    }
//}

void timerIsr(void)
{
    // Toggle Monitoring LED
    digitalWrite(LED_STANDBY, digitalRead(LED_STANDBY)^1 );
}

#ifdef FEATURE_BOUNCE2
void key_input_check2()
{
    // 출력이나 상태 판단 전에 모든 버튼을 먼저 갱신
    for (uint8_t i = 0; i < BUTTON_COUNT; ++i) {
        buttons[i].update();
    }

    // 이미 처리한 버튼은 해제될 때까지 기다림
    if (activeButton >= 0) {
        if (!buttons[activeButton].isPressed()) {
            DEBUG_PRINT(F("KEY_RELEASE"));

            digitalWrite(LED1, LOW);
            activeButton = -1;
            versionPrinted = false;
            return;
        }

        // KEY_3 또는 KEY_4 유지 중 KEY_2를 누르면
        // 해당 유지 구간에서 버전을 한 번만 출력
        if (activeButton == 3 && buttons[2].isPressed() &&  !versionPrinted) {
            versionPrinted = true;
            Keyboard.println(VERSION);
            DEBUG_PRINT(F("VERSION_PRINT"));
        }
        return;
    }

    // 기존과 같은 우선순위: KEY_0 → KEY_1 → KEY_2 → KEY_3/4
    for (uint8_t i = 0; i < BUTTON_COUNT; ++i) {
        if (!buttons[i].isPressed()) {
            continue;   //키 안눌려 있으면 넘어감
        }

        activeButton = static_cast<int8_t>(i);
        versionPrinted = false;
        digitalWrite(LED1, HIGH);

        switch (i) {
            case 0:
                DEBUG_PRINT(F("KEY_PUSH_0!!"));
                Keyboard.println(key0_sen0);
                Keyboard.println(key0_sen1);
                Keyboard.print(key0_sen2);
                break;

            case 1:
                DEBUG_PRINT(F("KEY_PUSH_1!!"));
                Keyboard.print(key1_sen0);
                break;

            case 2:
                DEBUG_PRINT(F("KEY_PUSH_2!!"));
                Keyboard.print(key2_sen0);
                break;

            case 3:
#ifdef _KEY3
                DEBUG_PRINT(F("KEY_PUSH_3!!"));
                Keyboard.print(key3_sen0);
#else
                DEBUG_PRINT(F("KEY_PUSH_4!!"));
                Keyboard.print(key4_sen0);
#endif
                break;
        }
        // 한 번의 검사에서는 버튼 하나만 처리
        return;
    }
}
#else
void key_input_check(void)
{
    switch(key.state){
        case KEY_NONE:
            if(PUSH(KEY_0)){    //Key_0
                current_time_ms = millis();
                if( !is_first_push ){
                    is_first_push = true;
                    last_time_ms = current_time_ms;
                }
                if((current_time_ms - last_time_ms) > DEBOUNCE_TIME_MS){
                    key.state = KEY_PUSH_0;
                    DEBUG_PRINT("KEY_PUSH_0!!");
                    Keyboard.println(key0_sen0);
                    Keyboard.println(key0_sen1);
                    Keyboard.print(key0_sen2);
                    digitalWrite( LED1, HIGH );
                    is_first_push = false;     
                }
            } 
            else if(PUSH(KEY_1)){   //Key_1
                current_time_ms = millis();
                if(!is_first_push){
                    is_first_push = true;
                    last_time_ms = current_time_ms;
                }
                if((current_time_ms - last_time_ms) > DEBOUNCE_TIME_MS){
                    key.state = KEY_PUSH_1;
                    DEBUG_PRINT("KEY_PUSH_1!!");
                    Keyboard.print(key1_sen0);
                    digitalWrite( LED1, HIGH );
                    is_first_push = false;   
                }
            }
            else if(PUSH(KEY_2)){   //Key_2 Password 1
                current_time_ms = millis();
                if(!is_first_push){
                    is_first_push = true;
                    last_time_ms = current_time_ms;
                }
                if((current_time_ms - last_time_ms) > DEBOUNCE_TIME_MS){
                    key.state = KEY_PUSH_2;
                    DEBUG_PRINT("KEY_PUSH_2!!");
                    Keyboard.print(key2_sen0);
                    //Keyboard.println(key2_sen0); //Enter Key Event
                    digitalWrite( LED1, HIGH );
                    is_first_push = false;   
                }
            }
#ifdef _KEY3
            else if(PUSH(KEY_3)){   //Key_3 Password 2
                current_time_ms = millis();
                if(!is_first_push){
                    is_first_push = true;
                    last_time_ms = current_time_ms;
                }
                if((current_time_ms - last_time_ms) > DEBOUNCE_TIME_MS){
                    key.state = KEY_PUSH_3;
                    DEBUG_PRINT("KEY_PUSH_3!!");
                    Keyboard.print(key3_sen0);
                    //Keyboard.println(key3_sen0); //Enter Key Event                 
                    digitalWrite( LED1, HIGH );
                    is_first_push = false;
                }
            }
#else
            else if(PUSH(KEY_4)){   //Key_4 Password 3
                current_time_ms = millis();
                if(!is_first_push){
                    is_first_push = true;
                    last_time_ms = current_time_ms;
                }
                if((current_time_ms - last_time_ms) > DEBOUNCE_TIME_MS){
                    key.state = KEY_PUSH_4;
                    DEBUG_PRINT("KEY_PUSH_4!!");
                    Keyboard.print(key4_sen0);
                    //Keyboard.println(key3_sen0); //Enter Key Event                 
                    digitalWrite( LED1, HIGH );
                    is_first_push = false;
                }
            }
#endif /* _KEY3 */
            else {
                is_first_push = false; 
            }
            break;

        case KEY_PUSH_0:
            if(RELEASE(KEY_0)){
                key.state = KEY_RELEASE_0;
                DEBUG_PRINT("KEY_RELEASE_0!!");
            }
            break;

        case KEY_PUSH_1:
            if(RELEASE(KEY_1)){
                key.state = KEY_RELEASE_1;
                DEBUG_PRINT("KEY_RELEASE_1!!");  
            }
            break;

        case KEY_PUSH_2:
            if(RELEASE(KEY_2)){
                key.state = KEY_RELEASE_2;
                DEBUG_PRINT("KEY_RELEASE_2!!");
            }
            break;
#ifdef _KEY3
        case KEY_PUSH_3:
            if(RELEASE(KEY_3)){
                key.state = KEY_RELEASE_3;
                DEBUG_PRINT("KEY_RELEASE_3!!");
            }
            //Display version 
            if(PUSH(KEY_2) && !key.isPushed){
                Keyboard.println(VERSION);
                key.isPushed = true;
            }
            break;
#else
        case KEY_PUSH_4:
            if(RELEASE(KEY_4)){
                key.state = KEY_RELEASE_4;
                DEBUG_PRINT("KEY_RELEASE_4!!");
            }
            //Display version 
            if(PUSH(KEY_2) && !key.isPushed){
                Keyboard.println(VERSION);
                key.isPushed = true;
            }
            break;
#endif /* _KEY3 */
        case KEY_RELEASE_0:
        case KEY_RELEASE_1:
        case KEY_RELEASE_2:
#ifdef _KEY3
        case KEY_RELEASE_3:
#else
        case KEY_RELEASE_4:
#endif /* _KEY3 */
            digitalWrite( LED1, LOW );
            is_first_push = false;
            key.isPushed = false;
            key.state = KEY_NONE;
            break;
      
        default:
            DEBUG_PRINT("Invalid Keyboard Status!!!");
            //Recover the status
            is_first_push = false;
            key.isPushed = false;
            key.state = KEY_NONE;
            break;
    }
}
#endif

void key_input_begin()
{
#ifdef FEATURE_BOUNCE2
    for (uint8_t i = 0; i < BUTTON_COUNT; ++i) {
        buttons[i].attach(buttonPins[i], INPUT_PULLUP);
        buttons[i].interval(DEBOUNCE_TIME_MS);
        buttons[i].setPressedState(LOW);
    }
#else
    pinMode(KEY_0, INPUT_PULLUP);
    pinMode(KEY_1, INPUT_PULLUP);
    pinMode(KEY_2, INPUT_PULLUP);
    pinMode(KEY_3, INPUT_PULLUP);
#endif
}

// Initialization
void setup()
{
    // initialize input and outout pins
    key_input_begin();
    pinMode(LED_STANDBY, OUTPUT);
    pinMode(LED1, OUTPUT);

    Serial.begin(115200); // initialize mouse and keyboard control
    //Mouse.begin();
    Keyboard.begin();
    
    digitalWrite(LED1, LOW);
    digitalWrite(LED_STANDBY, LOW);

    // set a timer of length 100000 microseconds
    // (or 0.1 sec - or 10Hz => the led will blink 5 times, 5 cycles of on-and-off, per second)
    Timer1.initialize(1000000); 
    Timer1.attachInterrupt( timerIsr ); // attach the service routine here
    DEBUG_PRINT("Boot Done!!");
}

void loop()
{
    //updateStandbyLed();
#ifdef FEATURE_BOUNCE2
    key_input_check2();
#else
    key_input_check();
#endif
}
