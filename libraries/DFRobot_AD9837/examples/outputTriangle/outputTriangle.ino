/*!
 * @file  outputTriangle.ino
 * @brief  Drive the AD9837 via SPI to output a triangular wave signal
 * @details  Configure the communication pin to be used and properly connect the AD9837 to the main controller
 * @n  Reset the AD9837 or power down the internal clock and DAC of AD9837
 * @n  Set the frequency and phase offset of the triangular wave signal
 * @note  The amplitude of the triangular wave signal is about 0.645V; the wave phase offset must be observed with reference.
 * @copyright  Copyright (c) 2010 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @license  The MIT License (MIT)
 * @author  [qsjhyy](yihuan.huang@dfrobot.com)
 * @version  V1.0
 * @date  2022-03-28
 * @url  https://github.com/DFRobot/DFRobot_AD9837
 */
#include <DFRobot_AD9837.h>

/**
 * @brief Constructor
 * @param fscPin - active-low control pin, pull down for SPI data transfer
 * @param pSpi - extern SPIClass SPI is defined in SPI.h; so just get SPI object address and the methods in SPI can be pointed to and used
 * @return None
 * @note Use SPI interface for communication with AD9837, but AD9837 lacks MISO, i.e. AD9837 only receives data and does not transmit
 * @n DAT pin---(connect to)---MCU MOSI; 
 * @n CLK pin---(connect to)---MCU SCK; 
 * @n FSC pin---(connect to)---the following ssPin (it can be changed according to the actual situation)
 * @n Set up digital pin according to the on-board pin connected with SPI chip-select pin
 * @n | MCU SPI Default Pin |  MOSI  |  SCK   |  MISO  |   | Recommended ssPin  |   | Module VCC Pin |
 * @n |---------------------------------------------|   |-----------------|   |------------|
 * @n |       UNO        | GPIO11 | GPIO13 | GPIO12 |   |   5 (GPIO5)     |   |     5V     |
 * @n |     MEGA2560     | GPIO51 | GPIO52 | GPIO50 |   |   5 (GPIO5)     |   |     5V     |
 * @n |     Leonardo     |       ICSP        |   |   5 (GPIO5)     |   |     5V     |
 * @n |        m0        |   MO   |  SCK   |   MI   |   |   5 (GPIO5)     |   |    3.3V    |
 * @n |     microbit     |   P15  |  P13   |  P14   |   |   P5 (GPIO5)    |   |    3.3V    |
 * @n |      ESP32       | GPIO23 | GPIO18 | GPIO19 |   |   D8 (GPIO5)    |   |    3.3V    |
 * @n |     ESP32-E      | GPIO23 | GPIO18 | GPIO19 |   |   D2 (GPIO25)   |   |    3.3V    |
 * @n |     ESP8266      |  MOSI  |  SCK   |  MISO  |   |   D6 (GPIO5)    |   |    3.3V    |
 */
uint8_t ssPin = 5;
DFRobot_AD9837 AD9837(/* fscPin= */ssPin, /* *pSpi= */&SPI);   // Initialize an instance, through which the class interface is called

void setup(void)
{
  Serial.begin(115200);

  /**
   * @brief Init function, including SPI communication init, module power-on reset
   * @param frequency - the maximum frequency of SPI communication
   * @return return true if init succeeded
   */
  AD9837.begin(/* frequency= */10000000);
  Serial.println("AD9837 begin ok!");

  /**
   * @brief Reset internal register to 0, this corresponds to an analog output of midscale.
   * @param mode - eReset_EN: reset internal register; eReset_DIS: quit reset status
   * @return None
   * @note A reset will not reset the phase, frequency, or control register.
   */
  AD9837.moduleReset(/* mode= */AD9837.eReset_DIS);

  /**
   * @brief The parts of AD9837 that are not in use like the internal clock and DAC can be powered down through the sleep function to minimize power consumption.
   * @param MCLK - eSleepMCLK_NO: enable internal MCLK; eSleepMCLK_YES: disable internal MCLK. The DAC output remains at its current value because NCO is no longer accumulating.
   * @param dacMode - eSleepDAC_NO: keep DAC active; eSleepDAC_YES: power down on-chip DAC. This is useful when AD9837 is used to output MSB of DAC data.
   * @return None
   * @note RESET operation will not reset the phase, frequency, or control register.
   */
  AD9837.moduleSleep(/* MCLK= */AD9837.eSleepMCLK_NO, /* dacMode= */AD9837.eSleepDAC_NO);

  /**
   * @brief OUT  The pin outputs triangle wave
   * @param phase - adjust phase offset, range: 0-360, unit: degree (corresponding to waveform phase offset of 0-2π)
   * @param freq - wave frequency, range: 0.06-8000000.00, unit: Hz
   * @return None
   * @note Recommended frequency range: 60-600000Hz, actual resolution is about 0.06Hz, which is obtained by dividing 16MHz into 2^28 
   */
  AD9837.outputTriangle(/* phase= */0, /* freq= */1000.0);
}

void loop()
{
  delay(100);
}
