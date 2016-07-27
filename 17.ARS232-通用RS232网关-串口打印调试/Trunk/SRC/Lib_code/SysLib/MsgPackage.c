/***************************Copyright BestFu 2014-05-14*************************
��	����	MsgPackage.c
˵	����	�豸������Ϣ������غ���
��	�룺	Keil uVision4 V4.54.0.0
��	����	v1.0
��	д��	Unarty
��	�ڣ�	2014.06.24 
�ޡ��ģ�	����
*******************************************************************************/
#include "MsgPackage.h"
#include "UserData.h"
#include "BestFuLib.h"
#include "cmdProcess.h"
#include "FaultManage.h"
#include "DataEncrypt.h"
//static u8 serialNum;	//��Ϣ��ˮ��

//Priivate function declaration
static void Msg_Package(CMDAim_t *aim, CMDPara_t *para, u8 *data);

/*******************************************************************************
�� �� ����	SaveSerialNum
����˵���� 	��Ϣ����
��	  ���� 	��
�� �� ֵ��	��
*******************************************************************************/
void SaveSerialNum(void)
{
	SysEEPROMData_Set(sizeof(gSysData.serialNum) , (u8*)&gSysData.serialNum);	
}

/*******************************************************************************
�� �� ����	Msg_Feedback
����˵���� 	��Ϣ����
��	  ���� 	pMsg:	��Ϣ����
�� �� ֵ��	TRUE(�ظ�)/FALSE(���ظ�)
*******************************************************************************/
void Msg_Feedback(Communication_t *pMsg)
{
	pMsg->version = COMMUNICATION_VERSION;
	pMsg->aim.userID = gSysData.userID;
	pMsg->aim.sourceID = gSysData.deviceID;
	
	if (pMsg->aim.object.id == gSysData.deviceID || pMsg->aim.objectType > SINGLE_ACT)
	{
		if(!EncryptCmd_put(&rcv433fifo, (u8*)pMsg, sizeof(Communication_t) + pMsg->para.len))
		{
			Fault_Upload(FAULT_2 , RECV_433_FIFO_OVER , NULL);//��ʱ��������		
		}
	}
	if(pMsg->aim.object.id != gSysData.deviceID)//Ŀ��ID�����Լ�
	{
		if(!EncryptCmd_put(&send433fifo, (u8*)pMsg, sizeof(Communication_t) + pMsg->para.len))
		{
			Fault_Upload(FAULT_2 , SEND_433_FIFO_OVER , NULL);//��ʱ��������
		}
	}		
}
	
/*******************************************************************************
�� �� ����	Msg_Upload
����˵���� 	��Ϣ�ϱ�
��	  ���� 	unit:	��Ԫ��
			cmd��	���Ժ�
			len:	��������
			*data:	��������
�� �� ֵ��	��
*******************************************************************************/
void Msg_Upload(u8 unit, u8 cmd, u8 len, u8 *data)
{
	u8 buf[64];
	
	buf[0] = COMPLETE;	//����ͨ�ųɹ���ʶ
	buf[1] = unit;
	buf[2] = cmd;
	buf[3] = len;
	memcpy(&buf[4], data, len);
	
	Msg_UploadUnit( 0x00, 0x00, len + 4, buf);
}

/*******************************************************************************
�� �� ����	Msg_UploadUnit
����˵���� 	��Ԫ��Ϣ�ϱ�
��	  ���� 	unit:	��Ԫ��
			cmd��	���Ժ�
			len:	��������
			*data:	��������
�� �� ֵ��	��
*******************************************************************************/
void Msg_UploadUnit(u8 unit, u8 cmd, u8 len, u8 *data)
{
	CMDAim_t aim;
	CMDPara_t para;
	
	aim.objectType = SINGLE_ACT;
	aim.object.id = MSG_UPLOAD_ID;
	aim.actNum = 0;
	
	para.msgType = EVENT;
	para.unit = unit;
	para.cmd = cmd;
	para.len = len;
	Msg_Package(&aim, &para, data);
}
/*******************************************************************************
�� �� ����	Msg_Send
����˵���� 	��Ϣ����
��	  ���� 	objectType:		Ŀ���ַ����
			objectID��		Ŀ���ַ
			actNum:			�㲥���ͺ�
			unit:			��Ԫ��
			cmd��			���Ժ�       
			len:			��������
			*data:			��������
�� �� ֵ��	��
*******************************************************************************/
void Msg_Send(ObjectType_t objecttype, u32 objectID, u16 actNum, u8 unit, u8 cmd, u8 len, u8 *data)
{
	CMDAim_t aim;
	CMDPara_t para;
	
	aim.objectType = objecttype;
	aim.object.id = objectID;			//����Ŀ���ַ
	aim.actNum = actNum;	
	
	/*��Ϣ��������ֵ*/
	para.msgType = WRITENACK;			//��Ϣ���ͣ��¼�
	para.cmd = cmd;
	para.unit = unit;
	para.len = len;
	Msg_Package(&aim, &para, data);
}

/*******************************************************************************
�� �� ����	Msg_Package
����˵���� 	��Ϣ���
��	  ���� 	aim:	Ŀ������
			para:	ָ������
			*data:	��������
�� �� ֵ��	��
*******************************************************************************/
void Msg_Package(CMDAim_t *aim, CMDPara_t *para, u8 *data)
{
	u8 msg[250];
	Communication_t *pMsg = (Communication_t*)msg;
	
	memcpy(&pMsg->aim, aim, sizeof(CMDAim_t));
	pMsg->aim.serialNum = ++gSysData.serialNum;		//������Ϣ��ˮ��
	
	/*��Ϣ��������ֵ*/
	memcpy(&pMsg->para, para, sizeof(CMDPara_t));
	memcpy(&msg[sizeof(Communication_t)], data, para->len);
	Msg_Feedback(pMsg);
}

/**************************Copyright BestFu 2014-05-14*************************/
