#define VERSIONSTRING "KSND ROM WRITTER FW V1.7 (2021/09/23 06:44)"
#define ARDUINO_UNO

//Manufacturer Code
#define ATMEL   0x1f
#define WINBOND 0xda

//FLASH ROM Type
#define AT29C256    0xdc
#define AT29C512    0x5d

// shiftOut part
#define SHIFT_DATA    A0
#define SHIFT_LATCH   A1
#define SHIFT_CLK     A2

#define PORTC_DS    0
#define PORTC_LATCH 1
#define PORTC_CLOCK 2

// eeprom stuff
// define the IO lines for the data - bus
#define D0 2
#define D1 3
#define D2 4
#define D3 5
#define D4 6
#define D5 7
#define D6 8
#define D7 9

// define the IO lines for the eeprom control
#define CE A3   //ChipSelect 
#define OE A4   //OutputEnable
#define WE A5   //WriteEnable

#define IND_LED 13

//a buffer for bytes to burn
//#define BUFFERSIZE 1024 
#define BUFFERSIZE 256 
byte buffer[BUFFERSIZE] = {0x00,};

//command buffer for parsing commands
#define COMMANDSIZE 32
char cmdbuf[COMMANDSIZE];

unsigned int startAddress,endAddress;
unsigned int lineLength,dataLength;
int sectorsize = 0;

//define COMMANDS
#define NOCOMMAND    0
#define VERSION      1

#define READ_HEX    10
#define READ_BIN    11
//#define READ_ITL    12

#define WRITE_HEX   20
#define WRITE_BIN   21
//#define WRITE_ITL   22

byte buf_k[64] = {0, };

// switch IO lines of databus to INPUT state
void data_bus_input(void) {
  pinMode(D0, INPUT);
  pinMode(D1, INPUT);
  pinMode(D2, INPUT);
  pinMode(D3, INPUT);
  pinMode(D4, INPUT);
  pinMode(D5, INPUT);
  pinMode(D6, INPUT);
  pinMode(D7, INPUT);
}

//switch IO lines of databus to OUTPUT state
void data_bus_output(void) {
  pinMode(D0, OUTPUT);
  pinMode(D1, OUTPUT);
  pinMode(D2, OUTPUT);
  pinMode(D3, OUTPUT);
  pinMode(D4, OUTPUT);
  pinMode(D5, OUTPUT);
  pinMode(D6, OUTPUT);
  pinMode(D7, OUTPUT);
}

//set databus to input and read a complete byte from the bus
//be sure to set data_bus to input before
byte read_data_bus(void) {
  return ((digitalRead(D7) << 7) + (digitalRead(D6) << 6) + (digitalRead(D5) << 5) + (digitalRead(D4) << 4) +
          (digitalRead(D3) << 3) + (digitalRead(D2) << 2) + (digitalRead(D1) << 1) + digitalRead(D0));
}

//write a byte to the data bus
//be sure to set data_bus to output before
void write_data_bus(byte data){
#if 1
  digitalWrite(D0, data & 1);
  digitalWrite(D1, data & 2);
  digitalWrite(D2, data & 4);
  digitalWrite(D3, data & 8);
  digitalWrite(D4, data & 16);
  digitalWrite(D5, data & 32);
  digitalWrite(D6, data & 64);
  digitalWrite(D7, data & 128);
#else
  //2 bits belong to PORTB and have to be set separtely 
  digitalWrite(D6, (data >> 6) & 0x01);
  digitalWrite(D7, (data >> 7) & 0x01);
  //bit 0 to 6 belong to bit 2 to 8 of PORTD
  PORTD = PIND | ( data << 2 );
#endif
}

//shift out the given address to the 74hc595 registers
void set_address_bus(unsigned int address){
#ifndef ARDUINO_UNO
  shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, (address >> 8) );
  shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, address);

  digitalWrite( SHIFT_LATCH, LOW);
  digitalWrite( SHIFT_LATCH, HIGH);
  digitalWrite( SHIFT_LATCH, LOW);
#else
  //get high - byte of 16 bit address
  byte hi = address >> 8;
  //get low - byte of 16 bit address
  byte low = address & 0xff;

  //disable latch line
  bitClear(PORTC,PORTC_LATCH);

  //shift out highbyte
  fastShiftOut(hi);
  //shift out lowbyte
  fastShiftOut(low);

  //enable latch and set address
  bitSet(PORTC,PORTC_LATCH);
#endif
}

#ifdef ARDUINO_UNO
//faster shiftOut function then normal IDE function (about 4 times)
void fastShiftOut(byte data) {
  //clear data pin
  bitClear(PORTC,PORTC_DS);
  //Send each bit of the myDataOut byte MSBFIRST
  for (int i=7; i>=0; i--)  {
    bitClear(PORTC,PORTC_CLOCK);
    //--- Turn data on or off based on value of bit
    if ( bitRead(data,i) == 1) {
      bitSet(PORTC,PORTC_DS);
    } else {      
      bitClear(PORTC,PORTC_DS);
    }
    //register shifts bits on upstroke of clock pin  
    bitSet(PORTC,PORTC_CLOCK);
    //zero the data pin after shift to prevent bleed through
    bitClear(PORTC,PORTC_DS);
  }
  //stop shifting
  bitClear(PORTC,PORTC_CLOCK);
}
#endif

byte read_byte( unsigned int address ){
  byte data = 0;

  data_bus_input();         //set databus for reading
  set_address_bus(address); //set address bus

  digitalWrite(WE, HIGH);   //disable write
  digitalWrite(CE, HIGH);   //enable chip select
  digitalWrite(OE, HIGH);   //first disbale output

  digitalWrite(CE, LOW);    //enable chip select
  digitalWrite(OE, LOW);    //enable output

  data = read_data_bus();   //read data bus in that address

  digitalWrite(CE, HIGH);   //enable chip select
  digitalWrite(OE, HIGH);   //disable output

  return data;
}

void write_byte(unsigned int address, byte data) {
  data_bus_output();        //set databus to output
  set_address_bus(address); //set address bus

  digitalWrite(OE, HIGH);
  digitalWrite(WE, HIGH);   //disable write
  digitalWrite(CE, HIGH);   //disable Chip
  write_data_bus(data);     //set data bus

  delayMicroseconds(1);     //wait some time to finish writing

  digitalWrite(CE, LOW);    //enable chip select
  digitalWrite(WE, LOW);    //enable write

  delayMicroseconds(1);     //wait some time to finish writing

  digitalWrite(WE, HIGH);   //disable write
  digitalWrite(CE, HIGH);   //disable Chip
}

void disable_protection_eeprom(void) {
  write_byte(0x5555,0xaa);
  write_byte(0x2aaa,0x55);
  write_byte(0x5555,0x80);
  write_byte(0x5555,0xaa);
  write_byte(0x2aaa,0x55);
  write_byte(0x5555,0x20);
}

void enable_protection_eeprom( void ) {
  write_byte(0x5555,0xaa);
  write_byte(0x2aaa,0x55);
  write_byte(0x5555,0xa0);
}

void erase_eeprom( void ) {
  disable_protection_eeprom();
  delay(10);
  
  write_byte(0x5555,0xaa);
  write_byte(0x2aaa,0x55);
  write_byte(0x5555,0x80);
  write_byte(0x5555,0xaa);
  write_byte(0x2aaa,0x55);
  write_byte(0x5555,0x10);
  
  delay(10);
  Serial.println("erase_eeprom");
}

void idread_eeprom( void ) {
  byte id;
  int end_addr = 0;
  
  write_byte(0x5555,0xaa);
  write_byte(0x2aaa,0x55);
  write_byte(0x5555,0x90);

  delay(10);

  id = read_byte(0x0001);
  Serial.print("EEPROM Type Code : 0x");
  Serial.print(id,HEX);
  Serial.print(" ");

  switch(id){
    case AT29C256:
      Serial.print("(AT29C256) Sector Size : ");
      sectorsize =  64;
      end_addr   = 0x7fff;                            
      break;

    case AT29C512:
      Serial.print("(AT29C512) Sector Size : ");
      sectorsize = 128; 
      end_addr   = 0xffff; 
      break;

    default:
      Serial.print("(Unknown)  Sector Size : ");
      sectorsize = 0; 
      end_addr = 0x0000;
      break;
  }
  Serial.println(sectorsize);

  write_byte(0x5555,0xaa);
  write_byte(0x2aaa,0x55);
  write_byte(0x5555,0xf0);
 
  delay(10);
} 

/************************************************************
 * convert a single hex digit (0-9,a-f) to byte
 * @param char c single character (digit)
 * @return byte represented by the digit 
 ************************************************************/
byte hexToDigit(char c){
  if (c >= '0' && c <= '9') {
    return c - '0';
  } else if (c >= 'a' && c <= 'f') {
    return c - 'a' + 10;
  } else if (c >= 'A' && c <= 'F') {
    return c - 'A' + 10;
  } else {
    return 0;   // getting here is bad: it means the character was invalid
  }
}

/************************************************************
 * convert a hex byte (00 - ff) to byte
 * @param c-string with the hex value of the byte
 * @return byte represented by the digits 
 ************************************************************/
byte hexToByte(char* a){
  return ((hexToDigit(a[0])*16) + hexToDigit(a[1]));
}

/************************************************************
 * convert a hex word (0000 - ffff) to unsigned int
 * @param c-string with the hex value of the word
 * @return unsigned int represented by the digits 
 ************************************************************/
unsigned int hexToWord(char* data) {
  return ((hexToDigit(data[0])*4096) + (hexToDigit(data[1])*256) +
    (hexToDigit(data[2])*16) + (hexToDigit(data[3])));
}

/**
 * read a data block from eeprom and write out a hex dump 
 * of the data to serial connection
 * @param from       start address to read fromm
 * @param to         last address to read from
 * @param linelength how many hex values are written in one line
 **/
void read_block(unsigned int from, unsigned int to, int linelength) {
  //count the number fo values that are already printed out on the
  //current line
  int    outcount = 0;
  //loop from "from address" to "to address" (included)
  for (unsigned int address = from; address <= to; address++) {
    if (outcount == 0) {
      //print out the address at the beginning of the line
      Serial.println();
      Serial.print("0x00");
      printAddress(address);
      Serial.print("  ");
    }
    //print data, separated by a space
    byte data = read_byte(address);
    printByte(data);
    Serial.print(" ");
    outcount = (++outcount % linelength);

  }
  //print a newline after the last data line
  Serial.println();
}

/**
 * read a data block from eeprom and write out the binary data 
 * to the serial connection
 * @param from       start address to read fromm
 * @param to         last address to read from
 **/
void read_binblock(unsigned int from, unsigned int to) {
  for (unsigned int address = from; address <= to; address++) {
    Serial.write(read_byte(address));
  }
  //print out an additional 0-byte as success return code
  Serial.print('\0');
}  
  
/**
 * write a data block to the eeprom
 * @param address  startaddress to write on eeprom
 * @param buffer   data buffer to get the data from
 * @param len      number of bytes to be written
 **/
void write_block(unsigned int address, byte* buffer, int len) {
  enable_protection_eeprom();
  for (unsigned int i = 0; i < len; i++) {
    write_byte(address+i,buffer[i]);
  }
}

/**
 * print out a 16 bit word as 4 character hex value
 **/
void printAddress(unsigned int address) {
  if(address < 0x0010) Serial.print("0");
  if(address < 0x0100) Serial.print("0");
  if(address < 0x1000) Serial.print("0");
  Serial.print(address, HEX);
}

/**
 * print out a byte as 2 character hex value
 **/
void printByte(byte data) {
  if(data < 0x10) Serial.print("0");
  Serial.print(data, HEX);  
}

//waits for a string submitted via serial connection
//returns only if linebreak is sent or the buffer is filled
void readCommand() {
  //first clear command buffer
  for(int i=0; i< COMMANDSIZE;i++) cmdbuf[i] = 0;
  //initialize variables
  char c = ' ';
  int idx = 0;
  //now read serial data until linebreak or buffer is full
  do {
    if(Serial.available()) {
      c = Serial.read();
      cmdbuf[idx++] = c;
    }
  } 
  while (c != '\n' && idx < (COMMANDSIZE)); //save the last '\0' for string end
  //change last newline to '\0' termination
  cmdbuf[idx - 1] = 0;
}

//parse the given command by separating command character and parameters
//at the moment only 5 commands are supported
byte parseCommand() {
  //set ',' to '\0' terminator (command string has a fixed strucure)
  //first string is the command character
  cmdbuf[1]  = 0;
  //second string is startaddress (4 bytes)
  cmdbuf[6]  = 0;
  //third string is endaddress (4 bytes)
  cmdbuf[11] = 0;
  //fourth string is length (2 bytes)
  cmdbuf[14] = 0;

  startAddress = hexToWord((cmdbuf+2));
  dataLength = hexToWord((cmdbuf+7));
  lineLength = hexToByte(cmdbuf+12);

  byte retval = 0;

  switch(cmdbuf[0]) {
    case 'R': retval = READ_HEX;  break;
    case 'r': retval = READ_BIN;  break;
    case 'W': retval = WRITE_HEX;  break;
    case 'w': retval = WRITE_BIN;  break;
    case 'v': retval = VERSION;    break;

    case 'i': idread_eeprom(); break;
    case 'e': erase_eeprom();  break;

    default:
      retval = NOCOMMAND;
      break;
  }
  return retval;
}


void setup() {
  //define the shiuftOut Pins as output
  pinMode(SHIFT_DATA, OUTPUT);
  pinMode(SHIFT_LATCH, OUTPUT);
  pinMode(SHIFT_CLK, OUTPUT);

  data_bus_output();

  //define the EEPROM Pins as output
  // take care that they are HIGH
  pinMode(OE, OUTPUT);
  pinMode(CE, OUTPUT);
  pinMode(WE, OUTPUT);
  digitalWrite(OE, HIGH);
  digitalWrite(CE, HIGH);
  digitalWrite(WE, HIGH);

  pinMode(IND_LED, OUTPUT);
  digitalWrite(IND_LED, LOW);

  //set speed of serial connection
  Serial.begin(115200);
#ifndef ARDUINO_UNO
  while (!Serial);      //for LEO only
#endif
  digitalWrite(IND_LED, HIGH);
  Serial.println("Booting Ok!");
}

void loop() {
  readCommand();
  byte cmd = parseCommand();
  int bytes = 0;
  unsigned int addr = 0x0000;

  switch(cmd) {
    case READ_HEX:  //Local Display in terminal Screen
      if(lineLength == 0){
        lineLength = 16;
      }
      endAddress = startAddress + dataLength -1;
      read_block(startAddress,endAddress,lineLength);
      Serial.println('%');
      break;

    case READ_BIN:  //ROM -> WRITTER
      endAddress = startAddress + dataLength -1;
      read_binblock(startAddress,endAddress);
      break;

    case WRITE_BIN:  //WRITTER->ROM
      //take care for max buffer size
      if(dataLength > 1024){
        dataLength = 1024;
      }
      endAddress = startAddress + dataLength -1;
      while(bytes < dataLength) {
        if(Serial.available())
          buffer[bytes++] = Serial.read();
      }
      write_block(startAddress, buffer, dataLength);
      Serial.println('%');
      break;
   
    case WRITE_HEX:     //29C256 Flash_ROM
      disable_protection_eeprom();
      //enable_protection_eeprom();
      while(bytes < 64) {
        if(Serial.available()) 
          buf_k[bytes++] = Serial.read();
      }

      for(int index =0; index < 64; index++){
        write_byte( startAddress, buf_k[index] );
        addr++;
      }
      delay(5);
      Serial.println('%');
      break;

    case VERSION:
      Serial.println(VERSIONSTRING);
      break;
  }
}
