# -*- coding: utf-8 -*
'''!
  @file  output_sin.py
  @brief  Drive the AD9837 via SPI to output a sine wave signal
  @details  Configure the communication pin to be used and properly connect the AD9837 to the main controller
  @n  Reset the AD9837 or power down the internal clock and DAC of AD9837
  @n  Set frequency and phase offset of the sine wave signal
  @note  The amplitude of the sine wave signal is about 0.645V; the wave phase offset must be observed with reference; VCC to 3.3V
  @copyright  Copyright (c) 2010 DFRobot Co.Ltd (http://www.dfrobot.com)
  @license  The MIT License (MIT)
  @author  [qsjhyy](yihuan.huang@dfrobot.com)
  @version  V1.0
  @date  2022-03-28
  @url  https://github.com/DFRobot/DFRobot_AD9837
'''
from __future__ import print_function
import sys
import os
sys.path.append(os.path.dirname(os.path.dirname(os.path.realpath(__file__))))   # including library file path

from DFRobot_AD9837 import *

'''
  # @brief Module init
  # @param cs - cs chip select pin
  # @param bus - SPI bus 
  # @param dev - SPI device number
  # @param speed - SPI communication frequency
'''
AD9837 = DFRobot_AD9837(cs=8, bus=0, dev=0, speed=10000000)


def setup():
  AD9837.begin()
  print("sensor begin successfully!!!")

  '''
    # @brief Reset internal register to 0, this corresponds to an analog output of midscale.
    # @param mode - e_reset_EN: reset internal register; e_reset_DIS: quit reset status
    # @note A reset will not reset the phase, frequency, or control register.
  '''
  AD9837.module_reset(mode=AD9837.e_reset_DIS)

  '''
    # @brief The parts of AD9837 that are not in use like the internal clock and DAC can be powered down through the sleep function to minimize power consumption.  
    # @param MCLK - e_sleep_MCLK_NO: enable internal MCLK; e_sleep_MCLK_YES: disable internal MCLK. The DAC output remains at its current value because NCO is no longer accumulating.
    # @param DAC - e_sleep_DAC_NO: keep DAC active; e_sleep_DAC_YES: power down on-chip DAC. This is useful when AD9837 is used to output MSB of DAC data.
    # @note RESET operation will not reset the phase, frequency, or control register.
  '''
  AD9837.module_sleep(MCLK=AD9837.e_sleep_MCLK_NO, DAC=AD9837.e_sleep_DAC_NO)

  '''
    # @brief OUT  The pin outputs sine wave
    # @param phase - adjust phase offset, range: 0-360, unit: degree (corresponding to waveform phase offset of 0-2π)
    # @param freq - wave frequency, range: 0.06-8000000.00, unit: Hz
    # @note Recommended frequency range: 60-600000Hz, actual resolution is about 0.06Hz, which is obtained by dividing 16MHz by 2^28 
  '''
  AD9837.output_sine_wave(phase=0, freq=1000.0)


def loop():
  time.sleep(0.5)


if __name__ == "__main__":
  setup()
  while True:
    loop()
