// Keyboard library Mouse library
//#include <Mouse.h>
#include <Keyboard.h>
#include <TimerOne.h>

//definition
//#define DEBUG

#define VERSION         "KSND Multi Key Shield FW V1.3 (2023/03/24)"
#define PUSH(x)         digitalRead(x)==LOW 
#define RELEASE(x)      digitalRead(x)==HIGH 
#define debounceTime    50

#define KEY_0   8
#define KEY_1   9
#define KEY_2   10
#define KEY_3   11

// Indicator
#define LED1    6

long last_time_ms = 0; 
long current_time_ms;
boolean is_first_push = false;

//Key_0 Review comment
const char key0_sen0[] = "I Checked the completion of the basic test from the person in charge.";
const char key0_sen1[] = "It seems to be no problem when checking the change list.";
const char key0_sen2[] = "Approved.";

//Key_1 Review Feedback comment
const char key1_sen0[] = "flqbgkduTtmqslek."; //reviewed for hangle

//Key_2 primary Password
const char key2_sen0[] = "00153pyk*";  //password_1

//Key_3 secondary Password
const char key3_sen0[] = "yykp0153*";  //password_2

enum key_state {
    KEY_NONE,
    KEY_PUSH_0,
    KEY_RELEASE_0,
    KEY_PUSH_1,
    KEY_RELEASE_1,
    KEY_PUSH_2,
    KEY_RELEASE_2,
    KEY_PUSH_3,
    KEY_RELEASE_3,
    KEY_MAX
};

struct _keyboard {
    key_state state = KEY_NONE;
    boolean isPushed = false;
} key;

void timerIsr(void) {
    // Toggle LED
    digitalWrite( 13, digitalRead( 13 ) ^ 1 );
}

int key_input_check(void)
{
    switch(key.state){
        case KEY_NONE:
            if(PUSH(KEY_0)){    //Key_0
                current_time_ms = millis();
                if( !is_first_push ){
                    is_first_push = true;
                    last_time_ms = current_time_ms;
                }
                if((current_time_ms - last_time_ms) > debounceTime){
                    key.state = KEY_PUSH_0;
#ifdef DEBUG 
                    Serial.println("KEY_PUSH_0!!");
#endif
                    Keyboard.println(key0_sen0);
                    Keyboard.println(key0_sen1);
                    Keyboard.print(key0_sen2);
                    digitalWrite( LED1, HIGH );
                    is_first_push = false;     
                }
            } 
            else if(PUSH(KEY_1)){   //Key_1
                current_time_ms = millis();
                if( !is_first_push ){
                    is_first_push = true;
                    last_time_ms = current_time_ms;
                }
                if((current_time_ms - last_time_ms) > debounceTime){
                    key.state = KEY_PUSH_1;
#ifdef DEBUG 
                    Serial.println("KEY_PUSH_1!!");
#endif
                    Keyboard.print(key1_sen0);
                    digitalWrite( LED1, HIGH );
                    is_first_push = false;   
                }
            }
            else if(PUSH(KEY_2)){   //Key_2
                current_time_ms = millis();
                if( !is_first_push ){
                    is_first_push = true;
                    last_time_ms = current_time_ms;
                }
                if((current_time_ms - last_time_ms) > debounceTime){
                    key.state = KEY_PUSH_2;
#ifdef DEBUG 
                    Serial.println("KEY_PUSH_2!!");
#endif
                    Keyboard.print(key2_sen0);
                    //Keyboard.println(key2_sen0); //Enter Key Event
                    digitalWrite( LED1, HIGH );
                    is_first_push = false;   
                }
            }
            else if(PUSH(KEY_3)){   //Key_3
                current_time_ms = millis();
                if( !is_first_push ){
                    is_first_push = true;
                    last_time_ms = current_time_ms;
                }
                if((current_time_ms - last_time_ms) > debounceTime){
                    key.state = KEY_PUSH_3;
#ifdef DEBUG 
                    Serial.println("KEY_PUSH_3!!");
#endif
                    Keyboard.print(key3_sen0);
                    //Keyboard.println(key3_sen0); //Enter Key Event                 
                    digitalWrite( LED1, HIGH );
                    is_first_push = false;
                }
            } else {
                is_first_push = false; 
            }
            break;

        case KEY_PUSH_0:
            if(RELEASE(KEY_0)){
                key.state = KEY_RELEASE_0;
#ifdef DEBUG 
                Serial.println("KEY_RELEASE_0!!");
#endif
            }
            break;

        case KEY_PUSH_1:
            if(RELEASE(KEY_1)){
                key.state = KEY_RELEASE_1;
#ifdef DEBUG
                Serial.println("KEY_RELEASE_1!!");
#endif
            }
            break;

        case KEY_PUSH_2:
            if(RELEASE(KEY_2)){
                key.state = KEY_RELEASE_2;
#ifdef DEBUG 
                Serial.println("KEY_RELEASE_2!!");
#endif
            }
            break;

        case KEY_PUSH_3:
            if(RELEASE(KEY_3)){
                key.state = KEY_RELEASE_3;
#ifdef DEBUG            
                Serial.println("KEY_RELEASE_3!!");
#endif
            }
            //Display version 
            if(PUSH(KEY_2) && !key.isPushed){
                Keyboard.println(VERSION);
                key.isPushed = true;
            }
            break;

        case KEY_RELEASE_0:
        case KEY_RELEASE_1:
        case KEY_RELEASE_2:
        case KEY_RELEASE_3:
            digitalWrite( LED1, LOW );
            is_first_push = false;
            key.isPushed = false;
            key.state = KEY_NONE;
            break;
      
        default:
#ifdef DEBUG            
            Serial.println("Invalid Keyboard Status!!!");
#endif
            //Recover the status
            is_first_push = false;
            key.isPushed = false;
            key.state = KEY_NONE;
            break;
    }
}

// Initialization
void setup()
{
    // initialize input and outout pins
    pinMode(KEY_0, INPUT_PULLUP);
    pinMode(KEY_1, INPUT_PULLUP);
    pinMode(KEY_2, INPUT_PULLUP);
    pinMode(KEY_3, INPUT_PULLUP);
    pinMode(13, OUTPUT);
    pinMode(LED1, OUTPUT);

    Serial.begin(115200); // initialize mouse and keyboard control
    //Mouse.begin();
    Keyboard.begin();
    
    // set a timer of length 100000 microseconds (or 0.1 sec - or 10Hz => the led will blink 5 times, 5 cycles of on-and-off, per second)
    Timer1.initialize(500000); 
    Timer1.attachInterrupt( timerIsr ); // attach the service routine here
#ifdef DEBUG
    Serial.println("Boot Done!!");
#endif /* DEBUG */
}

void loop()
{
    key_input_check();
}
