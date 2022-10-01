#define VERSIONSTRING "KSND ROM WRITTER FW V2.1 (2021/09/23 07:13)"
#define ARDUINO_NANO

//Manufacturer Code
#define ATMEL   0x1f
#define WINBOND 0xda

//FLASH ROM Type
#define AT29C256    0xdc
#define AT29C512    0x5d
#define AT29C010A   0xd5
#define AT29C020A   0xda
#define AT29C040A   0xa4
#define AT29C1024   0x25

#define W29C020    0x45

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

#define HEX_DISP_5 10
#define HEX_DISP_6 11
#define HEX_DISP_9 12
#define HEX_DISP_10 13

//a buffer for bytes to burn
#define BUFFERSIZE 256 
byte buffer[BUFFERSIZE] = {0x00,};

//command buffer for parsing commands
#define COMMANDSIZE 32
char cmdbuf[COMMANDSIZE];

unsigned long startAddress,endAddress,dataLength;
unsigned int lineLength;
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

#define TEST_WRITE    30
#define TEST_READ     31
#define TEST_ADDRESS  32
#define TEST_READ_BIN 33

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
void set_address_bus(unsigned long address){
#ifndef ARDUINO_NANO
  shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, (address >> 16) );
  shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, (address >> 8) );
  shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, address);

  digitalWrite( SHIFT_LATCH, LOW);
  digitalWrite( SHIFT_LATCH, HIGH);
  digitalWrite( SHIFT_LATCH, LOW);
#else
  //get top - byte of A17,A16 bit address
  byte top = address >> 16;
  byte hi = address >> 8;
  byte low = address & 0xff;

  //disable latch line
  bitClear(PORTC,PORTC_LATCH);

  //shift out
  fastShiftOut(top);
  fastShiftOut(hi);
  fastShiftOut(low);

  //enable latch and set address
  bitSet(PORTC,PORTC_LATCH);
#endif
}

#ifdef ARDUINO_NANO
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

byte read_byte( unsigned long address ){
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

void write_byte(unsigned long address, byte data) {
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

void boot_block_disable(void) {
  write_byte(0x5555,0xaa);
  write_byte(0x2aaa,0x55);
  write_byte(0x5555,0x80);
  write_byte(0x5555,0xaa);
  write_byte(0x2aaa,0x55);
  write_byte(0x5555,0x40);
  //write_byte(0x00000,0x00);   //First 8K Boot
  //write_byte(0x3ffff,0xff);   //Last 8K Boot
  write_byte(0x2aaa,0xaa);
  delay(10);
  Serial.println("boot_block_last_8k");
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
  
  delay(50);
  Serial.println("erase_eeprom");
}

void idread_eeprom( void ) {
  byte device_code,manufacture_code,a2,a3;
  long end_addr = 0;
  
  write_byte(0x5555,0xaa);
  write_byte(0x2aaa,0x55);
  write_byte(0x5555,0x90);
  //write_byte(0x5555,0xaa);
  //write_byte(0x2aaa,0x55);
  //write_byte(0x5555,0x60);

  delayMicroseconds(10);

  manufacture_code = read_byte(0x00000);
  device_code = read_byte(0x00001);
  a2 = read_byte(0x00002);
  a3 = read_byte(0x3fff2);

  Serial.print(a2,HEX);
  Serial.print(" ");
  Serial.print(a3,HEX);
  Serial.println(" ");
    
  Serial.print("EEPROM Manufacture Code : 0x");
  Serial.println(manufacture_code, HEX);
  Serial.print("ROM Device Code : ");

  switch(device_code){
    case AT29C256:
      Serial.print("(AT29C256) Sector Size : 64 ");
      sectorsize =  64;
      end_addr   = 0x7fff;                            
      break;

    case AT29C512:
      Serial.print("(AT29C512) Sector Size : 128 ");
      sectorsize = 128; 
      end_addr   = 0xffff; 
      break;

    case AT29C010A:
      Serial.print("(AT29C010A) Sector Size : ");
      sectorsize = 128; 
      end_addr = 0x1ffff;
      break;

    case W29C020:
      Serial.print("(W29C020) Sector Size : 128 ");
      sectorsize = 128; 
      end_addr = 0x3ffff;
      break;

    default:
      Serial.print("Unknown Deivce Code 0x");
      Serial.print(device_code, HEX);
      Serial.println(" ");
      sectorsize = 0; 
      end_addr = 0x0000;
      break;
  }
  write_byte(0x5555,0xaa);
  write_byte(0x2aaa,0x55);
  write_byte(0x5555,0xf0);
 
  delayMicroseconds(10);
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
byte hex_to_Byte(char* a){
  return ((hexToDigit(a[0])*16) + hexToDigit(a[1]));
}

/************************************************************
 * convert a hex word (0000 - ffff) to unsigned int
 * @param c-string with the hex value of the word
 * @return unsigned int represented by the digits 
 ************************************************************/
unsigned int hex_to_Word(char* data) {
  return ( (unsigned int)hexToDigit(data[0])*4096 
    + (unsigned int)hexToDigit(data[1])*256
    + (unsigned int)hexToDigit(data[2])*16
    + (unsigned int)hexToDigit(data[3]));
}

/************************************************************
 * convert a hex word (0000 - fffff) to unsigned int
 * @param c-string with the hex value of the word
 * @return unsigned int represented by the digits 
 ************************************************************/
unsigned long hex_to_Word_3byte(char* data) {
  return (  (unsigned long)hexToDigit(data[0])*65536 
    + (unsigned long)hexToDigit(data[1])*4096
    + (unsigned long)hexToDigit(data[2])*256 
    + (unsigned long)hexToDigit(data[3])*16 
    + (unsigned long)hexToDigit(data[4]));
}

/**
 * read a data block from eeprom and write out a hex dump 
 * of the data to serial connection
 * @param from       start address to read fromm
 * @param to         last address to read from
 * @param linelength how many hex values are written in one line
 **/
void read_block(unsigned long from, unsigned long to, int linelength) {
  //count the number fo values that are already printed out on the
  //current line
  int    outcount = 0;
  //loop from "from address" to "to address" (included)
  for (unsigned long address = from; address <= to; address++) {
    if (outcount == 0) {
      //print out the address at the beginning of the line
      Serial.println();
      Serial.print("0x0");
      print_Address(address);
      Serial.print("  ");
    }
    //print data, separated by a space
    byte data = read_byte(address);
    print_Byte(data);
    Serial.print(" ");
    outcount = (++outcount % linelength);

  }
  //print a newline after the last data line
  Serial.println();
}

/**
 * read a data block from eeprom and write out the binary data a
 * to the serial connection
 * @param from       start address to read fromm
 * @param to         last address to read from
 **/
void read_binblock(unsigned long from, unsigned long to) {
  for (unsigned long address = from; address <= to; address++) {
    Serial.flush();
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
void write_block(unsigned long address, byte* buffer, unsigned long len) {
  enable_protection_eeprom();

  for (unsigned long i = 0; i < len; i++) {
    write_byte(address+i,buffer[i]);
  }
  delay(10); //millisec
}

/**
 * print out a 16 bit word as 4 character hex value
 **/
void print_Address(unsigned long address) {
  if(address < 0x0010) Serial.print("0");
  if(address < 0x0100) Serial.print("0");
  if(address < 0x1000) Serial.print("0");
  if(address < 0x10000) Serial.print("0");
  Serial.print(address, HEX);
}

/**
 * print out a byte as 2 character hex value
 **/
void print_Byte(byte data) {
  if(data < 0x10) Serial.print("0");
  Serial.print(data, HEX);  
}

void display_byte(byte data){
  digitalWrite(HEX_DISP_5, data & 0x01);
  digitalWrite(HEX_DISP_6, (data & 0x02) >> 1);
  digitalWrite(HEX_DISP_9, (data & 0x04) >> 2);
  digitalWrite(HEX_DISP_10, (data & 0x08) >> 3);
  //delayMicroseconds(1);     //wait some time to finish writing
}

//waits for a string submitted via serial connection
//returns only if linebreak is sent or the buffer is filled
void command_read(void) {
  unsigned long timer = 0, count=0;
  //first clear command buffer
  for(int i=0; i< COMMANDSIZE;i++) cmdbuf[i] = 0;
  //initialize variables
  char c = ' ';
  int idx = 0;
  //now read serial data until linebreak or buffer is full
  do {
    timer++;
    if(timer%50000 == 0){
       display_byte(count++);
    }
    if( Serial.available()) {
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
byte command_parse(void) {
  //set ',' to '\0' terminator (command string has a fixed strucure)
  //first string is the command character
  cmdbuf[1]  = 0;
  //second string is startaddress (5 bytes)
  cmdbuf[8]  = 0;
  //third string is endaddress (5 bytes)
  cmdbuf[15] = 0;
  //fourth string is length (2 bytes)
  cmdbuf[18] = 0;

  startAddress = hex_to_Word_3byte(cmdbuf+3);
  dataLength = hex_to_Word_3byte(cmdbuf+10);
  lineLength = hex_to_Byte(cmdbuf+16);

  byte retval = 0;
  
  switch(cmdbuf[0]) {
    case 'R': retval = READ_HEX;  break;
    case 'r': retval = READ_BIN;  break;
    case 'W': retval = WRITE_HEX;  break;
    case 'w': retval = WRITE_BIN;  break;
    case 'v': retval = VERSION;    break;

    case 't': retval = TEST_WRITE;  break;
    case 'a': retval = TEST_ADDRESS; break;
    case 'b': retval = TEST_READ_BIN; break;

    case 'i': idread_eeprom(); break;
    case 'e': erase_eeprom();  break;
    case 'k': boot_block_disable(); break;

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

  //pinMode(IND_LED, OUTPUT);
  //digitalWrite(IND_LED, LOW);

  pinMode(HEX_DISP_5, OUTPUT);
  pinMode(HEX_DISP_6, OUTPUT);
  pinMode(HEX_DISP_9, OUTPUT);
  pinMode(HEX_DISP_10, OUTPUT);
  display_byte(0);
    
  //set speed of serial connection
  Serial.begin(115200);

  //digitalWrite(IND_LED, LOW);
  Serial.println("Boot V2 Ok!");
}

void loop() {
  command_read();
  byte cmd = command_parse();
  int bytes = 0;
  static byte i = 0;

  switch(cmd) {
    case READ_HEX:  //Local Display in terminal Screen
      display_byte(0xb);
      if(lineLength == 0){
        lineLength = 16;
      }
      endAddress = startAddress + dataLength -1;
      read_block(startAddress,endAddress,lineLength);      
      Serial.println('%');      
      break;

    case READ_BIN:  //ROM -> WRITTER
      display_byte(0xb);
      endAddress = startAddress + dataLength -1;
      read_binblock(startAddress,endAddress);
      break;

    case WRITE_BIN:  //WRITTER->ROM
      display_byte(0xf);
      bytes = 0;
      //take care for max buffer size
      if(dataLength > BUFFERSIZE){
        dataLength = BUFFERSIZE;
      }
      while(bytes < dataLength) {
        if(Serial.available())
          buffer[bytes++] = Serial.read();
      }
      write_block(startAddress, buffer, dataLength);
      Serial.println('%');
      break;

    case TEST_WRITE:     //29C256 Flash_ROM
      enable_protection_eeprom();
      write_block(0x08000, buffer, 128);
      write_block(0x08080, buffer, 128);
      break;

    case TEST_ADDRESS:
      startAddress = 0x00000;
      endAddress = 0x00fff;
      lineLength = 0x20;
      read_block(startAddress,endAddress,lineLength);      
      break;
      
    case TEST_READ_BIN:
      startAddress = 0x00000;
      endAddress = 0x00fff;
      read_binblock(startAddress,endAddress); 
      break;

    case VERSION:
      Serial.println(VERSIONSTRING);
      break;
  }
}
