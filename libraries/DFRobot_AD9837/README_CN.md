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

![产品实物图](./resources/images/AD9837.png)


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

要使用这个库, 首先下载库文件, 将其粘贴到\Arduino\libraries目录中, 然后打开示例文件夹并在文件夹中运行演示。


## 方法

```C++

/************************ Init ********************************/
  /**
   * @fn DFRobot_AD9837
   * @brief Constructor
   * @param fscPin - 低电平有效控制引脚, 拉低时可进行SPI数据传输
   * @param pSpi - extern SPIClass SPI is defined in SPI.h; so just get SPI object address and the methods in SPI can be pointed to and used
   * @return None
   */
  DFRobot_AD9837(int8_t fscPin=5, SPIClass *pSpi=&SPI);

  /**
   * @fn begin
   * @brief Init function, 包括SPI通信初始化, 模块上电复位
   * @param frequency - SPI通信最大频率
   * @return None
   */
  void begin(uint32_t frequency=20000000);

/************************** Config function ******************************/
  /**
   * @fn moduleReset
   * @brief 可将内部寄存器复位至0，对应于中间电平的模拟输出。
   * @param mode - eReset_EN: 内置寄存器复位; eReset_DIS: 退出复位状态
   * @return None
   * @note 复位操作不会使相位、频率或控制寄存器复位。
   */
  void moduleReset(eControlRegBit_t mode=eReset_DIS);

  /**
   * @fn moduleReset
   * @brief 可使用休眠功能关断AD9837中不使用的部分，以将功耗降至最低。可关断的芯片部分是内部时钟和DAC。
   * @param MCLK - eSleepMCLK_NO: 使能内部MCLK; eSleepMCLK_YES: 禁用内部MCLK. 由于NCO不再执行累加, 因此DAC输出仍保持其当前值.
   * @param dacMode - eSleepDAC_NO: 使DAC处于活动状态; eSleepDAC_YES: 关断片内DAC. 当AD9837用于输出DAC数据的MSB时, 这点很有用.
   * @return None
   * @note 复位操作不会使相位、频率或控制寄存器复位。
   */
  void moduleSleep(eControlRegBit_t MCLK=eSleepMCLK_NO, eControlRegBit_t dacMode=eSleepDAC_NO);

/************************** Performance function ******************************/
  /**
   * @fn outputSin
   * @brief OUT 引脚输出正弦波
   * @param phase - 调节相位偏移, 范围: 0~360, 单位: 度 (对应0~2π的波形相位偏移)
   * @param freq - 波的频率, 范围: 0.06 ~ 8000000.00, 单位: Hz
   * @return None
   * @note 推荐频率范围: 60 ~ 600000Hz, 实际分辨率为16MHz除以2^28, 约为0.06Hz
   */
  void outputSin(uint16_t phase, float freq);

  /**
   * @fn outputTriangle
   * @brief OUT 引脚输出三角波
   * @param phase - 调节相位偏移, 范围: 0~360, 单位: 度 (对应0~2π的波形相位偏移)
   * @param freq - 波的频率, 范围: 0.06 ~ 8000000.00, 单位: Hz
   * @return None
   * @note 推荐频率范围: 60 ~ 600000Hz, 实际分辨率为16MHz除以2^28, 约为0.06Hz
   */
  void outputTriangle(uint16_t phase, float freq);

  /**
   * @fn outputSquare
   * @brief OUT 引脚输出方波
   * @param divide - eDIV2_2: DAC数据的MSB/2通过VOUT引脚输出; eDIV2_1: DAC数据的MSB通过VOUT引脚输出.
   * @param phase - 调节相位偏移, 范围: 0~360, 单位: 度 (对应0~2π的波形相位偏移)
   * @param freq - 波的频率, 范围: 0.06 ~ 8000000.00, 单位: Hz
   * @return None
   * @note 推荐频率范围: 60 ~ 600000Hz, 实际分辨率为16MHz除以2^28, 约为0.06Hz
   */
  void outputSquare(eControlRegBit_t divide, uint16_t phase, float freq);

```


## 兼容性

MCU                | Work Well    | Work Wrong   | Untested    | Remarks
------------------ | :----------: | :----------: | :---------: | :----:
Arduino Uno        |      √       |              |             |
Arduino MEGA2560   |      √       |              |             |
Arduino Leonardo   |      √       |              |             |
FireBeetle-ESP8266 |      √       |              |             |
FireBeetle-ESP32   |      √       |              |             |
FireBeetle-M0      |      √       |              |             |
Micro:bit          |      √       |              |             |


## 历史

- 2022/03/11 - 1.0.0 版本


## 创作者

Written by qsjhyy(yihuan.huang@dfrobot.com), 2022. (Welcome to our [website](https://www.dfrobot.com/))

