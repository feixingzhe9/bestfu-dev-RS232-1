/***************************Copyright BestFu 2014-05-14*************************
文	件：    command.c
说	明:	    从队列中获取一条指令
编	译：    Keil uVision4 V4.54.0.0
版	本：    v1.0
编	写：    Unarty
日	期：    2013-10-14
修  改:     暂无
*******************************************************************************/
#include "command.h"
#include "fifo.h"
#include "crc.h"
#include "DataEncrypt.h"
#include "MsgPackage.h"
#include "Updata.h"
#include "stdio.h"


const u8 HeadCMD[HEAD_SIZE] = {0xF1, 0xF1, 0xF1, 0xF2, 0xF1, 0xF1};
const u8 TailCMD[TAIL_SIZE] = {0xF2, 0xF2, 0xF2, 0xF1, 0xF2, 0xF2};

/*******************************************************************************
函 数 名：  head_put
功能说明：  队列中放置一个数据标识头
参	  数：  fifo:	目标队列
返 回 值：  放置结果，TRUR(放置成功)/FASLE
*******************************************************************************/
u8 head_put(FIFO_t *fifo)
{
	return fifo_puts(fifo, (u8*)HeadCMD, sizeof(HeadCMD));
}

/*******************************************************************************
函 数 名：  tail_put
功能说明：  队列中放置一个数据标识尾
参	  数：  fifo:	目标队列
返 回 值：  放置结果，TRUR(放置成功)/FASLE
*******************************************************************************/
u8 tail_put(FIFO_t *fifo)
{
	return fifo_puts(fifo, (u8*)TailCMD, sizeof(TailCMD));
}

#if 0
/*******************************************************************************
函 数 名：  cmd_get
功能说明：  从队列中获取一条指令
参	  数：  fifo:	获取源
            cmdBuf: 指令存储缓冲区	
返 回 值：  指令长度。为零，没有可读指令
*******************************************************************************/
u32 cmd_get(u8 *cmdBuf, FIFO_t *fifo)
{
	u32 i;
     
	ERRR(fifo == NULL, goto ERR1);
	
	ERRR(fifo_empty(fifo) == TRUE, goto ERR1);        //FIFO为空
	i = fifo_find(fifo, HeadCMD, sizeof(HeadCMD));
	fifo->front = i;
	ERRR(fifo_empty(fifo) == TRUE, goto ERR1);        //FIFO为空

	i = sizeof(Communication_t) + sizeof(HeadCMD);
	ERRR(i > fifo_validSize(fifo), goto ERR1); //数据空间不够

	i += fifo->data[(fifo->front + i - ARGE_SEAT)%fifo->size] + CRC_SIZE + sizeof(TailCMD);
	ERRR(i > CMD_SIZE, goto ERR2);	//数据指令长度错误
	ERRR(i > fifo_validSize(fifo), goto ERR1); //数据空间不够
	
	ERRR(fifo_cmp(fifo, (fifo->front + i - sizeof(TailCMD))%fifo->size
					, TailCMD, sizeof(TailCMD)) == FALSE, goto ERR2);	//核对数据尾
					
	ERRR(fifo_gets(fifo, cmdBuf, i) == FALSE, goto ERR1);	//数据内容出队
	ERRR(ChkCrcValue(&cmdBuf[sizeof(HeadCMD)], (i - sizeof(HeadCMD) - sizeof(TailCMD))), goto ERR1);	//CRC错误
	
	return i;

ERR2:	fifo->front = (fifo->front+sizeof(HeadCMD))%fifo->size; //数据错误将头移出
ERR1:	return 0;	
}
/*******************************************************************************
函 数 名：  cmd_put
功能说明：  放置一条指令到FIFO中
参	  数：  fifo:	目标地址
            cmdBuf: 指令存储缓冲区	
			cmdLen:	放置内容长度
返 回 值：  TRUE-成功，FALSE-失败
*******************************************************************************/
u32 cmd_put(FIFO_t *fifo, u8 *cmdBuf, u32 cmdLen)
{
	u16 crc;
	
	ERRR(fifo == NULL, goto ERR1);
	ERRR(fifo_puts(fifo, (u8*)HeadCMD, sizeof(HeadCMD)) == FALSE, goto ERR1);	//放置通信标志头
	crc = ChkCrcValue(cmdBuf, cmdLen);	//获取CRC
	ERRR(fifo_puts(fifo, cmdBuf, cmdLen) == FALSE, goto ERR1);	//放置参数区
	ERRR(fifo_putc(fifo, (u8)(crc>>8)) == FALSE, goto ERR1);	//放置CRC高位	
	ERRR(fifo_putc(fifo, (u8)(crc)) == FALSE, goto ERR1);		//放置CRC底位	
	ERRR(fifo_puts(fifo, (u8*)TailCMD, sizeof(TailCMD)) == FALSE, goto ERR1);	//放置通信标尾
	
	return TRUE;
ERR1:	return FALSE;
}
#endif

#if (COMMUNICATION_SHORT_FRAME == 1)
/*******************************************************************************
函 数 名：  short_get
功能说明：  从队列中获取一条短帧指令
参	  数：  fifo:	获取源
            cmdBuf: 指令存储缓冲区	
返 回 值：  指令长度。为零，没有可读指令
*******************************************************************************/
u32 short_get(u8 *cmdBuf, FIFO_t *fifo)
{
	u32 i;
     
	ERRR(fifo == NULL, goto ERR1);
	
	ERRR(fifo_empty(fifo) == TRUE, goto ERR1);        //FIFO为空
	i = fifo_find(fifo, HeadCMD, sizeof(HeadCMD));
	fifo->front = i;
	ERRR(fifo_empty(fifo) == TRUE, goto ERR1);        //FIFO为空
	
	i = sizeof(HeadCMD) + 1; //帧头和短帧长度字节
	i += fifo->data[(fifo->front + i - ARGE_SEAT)%fifo->size] + CRC_SIZE + sizeof(TailCMD);
	ERRR(i > CMD_SIZE,goto ERR2);	//数据指令长度错误
	ERRR(i > fifo_validSize(fifo), goto ERR1); //数据空间不够
	
	ERRR(fifo_cmp(fifo, (fifo->front + i - sizeof(TailCMD))%fifo->size
					, TailCMD, sizeof(TailCMD)) == FALSE, goto ERR2);	//核对数据尾
				
	ERRR(fifo_gets(fifo, cmdBuf, i) == FALSE, goto ERR1);	//数据内容出队
	i = cmdBuf[sizeof(HeadCMD)]; //取有效字节长度
	ERRR(ChkCrcValue(cmdBuf+sizeof(HeadCMD)+1, i+2), goto ERR1);	//CRC错误
	
	memmove(cmdBuf,cmdBuf+sizeof(HeadCMD)+1,i); //移除数据头
	return i;

ERR2:	fifo->front = (fifo->front+sizeof(HeadCMD))%fifo->size; //数据错误将头移出
ERR1:	return 0;
}
/*******************************************************************************
函 数 名：  short_put
功能说明：  放置一条短帧指令到FIFO中
参	  数：  fifo:	目标地址
            cmdBuf: 指令存储缓冲区	
			cmdLen:	放置内容长度
返 回 值：  TRUE-成功，FALSE-失败
*******************************************************************************/
u32 short_put(FIFO_t *fifo, u8 *cmdBuf, u32 cmdLen)
{
	u16 crc;
	
	ERRR(fifo == NULL, goto ERR1);
	ERRR(fifo_puts(fifo, (u8*)HeadCMD, sizeof(HeadCMD)) == FALSE, goto ERR1);	//放置通信标志头
	crc = ChkCrcValue(cmdBuf, cmdLen);	//获取CRC
	ERRR(fifo_putc(fifo, cmdLen) == FALSE, goto ERR1);	//短帧长度byte
	ERRR(fifo_puts(fifo, cmdBuf, cmdLen) == FALSE, goto ERR1);	//放置参数区
	ERRR(fifo_putc(fifo, (u8)(crc>>8)) == FALSE, goto ERR1);	//放置CRC高位	
	ERRR(fifo_putc(fifo, (u8)(crc)) == FALSE, goto ERR1);		//放置CRC底位	
	ERRR(fifo_puts(fifo, (u8*)TailCMD, sizeof(TailCMD)) == FALSE, goto ERR1);	//放置通信标尾
	
	return TRUE;
ERR1:	return FALSE;
}
#endif
/*******************************************************************************
函 数 名：  EncryptCmd_get
功能说明：  从队列中获取一条加密指令
参	  数：  fifo:	获取源
            cmdBuf: 指令存储缓冲区	
返 回 值：  指令长度。为零，没有可读指令
*******************************************************************************/
u32 EncryptCmd_get(u8 *cmdBuf, FIFO_t *fifo)
{
	u32 i;
     
	ERRR(fifo == NULL, goto ERR1);
	
	ERRR(fifo_validSize(fifo) <= sizeof(HeadCMD), goto ERR1);        //FIFO有效长度小于头,则错误
	i = fifo_find(fifo, HeadCMD, sizeof(HeadCMD));
	fifo->front = i;
	
	ERRR(fifo_empty(fifo) == TRUE, goto ERR1);        //FIFO为空
	i = fifo->data[(fifo->front + sizeof(HeadCMD))%fifo->size]; //获取有效指令长度
	ERRR(i == COMMUNICATION_VERSION, goto ERR2);	 //旧通信协议帧直接移除 yanhuan adding 2016/01/13 
	i += sizeof(HeadCMD) + sizeof(TailCMD);		//获取参数区总长度
	ERRR(i > CMD_SIZE, {printf("error:data too long \n");goto ERR2;});	//数据指令长度错误
	ERRR(i > fifo_validSize(fifo), {printf("error:space error \n");goto ERR1;}); //数据空间不够

	ERRR(fifo_cmp(fifo, (fifo->front + i - sizeof(TailCMD))%fifo->size
        , TailCMD, sizeof(TailCMD)) == FALSE, {printf("error:tail error \n");goto ERR2;});	//核对数据尾
	ERRR(fifo_gets(fifo, cmdBuf, i) == FALSE, {printf("error:get error \n");goto ERR1;});	//数据内容出队

	return i;

ERR2:	fifo->front = (fifo->front+sizeof(HeadCMD))%fifo->size; //数据错误将头移出
ERR1:	return 0;	
}

/*******************************************************************************
函数  名:  BF_INT32U EncryptCmd_getEx(BF_INT08U *cmdBuf, FIFO_t *fifo)
功能说明:  从加密队列中获取一帧数据,并解密数据
参    数:  fifo：缓冲队列的首地址
		   cmdBuf：存储返回数据的首地址
返回  值:  无
*******************************************************************************/
u32 EncryptCmd_getEx(u8 *cmdBuf, FIFO_t *fifo)
{
    u32 pos = EncryptCmd_get(cmdBuf, fifo);

	if(pos && (!DecryptData((u8*)(&cmdBuf[HEAD_SIZE]))))	//若解密成功
	{
		pos = HEAD_SIZE + sizeof(Communication_t) + ((Communication_t*)&cmdBuf[HEAD_SIZE])->para.len + CRC_SIZE;
		memcpy(&cmdBuf[pos], (u8*)TailCMD, TAIL_SIZE); 
		pos += TAIL_SIZE;
	}
//    else
//    {
//        pos = 0;
//        printf("error:DecryptData error \n");   
//    }
    return (pos);
}

/*******************************************************************************
函 数 名：  EncryptCmd_put
功能说明：  加密一条指令后放置到FIFO中
参	  数：  fifo:	目标地址
            cmdBuf: 指令存储缓冲区	
			cmdLen:	放置内容长度
返 回 值：  指令长度。为零，没有可读指令
*******************************************************************************/
u32 EncryptCmd_put(FIFO_t *fifo, u8 *cmdBuf, u32 cmdLen)
{
	u16 crc;
	u8 cmdBuf_temp[256];

	ERRR(fifo == NULL, goto ERR1);
	ERRR(cmdLen > CMD_SIZE, goto ERR1);

	memcpy(cmdBuf_temp, cmdBuf, cmdLen);
	crc = ChkCrcValue(cmdBuf_temp, cmdLen);	//获取CRC
	ERRR(fifo_puts(fifo, (u8*)HeadCMD, sizeof(HeadCMD)) == FALSE, goto ERR1);	//放置通信标志头
	cmdBuf_temp[cmdLen++] = (u8)(crc >> 8);
	cmdBuf_temp[cmdLen++] = (u8)(crc);		//添加数据CRC
	
	ERRR(!EncryptData(cmdBuf_temp, cmdLen), goto ERR1);
	cmdLen = cmdBuf_temp[0];
	ERRR(cmdLen > CMD_SIZE, goto ERR1);
	ERRR(fifo_puts(fifo, cmdBuf_temp, cmdLen) == FALSE, goto ERR1);	//放置参数区
	ERRR(fifo_puts(fifo, (u8*)TailCMD, sizeof(TailCMD)) == FALSE, goto ERR1);	//放置通信标尾
	
	return TRUE;
ERR1:	return FALSE;
}
/**************************Copyright BestFu 2014-05-14*************************/
