/***************************Copyright BestFu 2016-07-02*************************
文 件：  led.h
说 明：  led指示头文件
编 译：  uVision V5.20.0
版 本：  v1.0
编 写：  jay
日 期：  2016/07/02
修　改：暂无
*******************************************************************************/
#ifndef __LED_H
#define __LED_H

#include "BF_type.h"

/**************************************宏定义**********************************/
#define GREEN_PORT	GPIO_B
#define GREEN_PIN	pin_9

#define RED_PORT	GPIO_B
#define RED_PIN		pin_8

/*********************************函数声明************************************/
void LED_Init(void);
void UnitInstructLED(void);
void GreenOn(void);
void RvrsGreen(void);
void RvrsRed(void);
void RedOn(void);
void RedOff(void);
void GreenOn(void);
void GreenOff(void);

#endif  
/**************************Copyright BestFu 2016-07-02*************************/
