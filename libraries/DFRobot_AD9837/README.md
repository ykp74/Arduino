# DFRobot_AD9837
* [中文版](./README_CN.md)

The AD9837 is a low power, programmable waveform generator capable of producing sine, triangular, and square wave outputs. Waveform generation is required in various types of sensing, actuation, and time domain reflectometry (TDR) applications. The output frequency and phase are software programmable, allowing easy tuning. The frequency registers are 28 bits wide: with a 16 MHz clock rate, resolution of 0.06 Hz can be achieved; with a 5 MHz clock rate, the AD9837 can be tuned to 0.02 Hz 
resolution.

The AD9837 is written to via a 3-wire serial interface. This serial interface operates at clock rates up to 40 MHz and is compatible with DSP and microcontroller standards. The device operates with a power supply from 2.3 V to 5.5 V. The AD9837 has a power-down (sleep) function. Sections of the device that are not being used can be powered down to minimize the current consumption of the part. For example, the DAC can be powered down when a clock output is being generated.

![产品实物图](./resources/images/AD9837.png)


## Product Link (https://www.dfrobot.com/)
    SKU: DFR0973


## Table of Contents

* [Summary](#summary)
* [Installation](#installation)
* [Methods](#methods)
* [Compatibility](#compatibility)
* [History](#history)
* [Credits](#credits)


## Summary

* Digitally programmable frequency and phase
* 8.5 mW power consumption at 2.3 V
* MCLK speed: 16 MHz (B grade), 5 MHz (A grade)
* 28-bit resolution: 0.06 Hz at 16 MHz reference clock
* Sinusoidal, triangular, and square wave outputs
* 2.3 V to 5.5 V power supply
* 3-wire SPI interface
* Extended temperature range: −40°C to +125°C
* Power-down option


## Installation

To use this library, first download the library file, paste it into the \Arduino\libraries directory, 
then open the examples folder and run the demo in the folder.


## Methods

```C++

/************************ Init ********************************/
  /**
   * @fn DFRobot_AD9837
   * @brief Constructor
   * @param fscPin - active-low control pin, pull down for SPI data transfer
   * @param pSpi - extern SPIClass SPI is defined in SPI.h; so just get SPI object address and the methods in SPI can be pointed to and used
   * @return None
   */
  DFRobot_AD9837(int8_t fscPin=5, SPIClass *pSpi=&SPI);

  /**
   * @fn begin
   * @brief Init function, including SPI communication init, module power-on reset
   * @param frequency - the maximum frequency of SPI communication
   * @return None
   */
  void begin(uint32_t frequency=20000000);

/************************** Config function ******************************/
  /**
   * @fn moduleReset
   * @brief Reset internal registers to 0, this corresponds to an analog output of midscale.
   * @param mode - eReset_EN: reset internal register; eReset_DIS: quit reset status
   * @return None
   * @note RESET operation will not reset the phase, frequency, or control register.
   */
  void moduleReset(eControlRegBit_t mode=eReset_DIS);

  /**
   * @fn moduleSleep
   * @brief The parts of AD9837 that are not in use like the internal clock and DAC can be powered down through the sleep function to minimize power consumption. 
   * @param MCLK - eSleepMCLK_NO: enable internal MCLK; eSleepMCLK_YES: disable internal MCLK. The DAC output remains at its present value as the NCO is no longer
   * @     accumulating.
   * @param dacMode - eSleepDAC_NO: keep DAC active; eSleepDAC_YES: power down on-chip DAC. This is useful when AD9837 is used to output MSB of DAC data.
   * @return None
   * @note  RESET operation will not reset the phase, frequency, or control register.
   */
  void moduleSleep(eControlRegBit_t MCLK=eSleepMCLK_NO, eControlRegBit_t dacMode=eSleepDAC_NO);

/************************** Performance function ******************************/
  /**
   * @fn outputSin
   * @brief OUT  The pin outputs sine wave
   * @param phase - Adjust phase offset, range: 0-360, unit: degree (corresponding to waveform phase offset of 0-2π)
   * @param freq - wave frequency, range: 0.06-8000000.00, unit: Hz
   * @return None
   * @note Recommended frequency range: 60-600000Hz, actual resolution is about 0.06Hz, which is obtained by dividing 16MHz by 2^28 
   */
  void outputSin(uint16_t phase, float freq);

  /**
   * @fn outputTriangle
   * @brief OUT  The pin outputs triangle wave
   * @param phase - adjust phase offset, range: 0-360, unit: degree (corresponding to waveform phase offset of 0-2π)
   * @param freq - wave frequency, range: 0.06-8000000.00, unit: Hz
   * @return None
   * @note Recommended frequency range: 60-600000Hz, actual resolution is about 0.06Hz, which is obtained by dividing 16MHz by 2^28
   */
  void outputTriangle(uint16_t phase, float freq);

  /**
   * @fn outputSquare
   * @brief OUT  The pin outputs square wave
   * @param divide - eDIV2_2: MSB/2 of DAC data is output through VOUT pin; eDIV2_1: MSB of DAC data is output via VOUT pin.
   * @param phase - adjust phase offset, range: 0-360, unit: degree (corresponding to waveform phase offset of 0-2π)
   * @param freq - wave frequency, range: 0.06-8000000.00, unit: Hz
   * @return None
   * @note Recommended frequency range: 60-600000Hz, actual resolution is about 0.06Hz, which is obtained by dividing 16MHz by 2^28
   */
  void outputSquare(eControlRegBit_t divide, uint16_t phase, float freq);

```


## Compatibility

MCU                | Work Well    | Work Wrong   | Untested    | Remarks
------------------ | :----------: | :----------: | :---------: | :----:
Arduino Uno        |      √       |              |             |
Arduino MEGA2560   |      √       |              |             |
Arduino Leonardo   |      √       |              |             |
FireBeetle-ESP8266 |      √       |              |             |
FireBeetle-ESP32   |      √       |              |             |
FireBeetle-M0      |      √       |              |             |
Micro:bit          |      √       |              |             |


## History

- 2022/03/11 - Version 1.0.0 released.


## Credits

Written by qsjhyy(yihuan.huang@dfrobot.com), 2022. (Welcome to our [website](https://www.dfrobot.com/))

