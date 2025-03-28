#include <Arduino.h>
#include "stdio.h"

#define PRODUCT       "=-=-=-= Tester_670 =-=-=-="
#define MAJOR_VERSION "0"
#define MINOR_VERSION "2"
#define VERSION       ("v" MAJOR_VERSION "." MINOR_VERSION)

//#define DEBUG_DETAIL
#define _NANO
// 91% is safe : 120544
// 92% has error : 121192

#define SERIAL_SPEED    115200

// Global --------------
int pin_WData[4] = { A1, A3, A2, A0 };  //D0 D1 D2 D3
int pin_RData[4] = { 6, 7, 8, 9 };      //Q0 Q1 Q2 Q3

int pin_WAddr[2] = { 2, 3 };
int pin_RAddr[2] = { 10, 12 };

const int pin_WE = 4;
const int pin_RE = 5;
const int pin_POWER = A6;

#ifdef _NANO
void DBG(String msg)          { Serial.print(msg); }
void DBGHEX(long int data)    { Serial.print( String(data, HEX) + " "); }
void DBG(int num)             { Serial.print( String(num) + " "); }
void DBGLN(String msg)        { Serial.println(msg); }
void DBGHEXLN(long int data)  { Serial.println( String(data, HEX) + " "); }
void DBGLN(int num)           { Serial.println( String(num) + " "); }
#else
void DBG(String msg)          { SerialUSB.print(msg); }
void DBGHEX(long int data)    { SerialUSB.print( String(data, HEX) + " "); }
void DBG(int num)             { SerialUSB.print( String(num) + " "); }
void DBGLN(String msg)        { SerialUSB.println(msg); }
void DBGHEXLN(long int data)  { SerialUSB.println( String(data, HEX) + " "); }
void DBGLN(int num)           { SerialUSB.println( String(num) + " "); }
#endif

void DUMP_BUF(uint8_t len, uint8_t *buf)
{
#ifdef _NANO
    for (int i = 0; i<len; i++) {
        Serial.print(buf[i], HEX);
        if( i % 32 == 31 )
            Serial.println("");
        else
            Serial.print(",");
    }
    Serial.println("");
#else
    for (int i = 0; i<len; i++) {
        SerialUSB.print(buf[i], HEX);
        if( i % 32 == 31 )
            SerialUSB.println("");
        else
            SerialUSB.print(",");
    }
    SerialUSB.println("");
#endif
}

void DBGPAUSE()
{
#ifdef _NANO
    while( Serial.available() == 0 );
    Serial.read();
#else
    while( SerialUSB.available() == 0 );
    SerialUSB.read();
#endif
}

/** 
 *  Writes a value pinCount bits long to the pins specified in the 3rd argument.
 */
void Write( byte value, int pinCount, int *pins )
{
#ifdef DEBUG_DETAIL  
    char buf[255];
    sprintf( buf, "W V[]=%x : ", value );
    DBG( buf );
#endif
    
    byte mask = 0x1;
    for( int i=0; i<pinCount; i++ )
    {
        int v = (value & mask) > 0 ? HIGH : LOW;
        mask <<= 1;

        digitalWrite( pins[i], v);
#ifdef DEBUG_DETAIL  
        DBG( v );
        DBG( "," );
#endif
    }
#ifdef DEBUG_DETAIL  
    DBGLN("");
#endif
}

/**
 * Reads value pinCount long from the pins specified in the 2nd argument.
 */
byte Read( int pinCount, int *pins )
{
#ifdef DEBUG_DETAIL  
    DBG( "R V[]=" );
#endif
    byte mask = 0x1, ret = 0;
    for( int i=0; i<pinCount; i++ )
    {
        int v = (digitalRead( pins[i] ) == LOW) ? 0 : 1;
        ret += (v * mask);
        mask <<= 1;
#ifdef DEBUG_DETAIL  
        DBG( v );
        DBG( "," );
#endif
    }
#ifdef DEBUG_DETAIL  
    DBG( ":" );
    DBGLN( ret );
#endif
    return ret;
}

/**
 * Write an data to a given address
 */
void Write670( byte addr, byte data )
{
    Write( addr, 2, pin_WAddr );
    Write( data, 4, pin_WData );

    // cycle the pin to initiate the write
    digitalWrite( pin_WE, LOW );
    delay(1);
    digitalWrite( pin_WE, HIGH );
}

/**
 * Read data from a given address
 */
byte Read670( byte addr )
{
    Write( addr, 2, pin_RAddr );
    
    // cycle the pin to initiate the read
    digitalWrite( pin_RE, LOW );
    byte ret = Read( 4, pin_RData );
    digitalWrite( pin_RE, HIGH );
    
    return ret;
}

void setup() {
    // put your setup code here, to run once:
#ifdef _NANO
    Serial.begin(SERIAL_SPEED); 
    Serial.println( String(PRODUCT) + String(" ") + String(VERSION) );
#else
    SerialUSB.begin(SERIAL_SPEED); 
    while (!SerialUSB) {
        ; // wait for serial port to connect. Needed for native USB port only
    }
    SerialUSB.println( String(PRODUCT) + String(" ") + String(VERSION) );
#endif

    pinMode( pin_WE, OUTPUT );
    pinMode( pin_WAddr[0], OUTPUT );
    pinMode( pin_WAddr[1], OUTPUT );

    pinMode( pin_RE, OUTPUT );
    pinMode( pin_RAddr[0], OUTPUT );
    pinMode( pin_RAddr[1], OUTPUT );

    pinMode( pin_POWER, OUTPUT );

    for( int i=0; i<4; i++ ){
        pinMode( pin_WData[i], OUTPUT );
        pinMode( pin_RData[i], INPUT_PULLUP );
    }

    digitalWrite( pin_WE, HIGH );
    digitalWrite( pin_RE, HIGH );
    digitalWrite( pin_POWER, LOW );
}

void loop() {
    // put your main code here, to run repeatedly:
    DBGLN("PHASE 0");
    
    for(int a=0; a<4; a++){
        byte data;
        data = Read670(a);
        DBG( "add : " );
        DBG( a );
        DBG( " data : " );
        DBGHEXLN( data );
    }
    DBGPAUSE();

#if 0
    DBGLN("PHASE 1");
    // Store and check 2 different values in 4 addresses one address at a time. Total 8 checks for phase 1
    byte v[2] = { 0x5, 0xa };
    for( int i=0; i<2; i++ ){
        for( int a=0; a<4; a++ ){
            Write670( a, v[i] );            
            if( Read670(a) != v[i] ) {
                DBG( "Error at ");
                DBGHEXLN( a );
                DBGPAUSE();
            } else {
                DBG(".");
            }
        }
    }

    DBGLN("\nPHASE 2");
    // Store and check 16 different values in 4 addresses one address at a time. Total 64 checks for phase 2
    for( int a=0; a<4; a++ ){
        for( int d=0; d<16; d++ ){
            Write670( a, d );
            if( Read670(a) != d ){
                DBG( "Error at ");
                DBGHEX( a );
                DBG( ", Value = ");
                DBGHEXLN( d );
                DBGPAUSE();
            } else {
                DBG(".");
            }
        }
    }
#endif
    DBGLN("\nCompleted");
    DBGPAUSE();
}
