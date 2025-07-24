#include <SPI.h>
#include <SdFat.h>
#include <SFEMP3Shield.h>
#include <Bounce2.h> 
#include <TimerOne.h>

#ifdef FEATURE_LCD
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display
#endif

#include "TM1651.h"
#define CLK 4 //pins definitions for TM1651 and can be changed to other ports       
#define DIO 3 

TM1651 Display(CLK,DIO);

#define B_NEXT  A0 /* breif Macro for the debounced NEXT pin, with pull-up */
#define B_STOP  A1 /* breif Macro for the debounced STOP pin, with pull-up */
#define B_PLAY  A2 /* breif Macro for the debounced PLAY pin, with pull-up */

/**
 * \breif Macro for the Debounce Period [milliseconds]
 */
#define BUTTON_DEBOUNCE_PERIOD 20 //ms

/**
 * \brief Object instancing the SdFat library.
 */
SdFat sd;

/**
 * \brief Object instancing the SFEMP3Shield library.
 */
SFEMP3Shield MP3player;

Bounce b_Next  = Bounce();
Bounce b_Stop  = Bounce();
Bounce b_Play  = Bounce();

/**
 * \brief Index of the current track playing.
 */
uint8_t current_track = 1;
uint8_t cnt = 0;

//////////////////////////////////////////////////////////////////////////////////
#define MAX_NODE 30
int8_t node_cnt = 0;

struct NODE {
    char file_name[13];
    NODE* prev;
    NODE* next;
};

NODE node[MAX_NODE];
NODE* head;
NODE* currNode;

struct _status {
    uint8_t num_files = 0;
    bool isPlayback = false;
};

_status status;

void initNode(void)
{
    current_track = 1;
    node_cnt = 0;
 
    head = getNode();
    Serial.println(F("initNode"));
}

NODE* getNode(void)
{
    return &node[node_cnt++];
}

void addNode2Tail(char* fileName)
{
  for(currNode=head; currNode != NULL; currNode=currNode->next){
      if(currNode->next == NULL ){
        NODE* newNode = getNode();
        strncpy(newNode->file_name, fileName, 13);

        newNode->next = currNode->next;
        currNode->next = newNode;
        newNode->prev = currNode;
        break;
      }
  }
}

int8_t getPlayNextNodeCnt(void)
{
    current_track++;
    if(current_track == node_cnt){
        current_track = 1;
    }
    return current_track;
}

#ifdef FEATURE_DEBUG_PRINT
void printNode(void){
    for(currNode = head; currNode != NULL; currNode=currNode->next){
        Serial.print(F(" : "));
        Serial.print(currNode->file_name);
        if( currNode->next == NULL ){
            Serial.println(F(" "));
        }
    }
}
#endif /* FEATURE_DEBUG_PRINT */

#ifdef FEATURE_LCD
void printLCDScreen(int line, char* string)
{
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Play File Name:");
    lcd.setCursor(1,line);
    lcd.print(string);
}
#endif /* FEATURE_LCD */
////////////////////////////////////////////////////////////////////////////////

void getFileName(void) 
{
    FatFile file;
    FatFile dir;
    char filename[13];

    if (!dir.open("/")) {
        Serial.println(F("디렉터리 열기 실패"));
        return;
    }

    Serial.println(F("Music Files found :"));
    sd.chdir("/");  // 루트 디렉터리로 이동

    while (file.openNext(&dir,O_READ)){
        file.getName(filename, sizeof(filename));
        if ( isFnMusic(filename) ) {
          addNode2Tail(filename);
#ifdef FEATURE_DEBUG_PRINT
          printNode();
#endif /* FEATURE_DEBUG_PRINT */
        }
        file.close();
    }
    delay(100);

    Serial.println( node_cnt-1 );  //Total files count
}

void timerIsr(void)
{
    // Toggle LED
    //digitalWrite( 13, digitalRead( 13 ) ^ 1 );
    if(!status.isPlayback){
        return;
    }

    cnt++;
    if(cnt > 0xf){
        cnt = 0;
    }
    Display.displayNum(0,cnt);
}

void displayNumbers(uint8_t count)
{
    uint8_t dec = count / 10;
    uint8_t cnt = count % 10;

    Display.displayNum(1,dec);
    Display.displayNum(2,cnt);
}

/**
 * \brief Setup the Arduino Chip's feature for our use.
 *
 * After Arduino's kernel has booted initialize basic features for this
 * application, such as Serial port and MP3player objects with .begin.
 */
void setup()
{
    uint8_t result = 0;

    Serial.begin(115200);

    pinMode(B_NEXT, INPUT);   //INPUT_PULLUP
    pinMode(B_STOP, INPUT);   //INPUT_PULLUP
    pinMode(B_PLAY, INPUT);   //INPUT_PULLUP

    pinMode(5, OUTPUT);
 
    b_Next.attach(B_NEXT);   b_Next.interval(BUTTON_DEBOUNCE_PERIOD);
    b_Stop.attach(B_STOP);   b_Stop.interval(BUTTON_DEBOUNCE_PERIOD);
    b_Play.attach(B_PLAY);   b_Play.interval(BUTTON_DEBOUNCE_PERIOD);

    if(!sd.begin(SD_SEL, SPI_FULL_SPEED)) 
        sd.initErrorHalt();

    if(!sd.chdir("/"))
        sd.errorHalt("sd.chdir");

    result = MP3player.begin();
    if(result != 0) {
        Serial.print(F("Error code: "));
        Serial.print(result);
        Serial.println(F(" when trying to start MP3 player"));
        if( result == 6 ) {
            Serial.println(F("Warning: patch file not found, skipping.")); // can be removed for space, if needed.
            Serial.println(F("Use the \"d\" command to verify SdCard can be read")); // can be removed for space, if needed.
        }
    }
  
    MP3player.setVolume(2,2);            //KSND Must Be Set Max Volume or Gain without Saturation  
    MP3player.setDifferentialOutput(1);  //KSND Must be set Differentioal Case
    //MP3player.setMonoMode(1);  //KSND
 
#ifdef FEATURE_LCD
    lcd.init();
    lcd.backlight();
    lcd.setCursor(0,0);
    lcd.print("- MP3 Play Test!");
#endif

    Display.displayClear();
    Display.displaySet(2);    //BRIGHT_TYPICAL = 2,BRIGHT_DARKEST = 0,BRIGHTEST = 7;

    initNode();
    getFileName();

    Display.displayNum(0,0xc);
    displayNumbers(node_cnt-1);  //current_track
    //Display.displayNum(2,current_track);

    // set a timer of length 100000 microseconds (or 0.1 sec - or 10Hz => the led will blink 5 times, 5 cycles of on-and-off, per second)
    Timer1.initialize(100000); 
    Timer1.attachInterrupt(timerIsr); // ISR 연결
} 

/**
 * \brief Main Loop the Arduino Chip
 *
 * This is called at the end of Arduino kernel's main loop before recycling.
 * And is where the user's is executed.
 *
 * \note If the means of refilling is not interrupt based then the
 * MP3player object is serviced with the availaible function.
 */
void loop() 
{
    uint8_t result = 0;

// Below is only needed if not interrupt driven. Safe to remove if not using.
#if defined(USE_MP3_REFILL_MEANS) \
    && ( (USE_MP3_REFILL_MEANS == USE_MP3_SimpleTimer) \
    ||   (USE_MP3_REFILL_MEANS == USE_MP3_Polled)      )

    MP3player.available();
#endif

    if( b_Play.update()) {
        if( b_Play.read() == HIGH ) {
            Serial.print(F("B_PLAY pressed # "));
            if( MP3player.getState() != playback && 
                MP3player.getState() != paused_playback && 
                MP3player.getState() != uninitialized) {
                Serial.print(current_track); Serial.print(F(" ")); Serial.println(node[current_track].file_name);
#ifdef FEATURE_LCD
                printLCDScreen(1,node[current_track].file_name);
#endif
                Display.displayNum(0,0xe);
                displayNumbers(current_track);
                //Display.displayNum(2,current_track);
               
                result = MP3player.playMP3(node[current_track].file_name, 0);
                //result = MP3player.playTrack(current_track);
                
                if( result != 0 ){
                    Serial.print(F("Playback Error !!! : "));  Serial.println(result); 
                    status.isPlayback = false;
                } else {
                    status.isPlayback = true;
                }
            } else {
                /* Pause or Resume status */
                if( MP3player.getState()!= paused_playback && MP3player.getState() != uninitialized){
                    Serial.println(F("PAUSE"));
                    MP3player.pauseMusic(); 
                    status.isPlayback = false;
                } else {
                    Serial.println(F("RESUME"));
                    MP3player.resumeMusic(); 
                    status.isPlayback = true;
                }
            }
        }
    }

    if( b_Next.update()) {
        if( b_Next.read() == HIGH ) {
            getPlayNextNodeCnt();
            Serial.print(F("B_NEXT pressed # ")); Serial.print(current_track);  Serial.print(F(" "));
            Serial.println(node[current_track].file_name);

#ifdef FEATURE_LCD
            printLCDScreen(1,node[current_track].file_name);
#endif
            displayNumbers(current_track);
            //Display.displayNum(2,current_track);

            if( MP3player.getState() != playback && 
                MP3player.getState() != paused_playback && 
                MP3player.getState() != uninitialized){
              /* No Playback status */
              Display.displayNum(0,0x5);
            } else {
                Display.displayNum(0,0xe);  
                MP3player.stopTrack();
                delay(100);
                result = MP3player.playMP3(node[current_track].file_name, 0);
                if( result != 0 ){
                    Serial.print(F("Playback Error !!! : ")); Serial.println(result);
                    status.isPlayback = false;
                } else {
                    status.isPlayback = true;
                }
            }
        }
    }

    if (b_Stop.update()) {
        if (b_Stop.read() == HIGH)	{
            Serial.print(F("B_STOP pressed ALL Stop! # "));  Serial.print(current_track); Serial.print(F(" "));
            Serial.println(node[current_track].file_name);
            /* No Playback status */
            Display.displayNum(0,0x5);
            MP3player.stopTrack();
            status.isPlayback = false;
        }
    }
}