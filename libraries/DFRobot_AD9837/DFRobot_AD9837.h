/*!
 * @file  DFRobot_AD9837.h
 * @brief  Define infrastructure of DFRobot_AD9837 class
 * @details  Declaration-related function interfaces for setting waveform, frequency, phase, SPI communication, etc.
 * @copyright  Copyright (c) 2010 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @license  The MIT License (MIT)
 * @author  [qsjhyy](yihuan.huang@dfrobot.com)
 * @version  V1.0
 * @date  2022-03-03
 * @url  https://github.com/DFRobot/DFRobot_AD9837
 */
#ifndef __DFRobot_AD9837_H__
#define __DFRobot_AD9837_H__

#include <Arduino.h>
#include <SPI.h>


// #define ENABLE_DBG   //!< Open this macro and you can see the details of the program
#ifdef ENABLE_DBG
  #define DBG(str, ...) {Serial.print("[");Serial.print(__FUNCTION__); Serial.print("(): "); Serial.print(__LINE__); Serial.print("] "); Serial.print(str), Serial.println(__VA_ARGS__);}
#else
  #define DBG(str, ...)
#endif


/* AD9837 register address */
#define AD9837_CONTROL_REG    uint16_t(0x0000)   ///< control register, which is 16 bits

#define AD9837_FREQ0_REG      uint16_t(0x4000)   ///< frequency register 0, which is 28 bits
#define AD9837_FREQ1_REG      uint16_t(0x8000)   ///< frequency register 1, which is 28 bits
#define AD9837_PHASE0_REG     uint16_t(0xC000)   ///< phase register 0, which is 12 bits, so the b12 value can be set as per your need, the address can be 0xD000
#define AD9837_PHASE1_REG     uint16_t(0xE000)   ///< phase register 1, which is 12 bits, so the b12 value can be set as per your need, the address can be 0xF000


class DFRobot_AD9837
{
public:

/************************* Interrupt Pin Configuration *******************************/
  /**
   * @enum  eControlRegBit_t
   * @brief  Control register config enum
   */
  typedef enum
  {
    eB28_0 = 0<<13,   /**< B28 = 0, the 28-bit frequency register operates as two 14-bit registers, one containing the 14 MSBs and the other containing the 14 LSBs. */
    eB28_1 = 1,   /**< B28 = 1 allows a complete word to be loaded into a frequency register in two consecutive writes. */

    eHLB_L = 0<<12,   /**< HLB = 0 allows a write to the 14 LSBs of the addressed frequency register. */
    eHLB_M = 1,   /**< HLB = 1 allows a write to the 14 MSBs of the addressed frequency register. */

    eFSEL_0 = 0<<11,   /**< when bit FSEL = 0, the FREQ0 register defines the output frequency as a fraction of MCLK frequency */
    eFSEL_1 = 1,   /**< when bit FSEL = 1, the FREQ1 register defines the output frequency as a fraction of MCLK frequency */

    ePSEL_0 = 0<<10,   /**< when bit PSEL = 0, the content of PHASE0 register is added to the output of the phase accumulator */
    ePSEL_1 = 1,   /**< when bit PSEL = 1, the content of PHASE1 register is added to the output of the phase accumulator */

    eReset_DIS = 0<<8,   /**< RESET = 0 disables the reset function */
    eReset_EN = 1,   /**< RESET = 1 resets internal registers to 0, which corresponds to an analog output of midscale */

    eSleepMCLK_NO = 0<<7,   /**< SLEEP1 = 0 enables the internal MCLK */
    eSleepMCLK_YES = 1,   /**< SLEEP1 = 1 disables the internal MCLK. The DAC output remains at its current value because NCO is no longer accumulating. */

    eSleepDAC_NO = 0<<6,   /**< SLEEP2 = 0 indicates that the DAC is active */
    eSleepDAC_YES = 1,   /**< SLEEP2 = 1 powers down the on-chip DAC. This is useful when the AD9837 is used to output the MSB of the DAC data. */

    eOPBITEN_DAC = 0<<5,   /**< OPBITEN = 0 connects the DAC output to VOUT. */
    eOPBITEN_MSB = 1,   /**< When OPBITEN = 1, the VOUT pin does not provide the DAC output. Instead, the MSB (or MSB/2) of the DAC data is connected to the VOUT pin. */

    eDIV2_2 = 0<<3,   /**< DIV2 = 0, the MSB/2 of the DAC data output through the VOUT pin. */
    eDIV2_1 = 1,   /**< DIV2 = 1, the MSB of the DAC data output through the VOUT pin. */

    eMODE_SIN = 0<<1,   /**< MODE = 0, the SIN ROM converst the phase information into amplitude information, resulting in a sinusoidal signal at the output. */
    eMODE_TRI = 1,   /**< MODE = 1, bypasses the SIN ROM, resulting in a triangle save output from the DAC. */
  }eControlRegBit_t;

/************************* Module Control Register Structure *******************************/
  /**
   * @struct sControl_t
   * @brief Control register, the AD9837 contains a 16-bit control register.
   * @note Register structure:
   * @n -----------------------------------------------------------------------------------
   * @n |   b15    |  b14   |   b13   |   b12   |   b11   |   b10   |    b9    |    b8    |
   * @n -----------------------------------------------------------------------------------
   * @n |   regH   |  regL  |   B28   |   HLB   |   FSEL  |   PSEL  |reserved3 |  RESET   |
   * @n -----------------------------------------------------------------------------------
   * @n -----------------------------------------------------------------------------------
   * @n |    b7    |   b6   |    b5   |    b4    |    b3   |    b2    |   b1   |   b0     |
   * @n -----------------------------------------------------------------------------------
   * @n |  SLEEP1  | SLEEP2 | OPBITEN |reserved2 |  DIV2   |reserved1 |  MODE  |reserved0 |
   * @n -----------------------------------------------------------------------------------
   */
  typedef struct
  {
    uint16_t   reserved0: 1; /**< Reserved bit, this bit should be set to 0 */
    uint16_t   mode: 1; /**< MODE = 1, bypasses the SIN ROM, resulting in a triangle save output from the DAC. MODE = 0, the SIN ROM converts the phase information into amplitude information, resulting in a sinusoidal signal at the output. */
    uint16_t   reserved1: 1; /**< Reserved bit, this bit should be set to 0 */
    uint16_t   DIV2: 1; /**< DIV2 = 1,  the MSB of the DAC data output through the VOUT pin. DIV2 = 0, the MSB/2 of the DAC data output through the VOUT pin. */
    uint16_t   reserved2: 1; /**< Reserved bit, this bit should be set to 0 */
    uint16_t   OPBITEN: 1; /**< When OPBITEN = 1, the VOUT pin does not provide the DAC output. OPBITEN = 0 connects the DAC output to VOUT. */
    uint16_t   sleep2: 1; /**< SLEEP2 = 1 powers down the on-chip DAC. This is useful when the AD9837 is used to output the MSB of the DAC data. SLEEP2 = 0 indicates that the DAC is active. */
    uint16_t   sleep1: 1; /**< SLEEP1 = 1 disables the internal MCLK. The DAC output remains at its current value because NCO is no longer accumulating. SLEEP1 = 0 enables the internal MCLK. */
    uint16_t   reset: 1; /**< RESET = 1 resets internal registers to 0, which corresponds to an analog output of midscale; RESET = 0 disables the reset function */
    uint16_t   reserved3: 1; /**< Reserved bit, this bit should be set to 0 */
    uint16_t   PSEL: 1; /**< when bit PSEL = 0, the content of PHASE0 register is added to the output of the phase accumulator; when bit PSEL = 1, the PHASE1 register data is added to it */
    uint16_t   FSEL: 1; /**< when bit FSEL = 0, the FREQ0 register defines the output frequency as a fraction of MCLK frequency; when bit FSEL = 1, the FREQ1 register defines it */
    uint16_t   HLB: 1; /**< B28 = 0 determines writing to a register. 0: write to LSB, 1: write to MSB */
    uint16_t   B28: 1; /**< A complete word is loaded into a frequency register in two writes. 0: write to it as two 14-bit registers, 1: write to it as one 28-bit register */
    uint16_t   regL: 1; /**< register marker bit, low bit */
    uint16_t   regH: 1; /**< register marker bit, high bit */
  } __attribute__ ((packed)) sControl_t;

public:

/************************ Init ********************************/
  /**
   * @fn DFRobot_AD9837
   * @brief Constructor
   * @param fscPin - active-low control pin, pulled down for SPI data transfer
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
  void begin(uint32_t frequency=10000000);

/************************** Config function ******************************/
  /**
   * @fn moduleReset
   * @brief Reset internal register to 0, corresponding to analog output of midscale.
   * @param mode - eReset_EN: reset internal register; eReset_DIS: quit reset status
   * @return None
   * @note A reset will not reset the phase, frequency, or control register.
   */
  void moduleReset(eControlRegBit_t mode=eReset_DIS);

  /**
   * @fn moduleSleep
   * @brief The parts of AD9837 that are not in use like the internal clock and DAC can be powered down through the sleep function to minimize power consumption. 
   * @param MCLK - eSleepMCLK_NO: enable internal MCLK; eSleepMCLK_YES: disable internal MCLK. The DAC output remains at its current value because NCO is no longer accumulating.
   * @param dacMode - eSleepDAC_NO: keep DAC active; eSleepDAC_YES: power down on-chip DAC. This is useful when AD9837 is used to output MSB of DAC data.
   * @return None
   * @note A reset will not reset the phase, frequency, or control register.
   */
  void moduleSleep(eControlRegBit_t MCLK=eSleepMCLK_NO, eControlRegBit_t dacMode=eSleepDAC_NO);

/************************** Performance function ******************************/
  /**
   * @fn outputSin
   * @brief OUT  The pin outputs sine wave
   * @param phase - adjust phase offset, range: 0-360, unit: degree (corresponding to waveform phase offset of 0-2π)
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

protected:

  /**
   * @fn outputStandardWave
   * @brief OUT  The pin outputs standard sine or triangle wave
   * @param type - eMODE_SIN: output sine wave; eMODE_TRI: output triangle save. 
   * @param phase - adjust phase offset, range: 0-360, unit: degree (corresponding to waveform phase offset of 0-2π)
   * @param freq - standard wave frequency, range: 0.06-8000000.00, unit: Hz
   * @return None
   * @note Recommended frequency range: 60-600000Hz, actual resolution is about 0.06Hz, which is obtained by dividing 16MHz by 2^28
   */
  void outputStandardWave(eControlRegBit_t type, uint16_t phase, float freq);

  /**
   * @fn selectRegister
   * @brief Select the register to be enabled. There are two frequency registers and two phase registers without interfering each other, and you are free to choose to enable them.
   * @param phaseReg - ePSEL_0: enable PHASE0 register; ePSEL_1: enable PHASE1 register.
   * @param freqReg - eFSEL_0: enable FREQ0 register; eFSEL_1: enable FREQ1 register.
   * @return None
   */
  void selectRegister(eControlRegBit_t phaseReg, eControlRegBit_t freqReg);

/************************** Register read/write port ******************************/

  /**
   * @fn setParam
   * @brief Set control register, frequency register and phase register
   * @param _writeControl - whether the control register must be modified
   * @param phase - adjust phase offset, range: 0-360, unit: degree (corresponding to waveform phase offset of 0-2π)
   * @param freq - wave frequency, range: 0.06-8000000.00, unit: Hz
   * @return None
   * @note Recommended frequency range: 60-600000Hz, actual resolution is about 0.06Hz, which is obtained by dividing 16MHz by 2^28
   */
  void setParam(bool _writeControl, uint16_t phase, float freq);

  /**
   * @fn writeReg
   * @brief Write register function, design it as a pure virtual function, implement the function body through a derived class
   * @param reg - Register address 16bits
   * @param pBuf - Storage and buffer for data to be written
   * @param size - Length of data to be written
   * @return None
   */
  void writeReg(uint16_t reg, const void * pBuf, size_t size=1);

private:
  // Private variables
  sControl_t AD9837Control;   // AD9837 Control Register Data
  uint16_t _phase;   // Phase Register Data
  uint16_t _freq[2];   // Frequency Register Data

  SPIClass * _pSpi;   // Pointer to SPI communication method
  uint8_t _csPin;   // Cs pin of SPI communication
  SPISettings  _SpiSettings;   // Settings of SPI communication
};

#endif
