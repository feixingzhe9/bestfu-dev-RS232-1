/***************************Copyright BestFu ***********************************
**  ��    ����  Sign433.c
**  ��    �ܣ�  �豸433�ź�ǿ�Ȳ���
**  ��    �룺  Keil uVision5 V4.72
**  ��    ����  V1.1
**  ��    д��  Seven
**  �������ڣ�  2014.10.08
**  �޸����ڣ�  2014.10.28
**  ˵    ����  >>V1.1  ��ֹ�͹����豸���Թ���������
*******************************************************************************/
#include "Sign433.h"
#include "Thread.h"
#include "MsgPackage.h"
#include "SI4432.h"

#ifdef STM32L151
extern  void StandbyCountReset(void);
#endif

#define MASTER_REC_FRAME_STREAM 0XB5        //�������ղ���������
#define MASTER_REC_TEST_REPORT  0XB6        //�������ղ��Ա���

Sign433Comm_t gSign433Comm;     //433�źŲ�����Ϣ�ṹ��
u8 Sign433_State=0;             // 0 ����   1 ���Ͳ���   2���ղ���
static u8 SendCount=0;          //���ͼ���

/*******************************************************************************
**��    ���� Sign433_SendTestFrame()
**��    �ܣ� ���Ͳ�������֡
********************************************************************************/
void Sign433_SendTestFrame(void)
{
    u8 pData[255];   //����������� 
    u8 len = gSign433Comm.TestPkgInfo.Length;    
    
#if STM32L151		//ר���ڵ͹��Ĳ�Ʒ������ʱ������
			StandbyCountReset();
#endif 
     
    pData[0] = 0XBB;            //����֡��
    pData[1] = SendCount++;
    
    if(SendCount > gSign433Comm.TestPkgInfo.Number)        //����֡��
    {
        pData[0] = 0xFF; 
        len = 2;
    }
    //Ŀ¼����/��/����          Ŀ��ID       �顢������   ��Ԫ��   ���Ժ�   ���ݳ���   ����  
    //(ObjectType_t objecttype, u32 objectID, u16 actNum, u8 unit, u8 cmd, u8 len, u8 *data);
    Msg_Send( SINGLE_ACT , gSign433Comm.MasterID , 0 , 0x01 , MASTER_REC_FRAME_STREAM , len , pData);
}

/*******************************************************************************
**��    ���� Sign433_SendTestFrameStream()
**��    �ܣ� ���Ͳ���������
********************************************************************************/
void Sign433_SendTestFrameStream(void)
{
    SendCount = 0;
    Thread_Login(MANY, gSign433Comm.TestPkgInfo.Number+2,   //��������֡
                       gSign433Comm.TestPkgInfo.TimeSpace, &Sign433_SendTestFrame);  // ע���߳� ��Ҫ���Ͳ������ݰ�
}
/*******************************************************************************
**��    ���� ClearRecTestStatus()
**��    �ܣ� �������RSSI��־
**��    ���� ��
**��    �أ� ��
**˵    ���� ��ֹ�����жϲ���433��������
********************************************************************************/
void ClearRecTestStatus(void)
{
	SetRecTestStatus(0);
}
/*******************************************************************************
**��    ���� Sign433_SendTestReport()
**��    �ܣ� ���Ͳ��Ա���֡
********************************************************************************/
void Sign433_SendTestReport(void)
{
	u8 *pRSSI=NULL,i;
	pRSSI=GetRSSIPara();
	for(i=0 ;i<2;i++)
	{
	  Msg_Send( SINGLE_ACT , gSign433Comm.MasterID , 0 , 0x01 ,  \
              MASTER_REC_TEST_REPORT , gSign433Comm.TestPkgInfo.Number+1, pRSSI);//81�������䷢�͸���֡
	}		   
}

/***************************Copyright BestFu **********************************/ 
