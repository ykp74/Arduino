#include <Arduino.h>
#include "SdFat-beta.h"
#include "OPL2.h"

const char *SDFAT_ver = SD_FAT_VERSION;

// Use built-in SD for SPI modes on Teensy 3.5/3.6.
// Teensy 4.0 use first SPI port.
// SDCARD_SS_PIN is defined for the built-in SD on some boards.
#ifndef SDCARD_SS_PIN
const uint8_t SD_CS_PIN = SS;
#else  // SDCARD_SS_PIN
// Assume built-in SD is used.
const uint8_t SD_CS_PIN = SDCARD_SS_PIN;
#endif  // SDCARD_SS_PIN

// SD_FAT_TYPE = 0 for SdFat/File as defined in SdFatConfig.h,
// 1 for FAT16/FAT32, 2 for exFAT, 3 for FAT16/FAT32 and exFAT.
#define SD_FAT_TYPE 3

// Try max SPI clock for an SD. Reduce SPI_CLOCK if errors occur.
#define SPI_CLOCK SD_SCK_MHZ(50)

int YM_Datapins[8] = {23, 22, 21, 20, 19, 18, 17, 16};

const int YM_CS = 34;
const int YM_RD = 33;
const int YM_WR = 32;
const int YM_A0 = 31;
const int YM_IC = 30; 
const int YM_IRQ = 29;

OPL2 opl2(YM_Datapins, YM_CS, YM_RD, YM_WR, YM_A0, YM_IRQ, YM_IC);

const int prev_btn = 0;
const int rand_btn = 1;
const int next_btn = 2;
const int loop_btn = 3;
const int shuf_btn = 4;

SdFat32 SD;
File32 vgm;

//Buffer & file stream
const unsigned int MAX_CMD_BUFFER = 1;
uint8_t cmdBuffer[MAX_CMD_BUFFER];
uint32_t bufferPos = 0;
const unsigned int MAX_FILE_NAME_SIZE = 1024;
char fileName[MAX_FILE_NAME_SIZE];

struct playback_list{
    int index;
    byte type;
    char fileName[MAX_FILE_NAME_SIZE];
};
playback_list name_list[256];

unsigned char cmd = 0;
int numberOfFiles = 0;
int32_t currentFileNumber = 0;


//Timing Variables
float singleSampleWait = 0;
const int sampleRate = 44100; //44100 standard
const float WAIT60TH = ((1000.0 / (sampleRate/(float)735))*1000);
const float WAIT50TH = ((1000.0 / (sampleRate/(float)882))*1000);
uint32_t waitSamples = 0;
unsigned long preCalced8nDelays[16];
unsigned long preCalced7nDelays[16];
unsigned long lastWaitData61 = 0;
unsigned long cachedWaitTime61 = 5;
unsigned long pauseTime = 0;
unsigned long startTime = 0;

//Song Data Variables
uint32_t loopOffset = 0;
uint16_t loopCount = 0;
uint16_t nextSongAfterXLoops = 3;
enum PlayMode {LOOP, PAUSE, SHUFFLE, IN_ORDER};
PlayMode playMode = IN_ORDER;

//GD3 Data
String trackTitle;
String gameName;
String systemName;
String gameDate;

enum StartUpProfile {
  FIRST_START, 
  NEXT, 
  PREVIOUS, 
  RNG, 
  REQUEST
};

#ifdef FEATURE_SUPPORT_IMF
File32 imfFile;
float imfSpeed;
long songLength = 0;

void imf_loadSong(char* fileName) {
  imfFile = SD.open(fileName, FILE_READ);
  imfFile.seek(0);

  songLength  = imfFile.read();
  songLength += imfFile.read() << 8;
  if (songLength == 0) {
    songLength = 65535;
    imfFile.seek(4);
  }
}

int imf_playSong(void) {
  byte reg  = imfFile.read();
  byte data = imfFile.read();
  float  wait = imfFile.read();
  wait += imfFile.read() << 8;

  if (reg != 0x00) {
    opl2.write(reg, data);
  }
  songLength -= 3;
  return round(wait * (1000 / imfSpeed));
}
#endif /* FEATURE_SUPPORT_IMF */

void FillBuffer()
{
    vgm.read(cmdBuffer, MAX_CMD_BUFFER );
    //Serial.print("File location: "); Serial.println(vgm.position(), HEX);
}

unsigned char GetByte()
{
  if(bufferPos == MAX_CMD_BUFFER){
    bufferPos = 0;
    FillBuffer();
  }
  return cmdBuffer[bufferPos++];
}

uint32_t ReadBuffer32() //Read 32 bit value from buffer
{
  byte v0 = GetByte();
  byte v1 = GetByte();
  byte v2 = GetByte();
  byte v3 = GetByte();
  return uint32_t(v0 + (v1 << 8) + (v2 << 16) + (v3 << 24));
}

 //Read 32 bit value straight from SD card
uint32_t ReadSD32()
{
  byte v0 = vgm.read();
  byte v1 = vgm.read();
  byte v2 = vgm.read();
  byte v3 = vgm.read();
  return uint32_t(v0 + (v1 << 8) + (v2 << 16) + (v3 << 24));
}

void ClearBuffers()
{
  bufferPos = 0;
  for(int i = 0; i <= MAX_CMD_BUFFER; i++)
    cmdBuffer[i] = 0;
}

void ClearTrackData()
{
  trackTitle = "";
  gameName = "";
  systemName = "";
  gameDate = "";
}

uint32_t EoFOffset = 0;
uint32_t VGMVersion = 0;
uint32_t GD3Offset = 0;

void GetHeaderData() //Scrape off the important VGM data from the header, then drop down to the GD3 area for song info data
{
  ReadBuffer32();               //V - G - M 0x00->0x03
  EoFOffset = ReadBuffer32();   //End of File offset 0x04->0x07
  VGMVersion = ReadBuffer32();  //VGM Version 0x08->0x0B
  for(int i = 0x0C; i<0x14; i++)GetByte(); //Skip 0x0C->0x14
  GD3Offset = ReadBuffer32();   //GD3 (song info) data offset 0x14->0x17

  uint32_t bufferReturnPosition = vgm.position();
  vgm.seek(0);
  vgm.seekCur(GD3Offset+0x14);
  uint32_t GD3Position = 0x00;
  ReadSD32(); GD3Position+=4;   //G - D - 3
  ReadSD32(); GD3Position+=4;   //Version data
  uint32_t dataLength = ReadSD32(); //Get size of data payload
  GD3Position+=4;

  String rawGD3String;
  // Serial.print("DATA LENGTH: ");
  // Serial.println(dataLength);

  for(int i = 0; i<dataLength; i++) //Convert 16-bit characters to 8 bit chars. This may cause issues with non ASCII characters. (IE Japanese chars.)
  {
    char c1 = vgm.read();
    char c2 = vgm.read();
    if(c1 == 0 && c2 == 0)
      rawGD3String += '\n';
    else
      rawGD3String += char(c1);
  }
  GD3Position = 0;

  while(rawGD3String[GD3Position] != '\n') //Parse out the track title.
  {
    trackTitle += rawGD3String[GD3Position];
    GD3Position++;
  }
  GD3Position++;

  while(rawGD3String[GD3Position] != '\n') GD3Position++; //Skip Japanese track title.
  GD3Position++;
  while(rawGD3String[GD3Position] != '\n') //Parse out the game name.
  {
    gameName += rawGD3String[GD3Position];
    GD3Position++;
  }
  GD3Position++;
  while(rawGD3String[GD3Position] != '\n') GD3Position++;//Skip Japanese game name.
  GD3Position++;
  while(rawGD3String[GD3Position] != '\n') //Parse out the system name.
  {
    systemName += rawGD3String[GD3Position];
    GD3Position++;
  }
  GD3Position++;
  while(rawGD3String[GD3Position] != '\n') GD3Position++;//Skip Japanese system name.
  GD3Position++;
  while(rawGD3String[GD3Position] != '\n') GD3Position++;//Skip English authors
  GD3Position++;
  // while(rawGD3String[GD3Position] != 0) //Parse out the music authors (I skipped this since it sometimes produces a ton of data! Uncomment this, comment skip, add vars if you want this.)
  // {
  //   musicAuthors += rawGD3String[GD3Position];
  //   GD3Position++;
  // }
  while(rawGD3String[GD3Position] != '\n') GD3Position++;//Skip Japanese authors.
  GD3Position++;
  while(rawGD3String[GD3Position] != '\n') //Parse out the game date
  {
    gameDate += rawGD3String[GD3Position];
    GD3Position++;
  }
  GD3Position++;
  Serial.println(trackTitle);
  Serial.println(gameName);
  Serial.println(systemName);
  Serial.println(gameDate);
  Serial.println("");
  vgm.seek(bufferReturnPosition); //Send the file seek back to the original buffer position so we don't confuse our program.
  waitSamples = ReadBuffer32(); //0x18->0x1B : Get wait Samples count
  loopOffset = ReadBuffer32();  //0x1C->0x1F : Get loop offset Postition
   for(int i = 0; i<5; i++){
     ReadBuffer32(); //Skip right to the VGM data offset position;
  }
 
  uint32_t vgmDataOffset = ReadBuffer32();
  if(vgmDataOffset == 0 || vgmDataOffset == 12){ //VGM starts at standard 0x40
    ReadBuffer32(); ReadBuffer32();
  } else {
    Serial.println("VGM spec 1.7+");
    for(int i = 0; i < vgmDataOffset-4; i++) {
      GetByte();  //VGM starts at different data position (Probably VGM spec 1.7+)
    }
  }
  //Offset manually set to -4 due to overshooting the data offset. 
  //This does not seem normal and will need to be fixed.
  //Serial.println("Starting postion: "); Serial.println(vgm.position(), HEX);
}

void StartupSequence(StartUpProfile sup, String request = "")
{
  ClearTrackData();

  switch(sup){
    case FIRST_START:
      currentFileNumber = 0;
      Serial.println("FIRST_START event");
      break;

    case NEXT:
      if(currentFileNumber+1 >= numberOfFiles) {
          currentFileNumber = 0;
      } else {
          currentFileNumber++;
      }
      Serial.println("NEXT event");
      break;
    
    case PREVIOUS:
      if(currentFileNumber != 0){
          currentFileNumber--;
      } else {
          currentFileNumber = numberOfFiles-1;
      }
      Serial.println("PREVIOUS event");
      break;
    
    case RNG:
    {
      randomSeed(micros());
      uint16_t randomFile = currentFileNumber;
      while(randomFile == currentFileNumber)
        randomFile = random(numberOfFiles-1);
      currentFileNumber = randomFile;
      Serial.println("RNG event");
      break;
    }
    case REQUEST:
    {
      bool fileFound = false;
      Serial.print("REQUEST: ");Serial.println(request);
      for(int i = 0; i<numberOfFiles; i++) {
        String tmpFN = String(fileName);
        tmpFN.trim();
        if(tmpFN == request.trim()){
          currentFileNumber = i;
          fileFound = true;
          break;
        }
      }
      if(fileFound) {
        Serial.println("File found!");
      } else {
        Serial.println("ERROR: File not found! Continuing with current song.");
        return;
      }
      Serial.println("REQUEST event");
      break;
    }
  }

  opl2.begin();
  waitSamples = 0;
  loopOffset = 0;
  lastWaitData61 = 0;
  cachedWaitTime61 = 0;
  pauseTime = 0;
  startTime = 0;
  loopCount = 0;
  cmd = 0;
  ClearBuffers();
  
  Serial.print("Current file number: "); Serial.print(currentFileNumber+1); Serial.print("/"); Serial.println(numberOfFiles);
  
  if(vgm.isOpen()){
    vgm.close();
  }

  vgm = SD.open(name_list[currentFileNumber+1].fileName, FILE_READ);
  if(!vgm){
    Serial.println("File open failed!");
  } else {
    Serial.println("Opened successfully...");
  }
  FillBuffer();
  GetHeaderData();
  
  singleSampleWait = ((1000.0 / (sampleRate/(float)1))*1000);

  for(int i = 0; i<16; i++) {
    if(i == 0) {
      preCalced8nDelays[i] = 0;
      preCalced7nDelays[i] = 1;
    } else {
      preCalced8nDelays[i] = ((1000.0 / (sampleRate/(float)i))*1000);
      preCalced7nDelays[i] = ((1000.0 / (sampleRate/(float)i+1))*1000);
    }
  }
  delay(1000);
}

void unsupportedCode(byte b) {
  char buf[50];
  sprintf(buf, "Unsupported code 0x%02X offset 0x%08lX", b, vgm.position() - 1);
  Serial.println(buf);
}

void setup()
{
    File32 root;
    File32 entry;

    pinMode(prev_btn, INPUT_PULLUP);
    pinMode(rand_btn, INPUT_PULLUP);
    pinMode(next_btn, INPUT_PULLUP);
    pinMode(loop_btn, INPUT_PULLUP);
    pinMode(shuf_btn, INPUT_PULLUP);
    
    opl2.begin();
    if(!SD.begin(SdioConfig(FIFO_SDIO))){
        Serial.println("Card Mount Failed");
        return;
    }
#ifdef FEATURE_SUPPORT_IMF
    imfSpeed = 560.0;
#endif /* FEATURE_SUPPORT_IMF */

    root = SD.open("/");
    numberOfFiles = 0;
    while(1){
      entry = root.openNextFile();
      if (!entry) 
          break;
      entry.getName(name_list[numberOfFiles].fileName, MAX_FILE_NAME_SIZE);
      Serial.println(name_list[numberOfFiles].fileName);
      name_list[numberOfFiles].index = numberOfFiles;
      entry.close();
      numberOfFiles++;
    }
    numberOfFiles--;  //No count 'System Volume Information'
    root.close();
    Serial.println("setup done");
    StartupSequence(FIRST_START);
}

void loop()
{
  while(Serial.available() || Serial2.available())
  {
    bool USBorBluetooh = Serial.available();
    char serialCmd = USBorBluetooh ? Serial.read() : Serial2.read();
    switch(serialCmd)
    {
      case '+': //Next song
        StartupSequence(NEXT);
      break;
      case '-': //Previous Song
        StartupSequence(PREVIOUS);
      break;
      case '*': //Pick random song
        StartupSequence(RNG);
      break;
      case '/': //Toggle shuffle mode
        playMode == SHUFFLE ? playMode = IN_ORDER : playMode = SHUFFLE;
        playMode == SHUFFLE ? Serial.println("SHUFFLE ON") : Serial.println("SHUFFLE OFF");
        //DrawOledPage();
      break;
      case '.': //Toggle loop mode
        playMode == LOOP ? playMode = IN_ORDER : playMode = LOOP;
        playMode == LOOP ? Serial.println("LOOP ON") : Serial.println("LOOP OFF");
        //DrawOledPage();
      break;
      case 'r': //Song Request, format:  r:mySongFileName.vgm - An attempt will be made to find and open that file.
        String req = USBorBluetooh ? Serial.readString(1024) : Serial2.readString(1024);
        req.remove(0, 1); //Remove colon character
        StartupSequence(REQUEST, req);
      break;
    }
  }
  if(!digitalRead(next_btn))
    StartupSequence(NEXT);
  if(!digitalRead(prev_btn))
    StartupSequence(PREVIOUS);
  if(!digitalRead(rand_btn))
    StartupSequence(RNG);
  if(!digitalRead(shuf_btn))
  {
    playMode == SHUFFLE ? playMode = IN_ORDER : playMode = SHUFFLE;
    playMode == SHUFFLE ? Serial.println("SHUFFLE ON") : Serial.println("SHUFFLE OFF");
  }
  if(!digitalRead(loop_btn)){
    playMode == LOOP ? playMode = IN_ORDER : playMode = LOOP;
    playMode == LOOP ? Serial.println("LOOP ON") : Serial.println("LOOP OFF");   
  }

  if(loopCount >= nextSongAfterXLoops){
    if(playMode == SHUFFLE)
      StartupSequence(RNG);
    if(playMode == IN_ORDER)
      StartupSequence(NEXT);
  }

  unsigned long timeInMicros = micros();
  if( timeInMicros - startTime <= pauseTime)
  {
    // Serial.print("timeInMicros"); Serial.print("\t"); Serial.println(timeInMicros);
    // Serial.print("DELTA"); Serial.print("\t"); Serial.println(timeInMicros - startTime);
    // Serial.print("startTime"); Serial.print("\t"); Serial.println(startTime);
    //Serial.print("pauseTime"); Serial.print("\t"); Serial.println(pauseTime);
    //delay(150);
    return;
  }
  
  cmd = GetByte();
  
  switch(cmd)
  {
    case 0x5A:
    {
      uint8_t addr = GetByte();
      uint8_t data = GetByte();
      opl2.write(addr, data);
      break;
    }

    case 0x61:
    {
      //Serial.print("0x61 WAIT: at location: ");
      //Serial.(parseLocation);
      //Serial.print("  -- WAIT TIME: ");
      uint16_t wait = 0;
      for ( int i = 0; i < 2; i++ ){
        wait += ( uint32_t( GetByte() ) << ( 8 * i ));
      }
      startTime = timeInMicros;
      pauseTime = ((1000.0 / (sampleRate/(float)wait))*1000);
      //delayMicroseconds(cachedWaitTime61);
      //delay(cachedWaitTime61);
      break;
    }

    case 0x62:
      startTime = timeInMicros;
      pauseTime = WAIT60TH;
      //delay(16.67);
      //delayMicroseconds(pauseTime);//Actual time is 16.67ms (1/60 of a second)
      break;

    case 0x63:
      startTime = timeInMicros;
      pauseTime = WAIT50TH;
      //delay(20);
      //delayMicroseconds(pauseTime); //Actual time is 20ms (1/50 of a second)
      break;
    
    case 0x70:
      break;
    case 0x71:
    case 0x72:
    case 0x73:
    case 0x74:
    case 0x75:
    case 0x76:
    case 0x77:
    case 0x78:
    case 0x79:
    case 0x7A:
    case 0x7B:
    case 0x7C:
    case 0x7D:
    case 0x7E:
      //unsupportedCode(cmd);
      break;

    case 0x7F:
    {
      //Serial.println("0x7n WAIT");
      uint32_t wait = cmd & 0x0F;
      //Serial.print("Wait value: ");
      //Serial.println(wait);
      startTime = timeInMicros;
      pauseTime = preCalced7nDelays[wait];
      //delay(preCalced7nDelays[wait]);
      break;
    }

    case 0x66:
      if(loopOffset == 0){
        loopOffset = 64;
      }
      loopCount++;
      vgm.seek(loopOffset-0x1C);
      FillBuffer();
      bufferPos = 0;
      Serial.println("CODE_END_OF_DATA");
      break;
      
    default:
      break;

  }
}