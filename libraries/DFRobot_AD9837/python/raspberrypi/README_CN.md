# DFRobot_AD9837
* [English Version](./README.md)

  AD9837是一款低功耗、可编程波形发生器，能够产生正弦
波、三角波和方波输出。各种类型的检测、信号激励和时
域反射(TDR)应用都需要波形发生器。输出频率和相位可
通过软件进行编程，调整简单。频率寄存器为28位：时钟
速率为16 MHz时，可以实现0.06 Hz的分辨率；而时钟速率
为5 MHz时，则可以实现0.02 Hz的分辨率。
  AD9837通过一个3线串行接口写入数据。该串行接口能够
以最高40 MHz的时钟速率工作，并且与DSP和微控制器标
准兼容。该器件采用2.3 V至5.5 V电源供电。
  AD9837具有关断(休眠)功能。器件中不用的部分可以关
断，以将功耗降至最低。例如，在产生时钟输出时，可以
关断DAC。

![产品实物图](../../resources/images/AD9837.png)


## 产品链接 (https://www.dfrobot.com.cn/)
    SKU: DFR0973


## 目录

* [概述](#概述)
* [库安装](#库安装)
* [方法](#方法)
* [兼容性](#兼容性)
* [历史](#历史)
* [创作者](#创作者)


## 概述

* 数字可编程频率和相位
* 功耗：8.5 mW(2.3 V时)
* MCLK速度：16 MHz(B级)，5 MHz(A级)
* 28位分辨率：0.06 Hz(16 MHz参考时钟)
* 正弦波/三角波/方波输出
* 2.3 V至5.5 V电源供电
* 3线SPI接口
* 扩展温度范围：−40°C至+125°C
* 省电选项


## 库安装

要使用库, 首先下载库文件, 将其粘贴到指定的目录中, 然后打开examples文件夹并在该文件夹中运行演示。


## 方法

```python

  '''!
    @brief Initialize sensor
  '''
  def begin(self):

  '''!
    @brief 可将内部寄存器复位至0, 对应于中间电平的模拟输出。
    @param mode - e_reset_EN: 内置寄存器复位; e_reset_DIS: 退出复位状态
    @note 复位操作不会使相位、频率或控制寄存器复位。
  '''
  def module_reset(self, mode):

  '''!
    @brief 可使用休眠功能关断AD9837中不使用的部分, 以将功耗降至最低。可关断的芯片部分是内部时钟和DAC。
    @param MCLK - e_sleep_MCLK_NO: 使能内部MCLK; e_sleep_MCLK_YES: 禁用内部MCLK. 由于NCO不再执行累加, 因此DAC输出仍保持其当前值.
    @param DAC - e_sleep_DAC_NO: 使DAC处于活动状态; e_sleep_DAC_YES: 关断片内DAC. 当AD9837用于输出DAC数据的MSB时, 这点很有用.
    @note 复位操作不会使相位、频率或控制寄存器复位。
  '''
  def module_sleep(self, MCLK, DAC):

  '''!
    @brief OUT 引脚输出正弦波
    @param phase - 调节相位偏移, 范围: 0~360, 单位: 度 (对应0~2π的波形相位偏移)
    @param freq - 波的频率, 范围: 0.06 ~ 8000000.00, 单位: Hz
    @note 推荐频率范围: 60 ~ 600000Hz, 实际分辨率为16MHz除以2^28, 约为0.06Hz
  '''
  def output_sine_wave(self, phase, freq):

  '''!
    @brief OUT 引脚输出三角波
    @param phase - 调节相位偏移, 范围: 0~360, 单位: 度 (对应0~2π的波形相位偏移)
    @param freq - 波的频率, 范围: 0.06 ~ 8000000.00, 单位: Hz
    @note 推荐频率范围: 60 ~ 600000Hz, 实际分辨率为16MHz除以2^28, 约为0.06Hz
  '''
  def output_triangle_wave(self, phase, freq):

  '''!
    @brief OUT 引脚输出方波
    @param divide - e_DIV2_2: DAC数据的MSB/2通过VOUT引脚输出; e_DIV2_1: DAC数据的MSB通过VOUT引脚输出.
    @param phase - 调节相位偏移, 范围: 0~360, 单位: 度 (对应0~2π的波形相位偏移)
    @param freq - 波的频率, 范围: 0.06 ~ 8000000.00, 单位: Hz
    @note 推荐频率范围: 60 ~ 600000Hz, 实际分辨率为16MHz除以2^28, 约为0.06Hz
  '''
  def output_square_wave(self, divide, phase, freq):

```


## 兼容性

* RaspberryPi 版本

| Board        | Work Well | Work Wrong | Untested | Remarks |
| ------------ | :-------: | :--------: | :------: | ------- |
| RaspberryPi2 |           |            |    √     |         |
| RaspberryPi3 |           |            |    √     |         |
| RaspberryPi4 |     √     |            |          |         |

* Python 版本

| Python  | Work Well | Work Wrong | Untested | Remarks |
| ------- | :-------: | :--------: | :------: | ------- |
| Python2 |     √     |            |          |         |
| Python3 |     √     |            |          |         |


## 历史

- 2022/03/14 - 1.0.0 版本


## 创作者

Written by qsjhyy(yihuan.huang@dfrobot.com), 2022. (Welcome to our [website](https://www.dfrobot.com/))

