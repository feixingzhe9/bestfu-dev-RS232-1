/***************************Copyright BestFu 2014-05-14*************************
文	件：	main.c
说	明：	系统主函数
编	译：	Keil uVision4 V4.54.0.0
版	本：	v1.0
编	写：	Unarty
日	期：	2014.06.22 
修　改：	暂无
*******************************************************************************/
#include "SysHard.h"
#include "Thread.h"
#include "wdg.h"
#include "cmdProcess.h"
#include "Instruct.h"
#include "DeviceReset.h"
#include "Test.h"

#include "USARTx.h"

/*******************************************************************************
函 数 名：	main
功能说明： 	主函数
参	  数： 	无
返 回 值：	无
*******************************************************************************/
int main(void)
{
	Thread_Init();
 	SysHard_Init();		//系统相关硬件初始化
	WDG_Init(5,2000);
	FIFOInit();			//初始化队列缓冲区
	Instruct_Init();
	DeviceResetCheck_Init();
	Test_Init();
	Unit_Init();
	
	while(1)
	{
		Process_FIFOData(&rcv433fifo, &rcv_433); 		//处理接收FIFO		
 		Thread_Process();
	}
}

/**************************Copyright BestFu 2014-05-14*************************/
