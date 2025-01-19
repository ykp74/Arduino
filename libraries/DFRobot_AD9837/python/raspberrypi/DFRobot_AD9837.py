# -*- coding: utf-8 -*
'''!
  @file  DFRobot_AD9837.py
  @brief  Define infrastructure of DFRobot_AD9837 class
  @details  Define and implement related function interfaces for setting waveform, frequency, phase, SPI communication, etc.
  @copyright  Copyright (c) 2010 DFRobot Co.Ltd (http://www.dfrobot.com)
  @license  The MIT License (MIT)
  @author  [qsjhyy](yihuan.huang@dfrobot.com)
  @version  V1.0
  @date  2022-03-11
  @url  https://github.com/DFRobot/DFRobot_AD9837
'''
import sys
import time

import spidev
import RPi.GPIO as GPIO

import logging
from ctypes import *


logger = logging.getLogger()
# logger.setLevel(logging.INFO)   # Display all print information
logger.setLevel(logging.FATAL)   # If you don’t want to display too many prints, only print errors, please use this option
ph = logging.StreamHandler()
formatter = logging.Formatter("%(asctime)s - [%(filename)s %(funcName)s]:%(lineno)d - %(levelname)s: %(message)s")
ph.setFormatter(formatter) 
logger.addHandler(ph)


# AD9837 Register address
## Control register, which is 16 bits
AD9837_CONTROL_REG = 0x0000
## Frequency register 0, which is 28 bits
AD9837_FREQ0_REG = 0x4000
## Frequency register 1, which is 28 bits
AD9837_FREQ1_REG = 0x8000
## Phase register 0, which is 12 bits, so the b12 value can be set as per your need, the address can be 0xD000
AD9837_PHASE0_REG = 0xC000
## Phase register 1, which is 12 bits, so the b12 value can be set as per your need, the address can be 0xF000
AD9837_PHASE1_REG = 0xE000


class DFRobot_AD9837(object):
  '''!
    @brief Define DFRobot_AD9837 basic class
    @details Drive The Programmable Waveform Generator
  '''

  e_B28_0 = 0<<13   ## B28 = 0, the 28-bit frequency register operates as two 14-bit registers, respectively containing 14 MSBs and 14 LSBs.
  e_B28_1 = 1<<13   ## B28 = 1, allows a complete word to be loaded into a frequency register in two consecutive writes.

  e_HLB_L = 0<<12   ## HLB = 0 allows a write to the 14 LSBs of the addressed frequency register.
  e_HLB_M = 1<<12   ## HLB = 1 allows a write to the 14 MSBs of the addressed frequency register. 

  e_FSEL_0 = 0<<11   ## When bit FSEL = 0, the FREQ0 register defines the output frequency as a fraction of MCLK frequency
  e_FSEL_1 = 1<<11   ## When bit FSEL = 1, the FREQ1 register defines the output frequency as a fraction of MCLK frequency

  e_PSEL_0 = 0<<10   ## When bit PSEL = 0, the content of PHASE0 register is added to the output of the phase accumulator
  e_PSEL_1 = 1<<10   ## When bit PSEL = 1, the content of PHASE1 register is added to the output of the phase accumulator

  e_reset_DIS = 0<<8   ## RESET = 0 disables the reset function
  e_reset_EN = 1<<8   ## RESET = 1 resets internal registers to 0, which corresponds to an analog output of midscale

  e_sleep_MCLK_NO = 0<<7   ## SLEEP1 = 0 enables the internal MCLK
  e_sleep_MCLK_YES = 1<<7   ## SLEEP1 = 1 disables the internal MCLK. The DAC output remains at its current value because NCO is no longer accumulating.

  e_sleep_DAC_NO = 0<<6   ## SLEEP2 = 0 indicates that the DAC is active
  e_sleep_DAC_YES = 1<<6   ## SLEEP2 = 1 powers down the on-chip DAC. This is useful when the AD9837 is used to output the MSB of the DAC data.

  e_OPBITEN_DAC = 0<<5   ## OPBITEN = 0 connects the DAC output to VOUT.
  e_OPBITEN_MSB = 1<<5   ## When OPBITEN = 1, the VOUT pin does not provide the DAC output. Instead, the MSB (or MSB/2) of the DAC data is connected to the VOUT pin.

  e_DIV2_2 = 0<<3   ## DIV2 = 0, the MSB/2 of the DAC data output through the pin VOUT.
  e_DIV2_1 = 1<<3   ## DIV2 = 1, the MSB of the DAC data output through the pin VOUT.

  e_mode_SIN = 0<<1   ## MODE = 0, the SIN ROM converts the phase information into amplitude information, resulting in a sinusoidal signal at the output.
  e_mode_TRI = 1<<1   ## MODE = 1, bypasses the SIN ROM, resulting in a triangle save output from the DAC.

  def __init__(self, cs=8, bus=0, dev=0, speed=10000000):
    '''!
      @brief Module init
      @param cs - cs chip select pin
      @param bus - SPI bus 
      @param dev - SPI device number
      @param speed - SPI communication frequency
    '''
    self._cs = cs
    GPIO.setmode(GPIO.BCM)
    GPIO.setwarnings(False)
    GPIO.setup(self._cs, GPIO.OUT, initial=1)
    GPIO.output(self._cs, GPIO.LOW)

    self._spi = spidev.SpiDev()
    self._spi.open(bus, dev)
    self._spi.mode = 2
    self._spi.no_cs = True
    self._spi.max_speed_hz = speed

    self._AD9837_control = 0x0000
    self._freq = 0xFFFFFF   # Initialize the two frequency registers to 0xFFFFFF, corresponding to a waveform frequency of 1 MHz. Note: [0x3FFF, 0x03FF]
    self._phase = 0x0   # Initialize the two phase registers to 0xFFF

  def begin(self):
    '''!
      @brief Initialize sensor
    '''
    self._AD9837_control |= self.e_B28_1   # Set as the frequency register whose 28-bit can be modified continuously
    self.module_reset(self.e_reset_EN)   # Enter sleep mode
    self._write_reg(AD9837_FREQ0_REG, (self._freq & 0x3FFF))   # Initialize FREQ0 register
    self._write_reg(AD9837_FREQ0_REG, ((self._freq >> 14) & 0x3FFF))
    self._write_reg(AD9837_FREQ1_REG, (self._freq & 0x3FFF))   # Initialize FREQ1 register
    self._write_reg(AD9837_FREQ1_REG, ((self._freq >> 14) & 0x3FFF))
    self._write_reg(AD9837_PHASE0_REG, self._phase)   # Initialize PHASE0 register
    self._write_reg(AD9837_PHASE1_REG, self._phase)   # Initialize PHASE1 register
    self.module_reset(self.e_reset_DIS)   # Quit sleep mode
    time.sleep(0.5)

  def module_reset(self, mode):
    '''!
      @brief Reset internal register to 0, corresponding to an analog output of midscale.
      @param mode - e_reset_EN: reset internal register; e_reset_DIS: quit reset status
      @note Reset operation will not reset the phase, frequency, or control register.
    '''
    if( (self._AD9837_control & self.e_reset_EN) != mode ):
      self._AD9837_control &= ~self.e_reset_EN
      self._AD9837_control |= (mode & self.e_reset_EN)
      self._write_reg(AD9837_CONTROL_REG, self._AD9837_control)

  def module_sleep(self, MCLK, DAC):
    '''!
      @brief The parts of AD9837 that are not in use like the internal clock and DAC can be powered down through the sleep function to minimize power consumption. 
      @param MCLK - e_sleep_MCLK_NO: enable internal MCLK; e_sleep_MCLK_YES: disable internal MCLK. The DAC output remains at its current value because NCO is no longer accumulating.
      @param DAC - e_sleep_DAC_NO: keep DAC active; e_sleep_DAC_YES: power down on-chip DAC. This is useful when AD9837 is used to output MSB of DAC data.
      @note A reset will not reset the phase, frequency, or control register.
    '''
    if ( (self._AD9837_control & self.e_sleep_MCLK_YES) != MCLK ) or \
       ( (self._AD9837_control & self.e_sleep_DAC_YES) != DAC ) :
      self._AD9837_control &= ~self.e_sleep_MCLK_YES
      self._AD9837_control |= (MCLK & self.e_sleep_MCLK_YES)
      self._AD9837_control &= ~self.e_sleep_DAC_YES
      self._AD9837_control |= (DAC & self.e_sleep_DAC_YES)
      self._write_reg(AD9837_CONTROL_REG, self._AD9837_control)

  def output_sine_wave(self, phase, freq):
    '''!
      @brief OUT  The pin outputs sine wave
      @param phase - adjust phase offset, range: 0-360, unit: degree (corresponding to waveform phase offset of 0-2π)
      @param freq - wave frequency, range: 0.06-8000000.00, unit: Hz
      @note Recommended frequency range: 60-600000Hz, actual resolution is about 0.06Hz, which is obtained by dividing 16MHz by 2^28 
    '''
    if ( (self._AD9837_control & self.e_OPBITEN_MSB) != self.e_OPBITEN_DAC ) or \
       ( (self._AD9837_control & self.e_mode_TRI) != self.e_mode_SIN ) :
      self._AD9837_control |= self.e_B28_1   # Set modifying the 28-bit frequency register continuously
      self._AD9837_control &= ~self.e_OPBITEN_MSB
      self._AD9837_control &= ~self.e_mode_TRI
      self._AD9837_control &= ~self.e_sleep_MCLK_YES   # Prevent internal clock or DAC from being off
      self._AD9837_control &= ~self.e_sleep_DAC_YES
      self._write_reg(AD9837_CONTROL_REG, self._AD9837_control)

    self._set_param(phase, freq)

  def output_triangle_wave(self, phase, freq):
    '''!
      @brief OUT the pin output triangle wave
      @param phase - adjust phase offset, range: 0-360, unit: degree (corresponding to waveform phase offset of 0-2π)
      @param freq - wave frequency, range: 0.06-8000000.00, unit: Hz
      @note Recommended frequency range: 60-600000Hz, actual resolution is about 0.06Hz, which is obtained by dividing 2^28 into 16MHz
    '''
    if ( (self._AD9837_control & self.e_OPBITEN_MSB) != self.e_OPBITEN_DAC ) or \
       ( (self._AD9837_control & self.e_mode_TRI) != self.e_mode_TRI ) :
      self._AD9837_control |= self.e_B28_1   # Set as the frequency register whose 28-bit can be modified continuously
      self._AD9837_control &= ~self.e_OPBITEN_MSB
      self._AD9837_control |= self.e_mode_TRI
      self._AD9837_control &= ~self.e_sleep_MCLK_YES   # Prevent internal clock or DAC from being off
      self._AD9837_control &= ~self.e_sleep_DAC_YES
      self._write_reg(AD9837_CONTROL_REG, self._AD9837_control)

    self._set_param(phase, freq)

  def output_square_wave(self, divide, phase, freq):
    '''!
      @brief OUT  The pin outputs square wave
      @param divide - e_DIV2_2: MSB/2 of DAC data is output through VOUT pin; e_DIV2_1: MSB of DAC data is output via VOUT pin.
      @param phase - adjust phase offset, range: 0-360, unit: degree (corresponding to waveform phase offset of 0-2π)
      @param freq - wave frequency, range: 0.06-8000000.00, unit: Hz
      @note Recommended frequency range: 60-600000Hz, actual resolution is about 0.06Hz, which is obtained by dividing 16MHz by 2^28 
    '''
    if ( (self._AD9837_control & self.e_OPBITEN_MSB) != self.e_OPBITEN_MSB ) or \
       ( (self._AD9837_control & self.e_DIV2_1) != divide) or \
       ( (self._AD9837_control & self.e_sleep_DAC_YES) != self.e_sleep_DAC_YES) or \
       ( (self._AD9837_control & self.e_mode_TRI) != self.e_mode_SIN ) :
      self._AD9837_control |= self.e_B28_1   # Set as the frequency register whose 28-bit can be modified continuously
      self._AD9837_control |= self.e_OPBITEN_MSB
      self._AD9837_control &= ~self.e_mode_TRI   # The mode bit must be reset in this mode.
      self._AD9837_control |= self.e_sleep_DAC_YES   # The DAC is not required when the AD9837 is only used to output the MSB of DAC data.
      self._AD9837_control &= ~self.e_DIV2_1
      self._AD9837_control |= (divide & self.e_DIV2_1)
      self._write_reg(AD9837_CONTROL_REG, self._AD9837_control)

    self._set_param(phase, freq)

  def _set_param(self, _phase, _freq):
    '''!
      @brief Set frequency register and phase register
      @param _phase - adjust phase offset, range: 0-360, unit: degree (corresponding to waveform phase offset of 0-2π)
      @param _freq - wave frequency, range: 0.06-8000000.00, unit: Hz
      @note Recommended frequency range: 60-600000Hz, actual resolution is about 0.06Hz, which is obtained by dividing 16MHz by 2^28
    '''
    # Modify phase register
    _phase = int(_phase / 360.00 * 4095)
    temp = (_phase & 0xFFF)
    if( self._phase != temp ):
      self._phase = temp
      phase_reg = AD9837_PHASE0_REG | ((self._AD9837_control & self.e_PSEL_1) << 3)
      self._write_reg(phase_reg, self._phase)

    # Modify frequency register
    data = int(_freq / 16000000.00 * 268435455)
    if( self._freq != data ):
      self._freq = data
      if (self._AD9837_control & self.e_FSEL_1) :
        freq_reg = AD9837_FREQ1_REG
      else:
        freq_reg = AD9837_FREQ0_REG
      self._write_reg(freq_reg, (self._freq & 0x3FFF))
      self._write_reg(freq_reg, ((self._freq >> 14) & 0x3FFF))

  def _write_reg(self, reg, data):
    '''!
      @brief writes data to a register
      @param reg register address
      @param data written data
    '''
    data |= reg
    logger.info("%#x" %data)
    GPIO.output(self._cs, GPIO.LOW)
    self._spi.xfer([data >> 8, data & 0xFF])
    GPIO.output(self._cs, GPIO.HIGH)
