/***************************Copyright BestFu 2014-05-14*************************
文 件：  RS232Attr.c
说 明：  音乐属性接口属性原代码文件
编 译：  Keil uVision4 V4.54.0.0
版 本：  v2.0
编 写：  Unarty
日 期：  2014.08.20
修　改：    2014.11.20 Unarty Add 音乐进度条指令 0x17 设置播放进度（0～100）
*******************************************************************************/
#include "RS232Attr.h"
#include "RS232Unit.h"
#include "USARTx.h"
#include "Thread.h"
#include "led.h"

//对属性读写的接口列表
const AttrExe_st RS232AttrTable[] =
{
    {0x01, LEVEL_0, Set_RS232Cfg_Attr       , Get_RS232Cfg_Attr     ,0},
    {0x03, LEVEL_0, Set_RS232Data_Attr      , Get_RS232Data_Attr	,0},
    {0x04, LEVEL_0, Set_RS232DataSend_Attr  , NULL					,0},
    {0x05, LEVEL_0, NULL                    , Get_Query_Status_Attr ,0},
};

/*******************************************************************************
函 数 名:  void RS232ParaInit(u8 unitID)
功能说明:  串口通信初始化
参    数:  unitID-单元号
返 回 值:  无
*******************************************************************************/
void UnitInstruct(void)
{
	UnitInstructLED();
}

/*******************************************************************************
函 数 名:  void RS232ParaInit(u8 unitID)
功能说明:  串口通信初始化
参    数:  unitID-单元号
返 回 值:  无
*******************************************************************************/
void RS232ParaInit(u8 unitID)
{
	RS232Unit_Init();
	LED_Init();
	Thread_Login(FOREVER,0,100,&CheckQueryRecieve);
	GreenOn();	
	Thread_Login(ONCEDELAY,1,2000,GreenOff);
}

/*******************************************************************************
函 数 名:  MusicEEPROMInit
功能说明:  音乐参数初始化
参    数:  unitID-单元号
返 回 值:  无
*******************************************************************************/
void RS232EEPROMInit(u8 unitID)
{
	UARTCfg_t cfg;
	u8 buf[UART_CMD_NUM_MAX] = {0};

	RS232_CfgDefault(&cfg);	
	Clr_QueryRcvData();//清除查询的数据	
	/*清除配置的命令*/
	EEPROM_Write(UART_FLAG_ADDR,UART_CMD_NUM_MAX,buf);//清除命令ID标志位，32个字节，一共256个标志位
}

/*******************************************************************************
函 数 名:  u8 ARS232_UnitAttrNum(void)
功能说明:  计算rs232网管属性命令的个数
参    数:  无
返 回 值:  返回rs232网管属性命令的个数
*******************************************************************************/
u8 ARS232_UnitAttrNum(void)
{
	return (BF_DIM(RS232AttrTable));
}

/*******************************************************************************
函 数 名:    Set_RS232Cfg_Attr
功能说明:   设置串口配置参数
参    数:     pData:  单元号/属性号/参数长度/参数值
返 回 值:    操作结果
*******************************************************************************/
MsgResult_t Set_RS232Cfg_Attr(UnitPara_t *pData)
{
    if (FALSE == RS232_CfgCheck((UARTCfg_t *)pData->data))   //参数异常
    {
        return PARA_MEANING_ERR;
    }

    RS232_CfgSave((UARTCfg_t *)pData->data); //保存设置参数
    Thread_Login(ONCEDELAY, 0, 10, RS232Unit_Init);  //随后重置433配置

    return COMPLETE;
}

/*******************************************************************************
函 数 名:    Get_RS232Cfg_Attr
功能说明:   获取串口配置参数
参    数:     pData:  单元号/属性号/参数长度/参数值
            *rlen-返回参数长度
            *rpara-返回参数存放地址
返 回 值:    消息执行结果
*******************************************************************************/
MsgResult_t Get_RS232Cfg_Attr(UnitPara_t *pData, u8 *rlen, u8 *rpara)
{
    *rlen = sizeof(UARTCfg_t);
    RS232_CfgGet((UARTCfg_t *)rpara);
    return COMPLETE;
}

/*******************************************************************************
函 数 名:    Set_RS232Data_Attr
功能说明:   设置RS232通信数据
参    数:     pData:  单元号/属性号/参数长度/参数值
返 回 值:    操作结果
*******************************************************************************/
MsgResult_t Set_RS232Data_Attr(UnitPara_t *pData)
{
	if(pData->data[2] + pData->data[pData->data[2] + 3] > 253)
	{
		return PARA_LEN_ERR;
	}
    if (!pData->data[1]) //删除
    {
        RS232_CmdFlagSet(pData->data[0], 0);    //清除指令标识
    }
    else
    {
        RS232_CmdAdd(pData->data[0], (u8 *)&pData->data[2]);
    }

    return COMPLETE;
}

/*******************************************************************************
函 数 名:    Get_RS232Data_Attr
功能说明:   获取串口发送数据
参    数:     pData:  单元号/属性号/参数长度/参数值
            *rlen-返回参数长度
            *rpara-返回参数存放地址
返 回 值:    消息执行结果
*******************************************************************************/
MsgResult_t Get_RS232Data_Attr(UnitPara_t *pData, u8 *rlen, u8 *rpara)
{
    *rlen = RS232_CmdGet(pData->data[0], rpara);

    return COMPLETE;
}

/*******************************************************************************
函 数 名:    Set_RS232DataSend_Attr
功能说明:   设置RS232通信数据发送
参    数:     pData:  单元号/属性号/参数长度/参数值
返 回 值:    操作结果
*******************************************************************************/
MsgResult_t Set_RS232DataSend_Attr(UnitPara_t *pData)
{

	u8 id = 0;
//    if (RS232_CmdFlagGet(pData->data[0]))   //如果ID有效
    {
        id = pData->data[0];
//        Thread_Login(ONCEDELAY, 0, 100, RS232_CmdSend);
		if(1 == RS232_CmdSend(id))
		{
			return COMPLETE;
		}
        
    }

    return CMD_EXE_ERR;
}

/*******************************************************************************
函 数 名:    Get_Query_Status_Attr
功能说明:   查询当前状态
参    数:     pData:  需要查询状态的指令
返 回 值:    操作结果
*******************************************************************************/
MsgResult_t Get_Query_Status_Attr(UnitPara_t *pData, u8 *rlen, u8 *rpara)
{
		
	UART_Recieve.dataRecieveFlag = FALSE;
	
	Thread_Login(FOREVER,0,100,&CheckQueryRecieve);
	
	USARTx_Send((u8 *)(pData->data), pData->len);

	//rpara = (u8 *)QueryRecieve.data;
		
	if(0 == QueryRecieve.len)//没有数据时  返回一个'0'
	{
		*rlen = 1;
		QueryRecieve.data[0] = '0';	
	}
	else
	{
		*rlen = QueryRecieve.len;
	}
	
	memmove(rpara,&QueryRecieve.data,*rlen);		
	return COMPLETE;	
}

/**************************Copyright BestFu 2014-05-14*************************/
