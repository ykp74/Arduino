//Crowduino : 타겟설정, Arduino M0(native USB Port) 설정권장.
//테스트 및 빌드 확인 코드.
//#include <Adafruit_GFX.h>  //OLED Driver header
#include <FreeRTOS_SAMD21.h>
#include <semphr.h>

#include <SD.h>
File root;

#include <DFRobot_AD9837.h>
#include <SPI.h>

const uint8_t AD9837_CS_PIN = 5;
const uint8_t SD_CS_PIN = 4;
const uint32_t SERIAL_WAIT_TIMEOUT_MS = 3000;
const SPISettings AD9837_SPI_SETTINGS(1000000, MSBFIRST, SPI_MODE2);
// Initialize an instance, through which the class interface is called.
DFRobot_AD9837 AD9837(/* fscPin= */ AD9837_CS_PIN, /* *pSpi= */ &SPI);

#define ERROR_LED_PIN 13              //Led Pin: Typical Arduino Board
//#define ERROR_LED_LIGHTUP_STATE HIGH  // the state that makes the led light up on your board, either low or high
#define SERIAL SerialUSB              //Sparkfun Samd21 Boards

//**************************************************************************
// global variables
//**************************************************************************
//TaskHandle_t Handle_aTask;
//TaskHandle_t Handle_bTask;
TaskHandle_t Handle_thread_input_control;
TaskHandle_t Handle_thread_monitorTask;
TaskHandle_t Handle_thread_led_control;
TaskHandle_t Handle_thread_sd_memory;

SemaphoreHandle_t Handle_spi_mutex;

//**************************************************************************
// Can use these function for RTOS delays
// Takes into account processor speed
// Use these instead of delay(...) in rtos tasks
//**************************************************************************
// void myDelayUs(int us) {
//   vTaskDelay(us / portTICK_PERIOD_US);
// }

void myDelayMs(int ms) {
  vTaskDelay((ms * 1000) / portTICK_PERIOD_US);
}

// void myDelayMsUntil(TickType_t *previousWakeTime, int ms) {
//   vTaskDelayUntil(previousWakeTime, (ms * 1000) / portTICK_PERIOD_US);
// }

bool lockSpi(TickType_t timeoutTicks = portMAX_DELAY) {
  return (Handle_spi_mutex != NULL) && (xSemaphoreTake(Handle_spi_mutex, timeoutTicks) == pdTRUE);
}

void unlockSpi() {
  if (Handle_spi_mutex != NULL) {
    xSemaphoreGive(Handle_spi_mutex);
  }
}

void printDirectory(File dir, int numTabs)
{
  while (true) {
    File entry = dir.openNextFile();
    if (!entry) {
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

#if 0
//*****************************************************************
// Create a thread that prints out A to the screen every two seconds
// this task will delete its self after printing out afew messages
//*****************************************************************
static void threadA(void *pvParameters) 
{
  SERIAL.println("Thread A: Started");

  for (int x = 0; x < 100; ++x) {
    SERIAL.print("A");
    SERIAL.flush();
    myDelayMs(500);
  }

  // delete ourselves.
  // Have to call this or the system crashes when you reach the end bracket and then get scheduled.
  SERIAL.println("Thread A: Deleting");
  vTaskDelete(NULL);
}

//*****************************************************************
// Create a thread that prints out B to the screen every second
// this task will run forever
//*****************************************************************
static void threadB(void *pvParameters)
{
  SERIAL.println("Thread B: Started");

  while (1) {
    SERIAL.println("B");
    SERIAL.flush();
    myDelayMs(2000);
  }
}
#endif

static void thread_input_control(void *pvParameters) 
{
  SERIAL.println("thread_input_control : Started");

  while (1) {
    if (SERIAL.available()) {
      char inputChar = SERIAL.read();
      switch (inputChar) {
        case 'r':
        case 'R':
          SERIAL.println("Reset!!!!!!!!");
          myDelayMs(500); // 메시지가 출력될 시간을 제공
          NVIC_SystemReset();  // 소프트웨어 리셋 함수 호출
          break;
        case 'a':
          SERIAL.println("a...");
          if (lockSpi(pdMS_TO_TICKS(500))) {
            AD9837.moduleReset(/* mode= */ AD9837.eReset_DIS);
            AD9837.outputSquare(/* divide= */ AD9837.eDIV2_1, /* phase= */ 0, /* freq= */ 10); //Output Frequency Hz
            unlockSpi();
          }
          break;
        case 'b':
          SERIAL.println("b...");
          if (lockSpi(pdMS_TO_TICKS(500))) {
            AD9837.moduleReset(/* mode= */ AD9837.eReset_DIS);
            AD9837.outputSquare(/* divide= */ AD9837.eDIV2_1, /* phase= */ 0, /* freq= */ 10000); //Output Frequency Hz
            unlockSpi();
          }
          break;
        case 's':
          SERIAL.println("s...");
          if (lockSpi(pdMS_TO_TICKS(500))) {
            AD9837.moduleReset(/* mode= */ AD9837.eReset_EN);
            unlockSpi();
          }
          break;
      }
    } else {
      myDelayMs(10);
    }
  }

  // delete ourselves.
  // Have to call this or the system crashes when you reach the end bracket and then get scheduled.
  SERIAL.println("Task Monitor: Deleting");
  vTaskDelete(NULL);
}

static void thread_led_control(void *pvParameters) 
{
    SERIAL.println("thread_led_control : Started");

    while (1) {
        digitalWrite(ERROR_LED_PIN, true);
        //AD9837.outputSquare(/* divide= */AD9837.eDIV2_1, /* phase= */0, /* freq= */1);
        // SERIAL.println("B");
        // SERIAL.flush();
        myDelayMs(1000);
        digitalWrite(ERROR_LED_PIN, false);
        //AD9837.outputSquare(/* divide= */AD9837.eDIV2_1, /* phase= */0, /* freq= */5);
        myDelayMs(1000);
    }

    // delete ourselves.
    // Have to call this or the system crashes when you reach the end bracket and then get scheduled.
    SERIAL.println("Task Monitor: Deleting");
    vTaskDelete(NULL);
}

static void thread_sd_memory(void *pvParameters) 
{
    SERIAL.println("thread_sd_memory : Started");
    bool spiLocked = false;

    //Init SD Memory Card and read the file name
    SERIAL.print("Initializing SD card...");
    spiLocked = lockSpi(pdMS_TO_TICKS(500));
    if (!spiLocked) {
        SERIAL.println("spi mutex timeout!");
    } else if (!SD.begin(SD_CS_PIN)) {
        SERIAL.println("initialization failed!");
    } else {
        SERIAL.println("initialization done.");
        root = SD.open("/");
        if (root) {
          printDirectory(root, 0);
          root.close();
        }
        SERIAL.println("done!");
    }

    if (spiLocked) {
        unlockSpi();
    }

    while (1) {
        //SERIAL.println("thread_sd_memory : Processiung!!");
        SERIAL.flush();
        myDelayMs(2000);
    }

    // delete ourselves.
    // Have to call this or the system crashes when you reach the end bracket and then get scheduled.
    SERIAL.println("Task Monitor: Deleting");
    vTaskDelete(NULL);
}

//*****************************************************************
// Task will periodically print out useful information about the tasks running
// Is a useful tool to help figure out stack sizes being used
// Run time stats are generated from all task timing collected since startup
// No easy way yet to clear the run time stats yet
//*****************************************************************
static void thread_Monitor(void *pvParameters) 
{
    static char ptrTaskList[768];  //temporary string buffer for task stats

    int x;
    int measurement;

    SERIAL.println("Task Monitor: Started");

    // run this task afew times before exiting forever
    while (1) {
        myDelayMs(10000);  // print every 10 seconds

        SERIAL.flush();
        SERIAL.println("");
        SERIAL.println("****************************************************");
        SERIAL.print("Free Heap: ");
        SERIAL.print(xPortGetFreeHeapSize());
        SERIAL.println(" bytes");

        SERIAL.print("Min Heap: ");
        SERIAL.print(xPortGetMinimumEverFreeHeapSize());
        SERIAL.println(" bytes");
        SERIAL.flush();

        SERIAL.println("****************************************************");
        SERIAL.println("Task            ABS             %Util");
        SERIAL.println("****************************************************");

        vTaskGetRunTimeStats(ptrTaskList);  //save stats to char array
        SERIAL.println(ptrTaskList);        //prints out already formatted stats
        SERIAL.flush();

        SERIAL.println("****************************************************");
        SERIAL.println("Task            State   Prio    Stack   Num     Core");
        SERIAL.println("****************************************************");

        vTaskList(ptrTaskList);       //save stats to char array
        SERIAL.println(ptrTaskList);  //prints out already formatted stats
        SERIAL.flush();

        SERIAL.println("****************************************************");
        SERIAL.println("[Stacks Free Bytes Remaining] ");

        measurement = uxTaskGetStackHighWaterMark(Handle_thread_sd_memory);
        SERIAL.print("Thread SD Memory ");
        SERIAL.println(measurement);

        measurement = uxTaskGetStackHighWaterMark(Handle_thread_input_control);
        SERIAL.print("Thread Input Control ");
        SERIAL.println(measurement);

        measurement = uxTaskGetStackHighWaterMark(Handle_thread_led_control);
        SERIAL.print("Thread LED Control ");
        SERIAL.println(measurement);

        measurement = uxTaskGetStackHighWaterMark(Handle_thread_monitorTask);
        SERIAL.print("Monitor Stack: ");
        SERIAL.println(measurement);

        SERIAL.println("****************************************************");
        SERIAL.flush();
    }

    // delete ourselves.
    // Have to call this or the system crashes when you reach the end bracket and then get scheduled.
    SERIAL.println("Task Monitor: Deleting");
    vTaskDelete(NULL);
}

void softwareReset() {
    NVIC_SystemReset();  // 소프트웨어 리셋 함수 호출
}

void init_ad9837(void) 
{
    //Function generator
    if (!lockSpi(pdMS_TO_TICKS(500))) {
      SerialUSB.println("AD9837 init skipped: SPI mutex unavailable");
      return;
    }

    AD9837.begin(/* frequency= */ 10000000);
    AD9837.moduleReset(/* mode= */ AD9837.eReset_DIS);
    AD9837.moduleSleep(/* MCLK= */ AD9837.eSleepMCLK_NO, /* dacMode= */ AD9837.eSleepDAC_NO);
    AD9837.outputSquare(/* divide= */ AD9837.eDIV2_1, /* phase= */ 0, /* freq= */ 1); //Output Frequency Hz
    unlockSpi();

    SerialUSB.println("AD9837 begin ok!");

    // SPI 초기화
    // SPI.begin();
    // SPI.setClockDivider(SPI_CLOCK_DIV64); // SPI 클럭 속도 설정
    // SPI.setDataMode(SPI_MODE1);          // AD9837은 SPI 모드 2 사용 (CPOL=1, CPHA=0)
    // SPI.setBitOrder(MSBFIRST);           // MSB부터 전송
    
    // pinMode(CS_PIN, OUTPUT);  // CS 핀 설정
    // digitalWrite(CS_PIN, HIGH); // 기본적으로 비활성화

    // // AD9837 리셋
    // resetAD9837();
    // setFrequency(frequency); // 초기 주파수 설정
}

void setup() {
  // Open serial communications and wait for port to open:
  //M0 에서는 시리얼출력시에는 USB로 설정해야 한다. Serial이 3개로 지원되기 때문
  SerialUSB.begin(115200);

  uint32_t serialWaitStart = millis();
  while (!SerialUSB && (millis() - serialWaitStart < SERIAL_WAIT_TIMEOUT_MS)) {
    delay(10);  // wait briefly for serial port to connect. Needed for native USB port only
  }

  SERIAL.println("Booting!!!!!!!!!!!");

  pinMode(AD9837_CS_PIN, OUTPUT);
  pinMode(SD_CS_PIN, OUTPUT);

  Handle_spi_mutex = xSemaphoreCreateMutex();
  if (Handle_spi_mutex == NULL) {
    SERIAL.println("SPI mutex create failed!");
  }
  
  //init AD9837 process
  init_ad9837();

  pinMode(ERROR_LED_PIN, OUTPUT);
  digitalWrite(ERROR_LED_PIN, LOW);

  // Create the threads that will be managed by the rtos
  // Sets the stack size and priority of each task
  // Also initializes a handler pointer to each task, which are important to communicate with and retrieve info from tasks

  //xTaskCreate(threadA,     "Task A",       256, NULL, tskIDLE_PRIORITY + 3, &Handle_aTask);
  //xTaskCreate(threadB,     "Task B",       256, NULL, tskIDLE_PRIORITY + 2, &Handle_bTask);
  xTaskCreate(thread_input_control, "Task Input", 256, NULL,    tskIDLE_PRIORITY + 1, &Handle_thread_input_control);
  xTaskCreate(thread_sd_memory,     "Task SD", 256, NULL,       tskIDLE_PRIORITY + 2, &Handle_thread_sd_memory);
  xTaskCreate(thread_led_control,   "Task LED", 256, NULL,      tskIDLE_PRIORITY + 3, &Handle_thread_led_control);
  xTaskCreate(thread_Monitor,       "Task Monitor", 256, NULL,  tskIDLE_PRIORITY + 4, &Handle_thread_monitorTask);

  // Start the RTOS, this function will never return and will schedule the tasks.
  vTaskStartScheduler();

  // error scheduler failed to start
  // should never get here
   while(1){
     SERIAL.println("Scheduler Failed! \n");
     SERIAL.flush();
     myDelayMs(1000);
   }
}

void loop() {
  // nothing happens after setup finishes.
}
