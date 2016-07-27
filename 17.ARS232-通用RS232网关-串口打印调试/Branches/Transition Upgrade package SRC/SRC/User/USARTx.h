/***************************Copyright BestFu 2014-05-14*************************
文 件：  USARTx.h
说 明：  串口寄存器配置头文件
编 译：  Keil uVision4 V4.54.0.0
版 本：  v2.0
编 写：  Unarty
日 期：  2014.08.20
修　改：    暂无
*******************************************************************************/
#ifndef __USARTx_H
#define __USARTx_H

#include "BF_type.h"

#define USART       (2)     //串口编号

#if     (USART == 1)
#define UART        USART1      //串口寄存器
#define UART_CLK    (1<<14)     //串口时钟

#define UART_PORT       GPIO_A  //串口端口
#define UART_TX_PIN     pin_9   //串口发送管脚
#define UART_RX_PIN     pin_10  //串口接收管脚

#elif   (USART == 2)
#define UART        USART2      //串口寄存器
#define UART_CLK    (1<<17)     //串口时钟

#define UART_PORT       GPIO_A  //串口端口
#define UART_TX_PIN     pin_2   //串口发送管脚
#define UART_RX_PIN     pin_3   //串口接收管脚

#endif
#define QUERY_WAIT_TIME	40
typedef enum
{
	Bound_1200 =1,
	Bound_2400,
	Bound_4800,
	Bound_9600,
	Bound_14400,
	Bound_19200,
	Bound_38400,
	Bound_43000,
	Bound_56000,
	Bound_115200,
	Bound_128000,
	Bound_256000,	
	Bound_Maximum
}Bound_e;
typedef struct
{
    u8  bound;
    u8  check;
    u8  dataBit;
    u8  stopBit;
} UARTCfg_t;

#define UART_RECIEVE_TIME_OUT   1       //单位：10ms

typedef struct
{
    u8 Cnt;
		u8 Len;
    u32 timeOutCnt;
    u8 timeOutFlag;
    u8 data[200];
    u8 dataRecieveFlag;
} UART_Rec_t;

typedef struct
{
	u8 len;
	u8 cnt;
	u8 data[200];
	u8 flag;
} Query_Recieve_t;

extern UART_Rec_t UART_Recieve;
extern Query_Recieve_t QueryRecieve;
void CheckQueryRecieve(void);

void USARTx_Init(u32 pclk2, UARTCfg_t *uartCfg);
u8 USARTx_Send(u8 *data, u16 len);
u8 USARTx_Write(u8 c);
extern void UART_RecTimeOutCnt(void);
extern void Clr_UART_RecTimeOut(void);
extern void Clr_QueryRcvData(void);

#endif  //USART_x end 
/**************************Copyright BestFu 2014-05-14*************************/
