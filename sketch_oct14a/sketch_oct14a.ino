//Crowduino : 타겟설정, Arduino M0(native USB Port) 설정권장.
//테스트 및 빌드 확인 코드.
#include <Adafruit_GFX.h>     //OLED Driver header

#include <SPI.h>
#include <SD.h>

File root;

void setup() {
  // Open serial communications and wait for port to open:
  SerialUSB.begin(115200);   //M0 에서는 시리얼출력시에는 USB로 설정해야 한다. Serial이 3개로 지원되기 때문

  while (!SerialUSB) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
  SerialUSB.print("Initializing SD card...");

  if (!SD.begin(4)) {
    SerialUSB.println("initialization failed!");
    while (1);
  }
  SerialUSB.println("initialization done.");

  root = SD.open("/");

  printDirectory(root, 0);

  SerialUSB.println("done!");

}

void loop() {
  // nothing happens after setup finishes.
}

void printDirectory(File dir, int numTabs) {
  while (true) {

    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++) {
      SerialUSB.print('\t');
    }
    SerialUSB.print(entry.name());
    if (entry.isDirectory()) {
      SerialUSB.println("/");
      printDirectory(entry, numTabs + 1);
    } else {
      // files have sizes, directories do not
      SerialUSB.print("\t\t");
      SerialUSB.println(entry.size(), DEC);
    }
    entry.close();
  }
}
