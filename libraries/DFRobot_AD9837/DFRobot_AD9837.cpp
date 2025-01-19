/*!
 * @file DFRobot_AD9837.cpp
 * @brief  Define the infrastructure DFRobot_AD9837 class
 * @details The functions declared in the header file are implemented one by one.
 * @copyright Copyright (c) 2010 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @license The MIT License (MIT)
 * @author [qsjhyy](yihuan.huang@dfrobot.com)
 * @version  V1.0
 * @date  2022-03-04
 * @url https://github.com/DFRobot/DFRobot_AD9837
 */
#include "DFRobot_AD9837.h"

DFRobot_AD9837::DFRobot_AD9837(int8_t fscPin, SPIClass *pSpi)
{
  // Save the parameter required for SPI communication
  _pSpi = pSpi;
  _csPin = fscPin;
}

void DFRobot_AD9837::begin(uint32_t frequency)
{
  // 
  pinMode(_csPin, OUTPUT);
  digitalWrite(_csPin, HIGH);

  // SPI communication mode setting
  _SpiSettings = SPISettings(frequency, MSBFIRST, SPI_MODE2);

  // SPI communication init
  _pSpi->begin();

  // Control register init
  memset(&AD9837Control, 0, sizeof(AD9837Control));
  AD9837Control.B28 = eB28_1;

  moduleReset(eReset_EN);   // enter sleep mode

  // Initialize the two frequency registers to 0xFFFFFF, corresponding to a waveform frequency of 1 MHz
  _freq[0] = 0x3FFF;
  _freq[1] = 0x03FF;
  writeReg(AD9837_FREQ0_REG, _freq, 2);
  writeReg(AD9837_FREQ1_REG, _freq, 2);

  // Initialize the two phase registers to 0x0, corresponding to a waveform phase offset of 0
  _phase = 0x0;
  writeReg(AD9837_PHASE0_REG, &_phase);
  writeReg(AD9837_PHASE1_REG, &_phase);

  moduleReset(eReset_DIS);   // quit sleep mode
}

/***************** Config function ******************************/

void DFRobot_AD9837::moduleReset(eControlRegBit_t mode)
{
  if ( AD9837Control.reset != mode ) {
    AD9837Control.reset = mode;
    writeReg(AD9837_CONTROL_REG, &AD9837Control);
  }
}

void DFRobot_AD9837::moduleSleep(eControlRegBit_t MCLK, eControlRegBit_t dacMode)
{
  if ( (AD9837Control.sleep1 != MCLK) || (AD9837Control.sleep2 != dacMode) ) {
    AD9837Control.sleep1 = MCLK;
    AD9837Control.sleep2 = dacMode;
    writeReg(AD9837_CONTROL_REG, &AD9837Control);
  }
}

/***************** Performance function *************************/

void DFRobot_AD9837::outputSin(uint16_t phase, float freq)
{
  DBG("");
  // Indicate whether the control register needs to be modified
  bool writeControl = false;
  if ((eOPBITEN_DAC != AD9837Control.OPBITEN) || (eMODE_SIN != AD9837Control.mode)) {
    AD9837Control.OPBITEN = eOPBITEN_DAC;
    AD9837Control.mode = eMODE_SIN;
    writeControl = true;
  }
  AD9837Control.sleep1 = eSleepMCLK_NO;   // prevent the internal clock from being off
  AD9837Control.sleep2 = eSleepDAC_NO;   // prevent the DAC from being off

  setParam(writeControl, phase, freq);
}

void DFRobot_AD9837::outputTriangle(uint16_t phase, float freq)
{
  DBG("");
  // Indicate whether the control register needs to be modified
  bool writeControl = false;
  if ((eOPBITEN_DAC != AD9837Control.OPBITEN) || (eMODE_TRI != AD9837Control.mode)) {
    AD9837Control.OPBITEN = eOPBITEN_DAC;
    AD9837Control.mode = eMODE_TRI;
    writeControl = true;
  }
  AD9837Control.sleep1 = eSleepMCLK_NO;   // prevent the internal clock from being off
  AD9837Control.sleep2 = eSleepDAC_NO;   // prevent the DAC from being off

  setParam(writeControl, phase, freq);
}

void DFRobot_AD9837::outputSquare(eControlRegBit_t divide, uint16_t phase, float freq)
{
  DBG("");
  // Indicate whether the control register needs to be modified
  bool writeControl = false;
  if ((eOPBITEN_MSB != AD9837Control.OPBITEN) || (eMODE_SIN != AD9837Control.mode) \
     || (eSleepDAC_YES != AD9837Control.sleep2) || (AD9837Control.DIV2 != divide)) {
    // AD9837Control.sleep1 = eSleepMCLK_YES;
    AD9837Control.sleep2 = eSleepDAC_YES;   // The DAC is not required when the AD9837 is only used to output the MSB of DAC data.
    AD9837Control.OPBITEN = eOPBITEN_MSB;
    AD9837Control.mode = eMODE_SIN;   // The mode bit must be reset in this mode.
    AD9837Control.DIV2 = divide;
    writeControl = true;
  }

  setParam(writeControl, phase, freq);
}

void DFRobot_AD9837::outputStandardWave(eControlRegBit_t type, uint16_t phase, float freq)
{
  bool writeControl = false;
  if ((eOPBITEN_DAC != AD9837Control.OPBITEN) || (AD9837Control.mode != type)) {
    AD9837Control.sleep1 = eSleepMCLK_NO;   // prevent the internal clock from being off
    AD9837Control.sleep2 = eSleepDAC_NO;   // prevent the DAC from being off
    AD9837Control.OPBITEN = eOPBITEN_DAC;
    AD9837Control.mode = type;
    writeControl = true;
  }

  setParam(writeControl, phase, freq);
}

void DFRobot_AD9837::selectRegister(eControlRegBit_t phaseReg, eControlRegBit_t freqReg)
{
  if ((AD9837Control.PSEL != phaseReg) || (AD9837Control.FSEL != freqReg)) {
    AD9837Control.PSEL = phaseReg;   // enable the selected register
    AD9837Control.FSEL = freqReg;
    writeReg(AD9837_CONTROL_REG, &AD9837Control);
  }
}

/***************** SPI write register function ******************************/

void DFRobot_AD9837::setParam(bool _writeControl, uint16_t phase, float freq)
{
  // Change phase register
  phase = (uint16_t)(phase / 360.00 * 4095);
  DBG("phase = ", phase);
  uint16_t temp = (phase & 0xFFF);
  if (_phase != temp) {
    _phase = temp;
    writeReg((AD9837_PHASE0_REG | (uint16_t)(AD9837Control.PSEL << 13)), &_phase);
  }

  // Change frequency register
  uint32_t data = (uint32_t)(freq / 16000000.00 * 268435455);   // 2^28 = 268435455, MCLK frequency is 16MHz
  uint16_t buf[] = {(uint16_t)(data & 0x3FFF), (uint16_t)((data >> 14) & 0x3FFF)};
  if ( (_freq[0] != buf[0]) && (_freq[1] != buf[1]) ) {   // Both the MSB bit and LSB bit of the frequency register need to be changed.
    if (_writeControl || (eB28_1 != AD9837Control.B28)) {
      AD9837Control.B28 = eB28_1;
      writeReg(AD9837_CONTROL_REG, &AD9837Control);
      _writeControl = false;
    }
    _freq[0] = buf[0];
    _freq[1] = buf[1];
    writeReg((AD9837_FREQ0_REG << AD9837Control.FSEL), _freq, 2);

  } else if((_freq[0] != buf[0])) {   // only change the LSB bit of the frequency register
    if (_writeControl || (eB28_0 != AD9837Control.B28) || (eHLB_L != AD9837Control.HLB)) {
      AD9837Control.B28 = eB28_0;
      AD9837Control.HLB = eHLB_L;
      writeReg(AD9837_CONTROL_REG, &AD9837Control);
      // _writeControl = false;
    }
    _freq[0] = buf[0];
    writeReg((AD9837_FREQ0_REG << AD9837Control.FSEL), _freq);

  } else if((_freq[1] != buf[1])) {   // only change the MSB bit of the frequency register
    if (_writeControl || (eB28_0 != AD9837Control.B28) || (eHLB_M != AD9837Control.HLB)) {
      AD9837Control.B28 = eB28_0;
      AD9837Control.HLB = eHLB_M;
      writeReg(AD9837_CONTROL_REG, &AD9837Control);
      // _writeControl = false;
    }
    _freq[1] = buf[1];
    writeReg((AD9837_FREQ0_REG << AD9837Control.FSEL), &_freq[1]);

  } else if(_writeControl) {   // The control register needs to be changed.
    writeReg(AD9837_CONTROL_REG, &AD9837Control);

  }
}

void DFRobot_AD9837::writeReg(uint16_t reg, const void * pBuf, size_t size)
{
  if (NULL == pBuf) {
    DBG("pBuf ERROR!! : null pointer");
  }
  uint16_t * _pBuf = (uint16_t *)pBuf;

  _pSpi->beginTransaction(_SpiSettings);
  digitalWrite(_csPin, LOW);

  while(size--) {
    DBG("*_pBuf = ", (*_pBuf | reg), HEX);
    _pSpi->transfer16(*_pBuf | reg);
    _pBuf++;
  }

  digitalWrite(_csPin, HIGH);
  _pSpi->endTransaction();
}
