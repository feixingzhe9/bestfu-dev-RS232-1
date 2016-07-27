/***************************Copyright BestFu 2014-05-14*************************
��	����	main.c
˵	����	ϵͳ������
��	�룺	Keil uVision4 V4.54.0.0
��	����	v1.0
��	д��	Unarty
��	�ڣ�	2014.06.22 
�ޡ��ģ�	����
*******************************************************************************/
#include "BF_type.h"
#include "SysHard.h"
#include "Thread.h"
#include "cmdProcess.h"
#include "Instruct.h"
#include "wdg.h"
#include "DeviceReset.h"
#include "Test.h"
#include "stdio.h"


#define ITM_Port8(n)    (*((volatile unsigned char *)(0xE0000000+4*n)))
#define ITM_Port16(n)   (*((volatile unsigned short*)(0xE0000000+4*n)))
#define ITM_Port32(n)   (*((volatile unsigned long *)(0xE0000000+4*n)))
#define DEMCR           (*((volatile unsigned long *)(0xE000EDFC)))        
#define TRCENA          0x01000000

struct __FILE { int handle; /* Add whatever you need here */ };
FILE __stdout;
FILE __stdin;

int fputc(int ch, FILE *f)
{
    if (DEMCR & TRCENA)
    {
        while (ITM_Port32(0) == 0);
        ITM_Port8(0) = ch;
    }
    return ch;
}

/*******************************************************************************
  ������:  	main
����˵��:	������
	����:  	��
  ����ֵ:  	��
*******************************************************************************/
int main(void)
{
	Thread_Init();
	SysHard_Init();		//ϵͳ���Ӳ����ʼ��
	WDG_Init(5,2000);
	FIFOInit();			//��ʼ�����л�����
	Instruct_Init();
	DeviceResetCheck_Init();
	Test_Init();
	Unit_Init();
	            
	while(1)
	{
		Process_FIFOData(&rcv433fifo, &rcv_433); 		//�������FIFO	
		Process_FIFOData(&rcv433_shortfifo, &rcv_433short); 		//�������FIFO		
		Thread_Process();
	}
}

/**************************Copyright BestFu 2014-05-14*************************/
