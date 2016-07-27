/***************************Copyright BestFu 2014-05-14*************************
��	����    SysTestAttr.c
˵	����    ϵͳ��������
��	�룺    Keil uVision4 V4.54.0.0
��	����    v2.0
��	д��    Unarty
��	�ڣ�    2014.07.03 
�ޡ��ģ�	����
*******************************************************************************/
#include "SysTestAttr.h"
#include "SI4432.h"
#include "USART.h"
#include "cmdProcess.h"
#include "Test.h"
#include "Thread.h"
#include "Upload.h"

void SI4432Test_Fail(void);

/*******************************************************************************
�� �� ��:  	SI4432Test_Attr 
����˵��:  	433���߲�������
��    ��:  	*pdata-���������ַ��pData[0]Ϊ��Ԫ�ţ�
                                pData[1]Ϊ��������
                                pData[2]Ϊ����ֵ(0Ϊ�رգ���0Ϊ����)
�� �� ֵ:  1��ʾ�ɹ���0��ʾʧ��
*******************************************************************************/
MsgResult_t SI4432Test_Attr(UnitPara_t *pData)
{
	if (Test != TEST_STATE)		//�����ڲ���״̬
	{
		return CMD_EXE_ERR;
	}
	
	if ((u32)send_433.fun == (u32)&UART_Send)	//Ŀ�귢�ʹ�����Ϊ���ں���
	{
		ProcessSend_Fun(&Si4432_Send);
		Thread_Login(ONCEDELAY, 0, 1200, &SI4432Test_Fail);	//ע���ֹʧ�ܺ���		
	}
	else
	{
		ProcessSend_Fun(&UART_Send);			//����Ŀ�귢�ʹ�����
		Si4432_Idle();	//������ģ�����ڷ���״̬��ʹ�䲻���������ⲿ�ź�
	}
	
	return COMPLETE;
}

/*******************************************************************************
�� �� ��:  	SI4432_Test
����˵��:  	433ģ�����
��    ��:  	��
�� �� ֵ:  	��
*******************************************************************************/
void SI4432Test_Fail(void)
{
	Upload_Fault(WIFI_433);
	ProcessSend_Fun(&UART_Send); //�����ʹ����ص����ں���
}

/**************************Copyright BestFu 2014-05-14*************************/
