/***************************Copyright BestFu 2014-05-14*************************
��	����	Upload.c
˵	����	�豸�����ϱ���غ���
��	�룺	Keil uVision4 V4.54.0.0
��	����	v1.0
��	д��	Unarty
��	�ڣ�	2014.07.03 
�ޡ��ģ�	����
*******************************************************************************/
#include "Upload.h"
#include "MsgPackage.h"
#include "UnitShare_Attr.h"
#include "Thread.h"
#include "SysTick.h"
#include "cmdProcess.h"
#include "FaultManage.h"

//Private Function declaration
static u8 Fault;
static UploadCondition_t UploadUnit[MAX_UPLOAD_UNIT];
static void Upload_Attr(u8 unit);
static void FaultMsg(void);

#ifndef   STM32L151  //�������豸�ϱ� yanhuan adding 2015/12/18
extern void SysTickUpload(void); 
static void Upload_Idle(void);
#endif 
/*******************************************************************************
�� �� ����	Upload_Fault
����˵���� 	�����쳣�ϱ�
��	  ���� 	upload: �쳣���������쳣ö�ٸ��
�� �� ֵ��	��
*******************************************************************************/
void Upload_Fault(Upload_t upload)
{
	u32 time;
	
	Fault = (u8)upload;
	if (upload == USER_DATA_RESET)
	{
		time = 1000;
	}
	else
	{
		time = 5;
	}
#ifndef   STM32L151 		//���ǵ͹��Ĳ�Ʒ
	if(upload == RESTART)
		Thread_Login(ONCEDELAY, 0, 3000, &Upload_Idle);	//����ʱ�����ϱ�����ֹ��������
#endif 
	Thread_Login(ONCEDELAY, 0, time, &FaultMsg);
}
/*******************************************************************************
�� �� ����	FaultMsg
����˵���� 	�쳣��Ϣ��װ
��	  ���� 	upload: �쳣���������쳣ö�ٸ��
�� �� ֵ��	��
*******************************************************************************/
void FaultMsg(void)
{
	Msg_Upload(0, 0xFF, 1, (u8*)&Fault);
}
/*******************************************************************************
�� �� ����	Upload_Check
����˵���� 	�豸����״̬�ϱ�ʱ��˶�
��	  ���� 	��
�� �� ֵ��	��
ע    ��:	��
*******************************************************************************/
void Upload_Check(void)
{
	u8 i = UnitCnt_Get();
	while(--i)
	{
		if(gUnitData[i].UploadAble)
		{		
			if(UploadUnit[i].flag == 1 && ((++UploadUnit[i].cnt)*20) >= UploadUnit[i].time)
			{
				Upload_Attr(i);	
				UploadUnit[i].flag =0;//�����ϱ�����0				
			}
		}
		else
			UploadUnit[i].flag =0;//�����ϱ�����0			
	}
    //����Ƿ�����Ҫ�ϱ��ĵ�Ԫ
	for(i = 1; i < UnitCnt_Get() ;i++) 
	{
		if(UploadUnit[i].flag == 1)//������Ҫ�ϱ��ĵ�Ԫ
		{
			return ;
		}		
	}
	Thread_Logout(Upload_Check);
}
/*******************************************************************************
�� �� ����	Upload
����˵���� 	�豸����״̬�ϱ�
��	  ���� 	��
�� �� ֵ��	��
ע    ��:	��ɢ��10ms
*******************************************************************************/
void Upload(u8 unit)
{	
	u8 i,num ;
	if(unit)  
	{
		i = unit ;
		num = unit + 1 ;
	}
	else  //0 �ϱ����з�0��Ԫ
	{
		i = 1 ;
		num = UnitCnt_Get();
	}
	for( ; i < num ; i++)
	{
		if(gUnitData[i].UploadAble)
		{
			UploadUnit[i].time = 2500 + (Time_Get()%gUnitData[i].UploadSetTime*100)*10;//װ�ض�����Ԫ�ϱ���ʱ��
			UploadUnit[i].cnt = 0;
			UploadUnit[i].flag = 1; //��Ҫ�ϱ�����1
		}
		else
			UploadUnit[i].flag =0;//�����ϱ�����0
	}
	Thread_Login(FOREVER, 0, 20, &Upload_Check);
}

/*******************************************************************************
�� �� ����	void Upload_Immediately(u8 unit,u8 cmd,u8* data)
����˵���� 	�豸����״̬�ϱ��������ϱ���Ϊ�˶�������������
��	  ���� 	unit����Ҫ�����ϱ��ĵ�Ԫ��
			cmd:�ϱ������Ժ�
			data:�ϱ���������ָ��
�� �� ֵ��	��
ע    ��:	����ʱ�ϱ�
*******************************************************************************/
void Upload_Immediately(u8 unit,u8 cmd,u8* data)
{
	u8 len = 0u;
	u8 udata[255]={0};
	
	len = Get_SingleUnit_AttrLen(unit,cmd);
	if(len > 0u)
	{
		udata[0] = COMPLETE;
		udata[1] = unit;
		udata[2] = cmd;
		udata[3] = len;
		memcpy(&udata[4],data,len);
		Msg_UploadUnit(0x00 , 0x00, len + 4, &udata[0]);	
	}
}

#ifndef   STM32L151 		//���ǵ͹��Ĳ�Ʒ
/*******************************************************************************
�� �� ����	Upload_Idle
����˵���� 	����ʱ�䣬�豸����״̬�ϱ�����ֹ����
��	  ���� 	��
�� �� ֵ��	��
ע    ��:	1h-2h ��ɢ��1s-3600s
*******************************************************************************/
void Upload_Idle(void)
{
	u32 time = 3600000 + (GetRandomTime()%3600)*1000;
	Thread_Login(ONCEDELAY , 0, time, &Upload_Idle);
	Upload_Attr(0);
}
/*******************************************************************************
�� �� ����	SysTickUpload
����˵���� 	�������ϱ�����������ѯ����֡�ϱ� �豸ID+��ˮ�ţ�5 bytes��
��	  ���� 	��
�� �� ֵ��	��
ע    ��:	��
*******************************************************************************/
void SysTickUpload(void)
{
	#if (COMMUNICATION_SHORT_FRAME == 1) //��֡������
		u8 udata[10];
		(*(ShortCMDAim_t*)&udata).version = SYSTICK_SHORT_VERSION ; //�ϱ���֡�汾
		(*(ShortCMDAim_t*)&udata).userID = gSysData.userID ; 
		(*(ShortCMDAim_t*)&udata).sourceID = gSysData.deviceID ;
		(*(ShortCMDAim_t*)&udata).serialNum = ++gSysData.serialNum ;
	
		if(FALSE == short_put(&send433_shortfifo, udata, sizeof(ShortCMDAim_t)))//�ж�433�����Ƿ����
			Fault_Upload(FAULT_2 , SEND_433_FIFO_OVER , NULL);//��ʱ�������� yanhuan adding 2015/10/10
	#else
		Msg_UploadUnit(0, 0x00, 0, NULL);//����֡������
	#endif 	
}
#endif 
/*******************************************************************************
�� �� ����	static void Upload_Attr(u8 unit)
����˵���� 	�豸����״̬�ϱ�
��	  ���� 	��
�� �� ֵ��	��
*******************************************************************************/
static void Upload_Attr(u8 unit)
{
	u8 udata[255]={0};
	u8 i = UnitCnt_Get();
	while(i--)
	{				
		if(((unit)&&(i== unit||i ==0))||!unit) //unitΪ0��ȡ���е�Ԫ����
		{
			udata[0] = i; //��Ԫ��
			udata[5] = Get_ManyUnit_Attr((UnitPara_t*)udata, &udata[3], &udata[6]+udata[4]);
			udata[4] += udata[3];	
		}			
	}
	udata[4] ++; //����ͨ�ű�ʶλ		
	Msg_UploadUnit(0, 0x00, udata[4], &udata[5]);	
}


/**************************Copyright BestFu 2014-05-14*************************/
