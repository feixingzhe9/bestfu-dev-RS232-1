/*****************************************************************************
    文件： RS232Unit.h
    说明： 串口单元处理逻辑函数头文件
    编译： Keil uVision4 V4.54.0.0
    版本： v2.0
    编写： Unarty
    日期： 2014.08.19
*****************************************************************************/
#ifndef __RS232UNIT_H
#define __RS232UNIT_H

#include "BF_type.h"
#include "USARTx.h"

#define UART_CFG_ADDR       (0x4000)    //串口配置存储地址
#define UART_FLAG_ADDR      (0x4080)    //串口指令标识地址

#define UART_CMD_ADDR       (0x4100)    //串口发送指令存储地址

#define UART_CMD_SIZE       (0x100)     //串口指令长度


extern u8 gSendID;

void RS232Unit_Init(void);

void RS232_CmdSend(void);

void RS232_CmdFlagSet(u8 id, u8 val);
void RS232_CmdFlagClr(void);
u8   RS232_CmdFlagGet(u8);

void RS232_CmdAdd(u8 id, u8 *data);
u8   RS232_CmdGet(u8 id, u8 *data);

void RS232_CfgGet(UARTCfg_t *cfg);
void RS232_CfgSave(UARTCfg_t *cfg);
u8   RS232_CfgCheck(UARTCfg_t *cfg);
void RS232_CfgDefault(UARTCfg_t *cfg);

#endif  //RS232Unit.h end
/**************************Copyright BestFu 2014-05-14*************************/
