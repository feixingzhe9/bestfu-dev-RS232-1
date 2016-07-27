/***************************Copyright BestFu ***********************************
**  ��    ����  Sign433Attr.c
**  ��    �ܣ�  433�ź�ǿ�Ȳ������Խӿڲ�(�����豸)
**  ��    �룺  Keil uVision5 V4.72
**  ��    ����  V1.0
**  ��    д��  Seven
**  �������ڣ�  2014.10.08
**  �޸����ڣ�  2014.10.22
**  ˵    ����  
*******************************************************************************/
#include "Sign433.h"
#include "Sign433Attr.h"
#include "MsgPackage.h"
#include "UserData.h"
#include "SI4432.h"
#include "Thread.h"
#define STATE_IDLE           0       //����״̬
#define STATE_SEND_TEST      1       //�ӻ�����״̬
#define STATE_REC_TEST       2       //�ӻ�����״̬

/*******************************************************************************
**��    ���� Set_Sign433_StartSendFrame_Attr()
**��    �ܣ� �����������������������һ�η����źŲ���
**��    ���� *pData      --�������
**��    �أ� COMPLETE
**˵    ���� ����ţ�0xB0   
********************************************************************************/
MsgResult_t Set_Sign433_StartSendFrame_Attr(UnitPara_t *pData)
{
    if( Sign433_State != STATE_IDLE )    return CMD_EXE_ERR;
    else Sign433_State = STATE_SEND_TEST;
    
    gSign433Comm.MasterID              = *(u32*)&pData->data[0]; 
    gSign433Comm.SlaveID               = *(u32*)&pData->data[4];
    gSign433Comm.TestPkgInfo.Number    = pData->data[8];    
    gSign433Comm.TestPkgInfo.Length    = pData->data[9];
    gSign433Comm.TestPkgInfo.TimeSpace = *(u16*)&pData->data[10];
       
    Thread_Login(ONCEDELAY , 0 , 1000 , Sign433_SendTestFrameStream);//1��������źŷ��Ͳ���
    
    return COMPLETE;
}
/*******************************************************************************
**��    ���� Get_Sign433_MasterAddr
**��    �ܣ� ��ȡ���������ĵ�ַ
**��    ���� ��
**��    �أ� ��������address
**˵    ���� ��
********************************************************************************/
u32 Get_Sign433_MasterAddr(void)
{
	return (*(u32*)&gSign433Comm) ;
}
/*******************************************************************************
**��    ���� Get_Sign433_MasterAddr
**��    �ܣ� ��ȡ�������ݰ�����
**��    ���� ��
**��    �أ� ��������address
**˵    ���� ��
********************************************************************************/
u8 Get_Sign433_TestPackNum(void)
{
	return gSign433Comm.TestPkgInfo.Number;
}
/*******************************************************************************
**��    ���� Get_Sign433_StartSendFrame_Attr()
**��    �ܣ� ��ȡ�����źŲ���״̬
**��    ���� *pData      --�������
**��    �أ� COMPLETE
**˵    ���� ����ţ�0xB0   
********************************************************************************/
MsgResult_t Get_Sign433_StartSendFrame_Attr(UnitPara_t *pData, u8 *rlen, u8 *rpara)
{
    *rlen = 13;
    
    *(u32*)&rpara[0] = gSign433Comm.MasterID ;
    *(u32*)&rpara[4] = gSign433Comm.SlaveID  ;
    rpara[8]         = gSign433Comm.TestPkgInfo.Number ;
    rpara[9]         = gSign433Comm.TestPkgInfo.Length ;
    *(u16*)&rpara[10]= gSign433Comm.TestPkgInfo.TimeSpace ;
    rpara[12]        = Sign433_State ;  //��ǰ����״̬
    
    return COMPLETE;
}

/*******************************************************************************
**��    ���� Set_Sign433_StartRecFrame_Attr()
**��    �ܣ� �����������������������һ�� �����źŲ���
**��    ���� *pData      --�������
**��    �أ� COMPLETE
**˵    ���� ����ţ�0xB1  
********************************************************************************/
MsgResult_t Set_Sign433_StartRecFrame_Attr(UnitPara_t *pData)
{
    u8 *pRSSI = NULL;
    if(Sign433_State != STATE_SEND_TEST)    return CMD_EXE_ERR;
    else Sign433_State = STATE_REC_TEST;
    
    gSign433Comm.MasterID              = *(u32*)&pData->data[0]; 
    gSign433Comm.SlaveID               = *(u32*)&pData->data[4]; //gSysData.deviceID;
    gSign433Comm.TestPkgInfo.Number    = pData->data[8];    
    gSign433Comm.TestPkgInfo.Length    = pData->data[9];
    gSign433Comm.TestPkgInfo.TimeSpace = *(u16*)&pData->data[10];
    
    //�����RSSI����
	SetRecTestStatus(1);
	Thread_Login(ONCEDELAY , 0 , 5000 , ClearRecTestStatus); // yanhuan adding 2015/12/02
	pRSSI=GetRSSIPara();
	memset(pRSSI, 0, gSign433Comm.TestPkgInfo.Number+1);

    return COMPLETE;
}

/*******************************************************************************
**��    ���� Get_Sign433_StartRecFrame_Attr()
**��    �ܣ� ��ȡ���ղ��Բ���
**��    ���� *pData      --�������
**��    �أ� COMPLETE
**˵    ���� ����ţ�0xB1   
********************************************************************************/
MsgResult_t Get_Sign433_StartRecFrame_Attr(UnitPara_t *pData, u8 *rlen, u8 *rpara)
{
    *rlen = 13;
    
    *(u32*)&rpara[0] = gSign433Comm.MasterID ;
    *(u32*)&rpara[4] = gSign433Comm.SlaveID  ;
    rpara[8]         = gSign433Comm.TestPkgInfo.Number ;
    rpara[9]         = gSign433Comm.TestPkgInfo.Length ;
    *(u16*)&rpara[10]= gSign433Comm.TestPkgInfo.TimeSpace ;
    rpara[12]        = Sign433_State ;  //��ǰ����״̬
    
    return COMPLETE;
}
/*******************************************************************************
**��    ���� Set_Sign433_RecFrameStream_Attr()
**��    �ܣ� ���ղ���������
**��    ���� *pData      --�������
**��    �أ� COMPLETE
**˵    ���� ����ţ�0xB2
********************************************************************************/
MsgResult_t Set_Sign433_RecFrameStream_Attr(UnitPara_t *pData)
{
    Thread_Login(ONCEDELAY , 0 , 5000 , ClearRecTestStatus); // yanhuan adding 2015/12/02
    if((pData->data[0] == 0XFF)&&(Sign433_State==STATE_REC_TEST))      //����֡
    {
        Thread_Logout(ClearRecTestStatus);
				ClearRecTestStatus();
				Thread_Login(ONCEDELAY, 0, 1000, &Sign433_SendTestReport); //  2015/09/17 yanhuan adding ���200ms �������α���
        Sign433_State = STATE_IDLE;       
    }	
    return COMPLETE;
}
/*******************************************************************************
**��    ���� Get_DeviceThreshold_Attr
**��    �ܣ� ��ȡ�豸ͨ���ż�ֵ
**��    ���� *pData      --�������
**��    �أ� COMPLETE
**˵    ���� ����ţ�0xBE
********************************************************************************/
MsgResult_t Get_DeviceThreshold_Attr(UnitPara_t *data, u8 *rlen, u8 *rpara)  
{
	*rlen = 1 ;
	*rpara = GetThresholdPara();
	return COMPLETE;
}
/*******************************************************************************
**��    ���� Set_Sign433_TestState_Attr()
**��    �ܣ� ���ò���״̬--�жϵ�ǰ����
**��    ���� *pData      --�������
**��    �أ� COMPLETE
**˵    ���� ����ţ�0xBF
********************************************************************************/
MsgResult_t Set_Sign433_TestState_Attr(UnitPara_t *pData)
{
    Sign433_State = STATE_IDLE;
	Thread_Logout(Sign433_SendTestFrame); //ֹͣ����
    return COMPLETE;
}
/*******************************************************************************
**��    ���� Get_Sign433_TestState_Attr()
**��    �ܣ� ��ȡ����״̬
**��    ���� *pData      --�������
**��    �أ� COMPLETE
**˵    ���� ����ţ�0xBF
********************************************************************************/
MsgResult_t Get_Sign433_TestState_Attr(UnitPara_t *pData, u8 *rlen, u8 *rpara)
{
    *rlen = 1;
    rpara[0] = Sign433_State;
    return COMPLETE;
}
 
/***************************Copyright BestFu **********************************/    
