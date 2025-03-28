#define SHIFT_DATA 2
#define SHIFT_CLK 3
#define SHIFT_LATCH 4

#define EEPROM_D0 5
#define EEPROM_D7 12

#define WRITE_EN 13

byte data[] = {0x01, 0x4f, 0x12,0x06,0x4c,0x24,0x20,0x0f,0x00,0x04,0x08,0x60,0x31,0x42,0x30,0x38};

void setAddress(int address, bool outputEnable) {
  shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, (address >> 8) | (outputEnable ? 0x00:0x80) );  //chip enable
  shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, address);

  digitalWrite( SHIFT_LATCH, LOW);
  digitalWrite( SHIFT_LATCH, HIGH);
  digitalWrite( SHIFT_LATCH, LOW);
}

byte readEEPROM(int address){
  /* Pin Mode should be set INPUT for reading*/
  for(int pin = EEPROM_D0; pin <= EEPROM_D7; pin += 1){
    pinMode(pin, INPUT); 
  }

  setAddress( address, true );  //OutputMode is 0x00 false
  byte data =0;
  for(int pin = EEPROM_D7; pin >= EEPROM_D0; pin -= 1){
    data =(data << 1)+ digitalRead(pin);
  }
  return data;
}

void writeEEPROM(int address, byte data){
  /* Pin Mode should be set OUTPUT for Writing */
  for(int pin = EEPROM_D0; pin <= EEPROM_D7; pin += 1){
    pinMode(pin, OUTPUT); 
  }

  setAddress(address, false); //OutputMode is 0x80

  for(int pin = EEPROM_D0; pin <= EEPROM_D7; pin += 1){
    digitalWrite(pin, data & 1);
    data = data >> 1;
  }

  digitalWrite(WRITE_EN, LOW);
  delayMicroseconds(1);
  digitalWrite(WRITE_EN, HIGH);
  delay(10);
}

void printContents(){
 Serial.println("Print EEPROM Data!!");
 for(int base = 0; base <= 2047; base += 16){
    byte data[16];
    for( int offset = 0; offset <=15; offset += 1){
      data[offset] = readEEPROM( base + offset );
    }

    char buf[80];
    sprintf(buf,"%04x : %02x %02x %02x %02x %02x %02x %02x %02x   %02x %02x %02x %02x %02x %02x %02x %02x",
      base, data[0],data[1],data[2],data[3],data[4],data[5],data[6],data[7],
            data[8],data[9],data[10],data[11],data[12],data[13],data[14],data[15]);
     Serial.println(buf);
   }  
}

void setup() {
  // put your setup code here, to run once:
  pinMode(SHIFT_DATA, OUTPUT);
  pinMode(SHIFT_CLK, OUTPUT);
  pinMode(SHIFT_LATCH, OUTPUT);

  pinMode(WRITE_EN, OUTPUT);
  digitalWrite( WRITE_EN, HIGH);

  Serial.begin(115200);

  Serial.println("writeEEPROM!!");
  // for(int address = 0; address <=15; address +=1){
  //   writeEEPROM(address,data[address]);
  // }

  printContents();
}

void loop() {
  // put your main code here, to run repeatedly:

}
