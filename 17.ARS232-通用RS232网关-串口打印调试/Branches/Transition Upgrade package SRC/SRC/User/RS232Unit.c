/*****************************************************************************
    文件： RS232Unit.c
    说明： 串口单元处理逻辑函数
    编译： Keil uVision4 V4.54.0.0
    版本： v2.0
    编写： Unarty
    日期： 2014.08.19
*****************************************************************************/
#include "RS232Unit.h"
#include "USARTx.h"
#include "delay.h"
#include "Fifo.h"
#include "Thread.h"
#include "UnitCfg.h"

u8 gSendID;

/*******************************************************************************
函 数 名:    RS232Unit_Init
功能说明:   RS232单元初始化函数
参    数:     无
返 回 值:    无
*******************************************************************************/
void RS232Unit_Init(void)
{
    UARTCfg_t uartCfg;

    RS232_CfgGet(&uartCfg);
    if (FALSE == RS232_CfgCheck(&uartCfg))  //如果配置数据异常
    {
        RS232_CfgDefault(&uartCfg);
    }
    USARTx_Init(36, &uartCfg);
    USARTx_Send("1234567890", 10);

    
}


/*******************************************************************************
函 数 名:    RS232_CmdSend
功能说明:   串口指令发送
参    数:     id:     指令标识号
返 回 值:    无
*******************************************************************************/
void RS232_CmdSend(void)
{
    u8 data[256];

    EEPROM_Read(UART_CMD_ADDR + UART_CMD_SIZE * gSendID,  UART_CMD_SIZE, data);
    if (0 != RS232_CmdGet(gSendID, data))   //没数据
    {
			USARTx_Send(&data[1], data[0] /*+ data[data[0] + 1] + 1*/);//只发送指令，(期望的应答数据不需要发送?)
    }

}
/*******************************************************************************
函 数 名:    RS232_CmdFlagSet
功能说明:   串口指令标识设置
参    数:     id:     指令标识号
            val;    目标值
返 回 值:    无
*******************************************************************************/
void RS232_CmdFlagSet(u8 id, u8 val)
{
    u8 data, i;

    val &= 0x01;
    val <<= id & 0x07;
    i = 1 << (id & 0x07);
    EEPROM_Read(UART_FLAG_ADDR + (id >> 3),  1, &data);
    if ((data & i) != val) //当前标记值不是目标标记值
    {
        if (0 == val)
        {
            data &= ~i;
        }
        else
        {
            data |= i;
        }
        EEPROM_Write(UART_FLAG_ADDR + (id >> 3),  1, &data);
    }
}

/*******************************************************************************
函 数 名:    RS232_CmdFlagClr
功能说明:   串口指令标识清除
参    数:     无
返 回 值:    无
*******************************************************************************/
void RS232_CmdFlagClr(void)
{
    u32 data[4] = {0};

    EEPROM_Write(UART_FLAG_ADDR,  sizeof(data), (u8 *)data);
}

/*******************************************************************************
函 数 名:    RS232_CmdFlagGet
功能说明:   串口指令标识清除
参    数:     id:     指令标识号
返 回 值:    FALSE/TRUE
*******************************************************************************/
u8 RS232_CmdFlagGet(u8 id)
{
    u8 data;

    EEPROM_Read(UART_FLAG_ADDR + (id >> 3),  1, &data);

    return (data & (1 << (id & 0x07)) ? TRUE : FALSE);
}

/*******************************************************************************
函 数 名:    RS232_CmdAdd
功能说明:   串口发送指令添加
参    数:     id:     指令ID
            *data:  data[0]:指令长度，data[1~n]:指令内容
返 回 值:    无
*******************************************************************************/
void RS232_CmdAdd(u8 id, u8 *data)
{
    EEPROM_Write(UART_CMD_ADDR + UART_CMD_SIZE * id,
                 data[0] + data[data[0]+1] + 2, data);
    RS232_CmdFlagSet(id, 1);
}

/*******************************************************************************
函 数 名:    RS232_CmdGet
功能说明:   串口发送指令获取
参    数:     id:     指令ID
            *data:  指令内容
返 回 值:    len
*******************************************************************************/
u8 RS232_CmdGet(u8 id, u8 *data)
{

    if (FALSE == RS232_CmdFlagGet(id))  //如果指令ID不存在
    {
        return 0;
    }
    EEPROM_Read(UART_CMD_ADDR + UART_CMD_SIZE * id, UART_CMD_SIZE, data);
    if (data[0] > 80 || data[data[0] + 1] > 80) //参数长度异常
    {
        RS232_CmdFlagSet(id, 0);    //指令清除
        return 0;
    }

    return (data[0] + data[data[0] + 1]);
}

/*******************************************************************************
函 数 名:    RS232_CfgGet
功能说明:   串口配置数据获取
参    数:     无
返 回 值:    无
*******************************************************************************/
void RS232_CfgGet(UARTCfg_t *cfg)
{
    EEPROM_Read(UART_CFG_ADDR, sizeof(UARTCfg_t), (u8 *)cfg);
}

/*******************************************************************************
函 数 名:    RS232_CfgSave
功能说明:   串口配置存储
参    数:     无
返 回 值:    无
*******************************************************************************/
void RS232_CfgSave(UARTCfg_t *cfg)
{

    EEPROM_Write(UART_CFG_ADDR, sizeof(UARTCfg_t), (u8 *)cfg);
}

/*******************************************************************************
函 数 名:    RS232_CfgDefault
功能说明:   串口配置默认值
参    数:     无
返 回 值:    无
*******************************************************************************/
void RS232_CfgDefault(UARTCfg_t *cfg)
{
    cfg->bound = Bound_9600;
    cfg->check = 0;
    cfg->dataBit = 8;
    cfg->stopBit = 1;
    RS232_CfgSave(cfg);
}

/*******************************************************************************
函 数 名:    RS232_CfgCheck
功能说明:   串口配置数据核对
参    数:     cfg:    目标配置值
返 回 值:    FALSE/TRUE
*******************************************************************************/
u8 RS232_CfgCheck(UARTCfg_t *cfg)
{
//    switch (cfg->bound)
//    {
//		case 1200:
//    case 2400:
//    case 4800:
//    case 9600:
//    case 14400:
//    case 19200:
//    case 38400:
//    case 43000:
//    case 56000:
//    case 115200:
//    case 128000:
//    case 256000:
//        break;
//    default :
//        return FALSE;
//    }
		if((cfg->bound >= Bound_Maximum)||(0 == cfg->bound))
		{
			return FALSE;
		}
    switch (cfg->check)
    {
    case 0:
    case 1:
    case 2:
        break;
    default :
        return FALSE;
    }
    switch (cfg->dataBit)
    {
    case 8:
//    case 7:
        break;
    default :
        return FALSE;
    }
    switch (cfg->stopBit)
    {
    case 1:
    case 2:
    case 3:
//    case 4:
        break;
    default :
        return FALSE;
    }
    return TRUE;
}

/**************************Copyright BestFu 2014-05-14*************************/
