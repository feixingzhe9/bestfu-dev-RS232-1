/***************************Copyright BestFu 2014-05-14*************************
��	����	Updata.c
˵	����	�������ݴ�����
��	�룺	Keil uVision4 V4.54.0.0
��	����	v1.0
��	д��	Unarty
��	�ڣ�	2013-11-24
�ޡ��ģ�	2015.01.14 Unarty ��ӳ���ռ�˶ԣ���ֹ�ռ�Խ�絼���豸����ʧ�ܻ�����
*******************************************************************************/
#include "Updata.h"
#include "crc.h"
#include "sys.h"
#include "program.h"
#include "DevPartTab.h"
#include "DeviceIDSetAttr.h"
#include "BF_type.h"

//Private variable define


//variable define 
Soft_t IAP_FLAG;
Boot0Info_t bootflag; 		//bootloader��־
RadioPara_t RADIO_PARA ; 	//433ͨ�Ų���
u8 UPDATA_LEVEL = 0xEF;     //������ɼ��� F0/F1/F2/F3/F4
u8 UpgradeAim = 0;			//��¼����Ŀ��
UpgradePre_t UpgradePre = {&bootflag};

//Private function  declarartiuon
static u8 UpgradeReady(UpgradePre_t* pUpgradePre);

/*******************************************************************************
�� �� ����  void UpgradePreStage(UnitPara_t *pData)
����˵����  ����׼��F0�׶�
��	  ����  data[0]: ����������
			data[1]: ��������
			data[2~3]: �豸����
			data[4~7]: �����ļ����ܴ�С
			data[8]: ÿ���������Ĵ�С
			data[9~12]: ��������İ汾��
�� �� ֵ��  ��������״̬
*******************************************************************************/
u32 UpgradePreStage(UnitPara_t *pData)
{
	u32 result = 0u;
	u32 data = BOOTLOADER_1_JUMP_FLAG;
	DevPivotalSet_t* pDevPivotalSet = GetDevPivotalSet();
	
	if(0xF0 == UPDATA_LEVEL)
	{
		if(UPGRADE_BOOTLOADER == pData->data[1])	//�������Ŀ��,�Ƿ�������bootloader����
		{													
			UpgradePre.pBootInfo->appSize 	= *(u32*)(&pData->data[4]);
			UpgradePre.pBootInfo->SoftVer 	= *(u32*)(&pData->data[9]);
			UpgradePre.packetsize = *(u8*)(&pData->data[8]);
			UpgradePre.pBootInfo->jumpFlag	= BOOTLOADER_0_JUMP_FLAG;
			UpgradePre.packetnum  = NUM_ROUND(UpgradePre.pBootInfo->appSize,UpgradePre.packetsize);
			
			if(TRUE == (result = UpgradeReady(&UpgradePre)))
			{	
				UPDATA_LEVEL 	= 0xF1; 
				return COMPLETE;
			}
			return (result);
		}
		else if(UPGRADE_APP == pData->data[1])
		{
			//�豸��ת��bootloader1����
			Program_WriteEx(pDevPivotalSet->DevSoftInfoAddr,sizeof(data),(u8*)&data);
			JumpToCode(DevPartTab->boot1_start_addr);
			return (MsgResult_t)(0);			//����������������������
		}
	}
	else if(0xF1 == UPDATA_LEVEL)
	{
		return COMPLETE;
	}
	else
	{
		UPDATA_LEVEL = 0;
	}
	return PARA_MEANING_ERR;
}
	
/*******************************************************************************
�� �� ����  UpgradeReady
����˵����  ����׼��
��	  ����  pUpgradePre������׼���Ĵ洢ָ��
�� �� ֵ��  ׼����� TRUE(��ɣ�/FALSE(ʧ�ܣ�
*******************************************************************************/
static u8 UpgradeReady(UpgradePre_t* pUpgradePre)
{
    u32 usersize = 0u;
	DevPivotalSet_t* pDevPivotalSet = GetDevPivotalSet();
	
	if (pUpgradePre->packetsize & 0x03)	//������Ĵ�С���ǰ�4�ֽڶ���
	{
		return PACKET_SIZE_ALIGN_ERR;
	}
	usersize = (DevPartTab->boot1_flag_start_addr - DevPartTab->boot1_start_addr);
	if (pUpgradePre->pBootInfo->appSize > usersize)	//���Ӧ�ÿռ���ڿ��ÿռ��С
	{
		return APP_SIZE_BOUND_ERR;					//���������СԽ��
	}	
	memset(pUpgradePre->data, 0, sizeof(UpgradePre_t) - offsetof(UpgradePre_t,data));
	Program_WriteEx(pDevPivotalSet->Boot0FlagAddr, sizeof(bootflag.jumpFlag), (u8*)&bootflag.jumpFlag);
	return Program_Erase(DevPartTab->boot1_start_addr, pUpgradePre->pBootInfo->appSize);  //��Ӧ�洢�ռ�����
}

/******************************************************************************* 
�� �� ����  Updata_PackProcess
����˵����  �������ݰ�����
��	  ����  id:		��Ӧ����ID
			len:	�������ݳ���
			data:	��������
�� �� ֵ��  FALSE/TRUE
*******************************************************************************/
u8 Updata_PackProcess(u32 id, u8 len, u8 *data)
{
	u32 addr;

   	if ((id > (UpgradePre.packetnum - 1))|| (len > 200))
	{
		return FALSE;
	}
	if (!(UpgradePre.data[id >> 5]&(1<<(id & 0x1f))))
	{
	    addr = id*UpgradePre.packetsize + DevPartTab->boot1_start_addr;
	    if (Program_Write(addr, len, data))
		{ 
	        UpgradePre.data[id >> 5] |= (1<<(id & 0x1f)); //���հ�ID��1
	    }
		else
		{
			return FALSE;
		}
	}
	return TRUE;
}

/*******************************************************************************
�� �� ����	Updata_PackCheck
����˵���� 	�������ݰ��˶�
��	  ���� 	*len��	ȱ�����ݳ���
			*data:  ȱ��ID
�� �� ֵ��	TRUE/FLASE
*******************************************************************************/
u8 Updata_PackCheck(u8 *num, u16 *data)
{
    u32 i;

    for (i = 0, *num = 0; i < UpgradePre.packetnum; i++)
	{
        if (!(UpgradePre.data[i >> 5]&(1 << (i & 0x1f))))//��δ�յ�
		{       
			*data++ = i;
			(*num) += 2;
			if ((*num) > 100)
			{		
				break;
			}		
        }
    }
    return (*num == 0 ? TRUE : FALSE);
}

/*******************************************************************************
�� �� ����	Updata_Calibrate
����˵���� 	�豸�������У��
��	  ���� 	len:	���ݳ���
			crc:	У����
�� �� ֵ��	TRUE/FLASE
*******************************************************************************/
u8 Updata_Calibrate(u32 len, u16 crc)
{   
	crc -= ChkCrcValue((u8*)(DevPartTab->boot1_start_addr), len);
	
    return (crc ? FALSE : TRUE);
}

/*******************************************************************************
�� �� ����	Updata_Flag
����˵���� 	������ʶ
��	  ���� 	��
�� �� ֵ��	TRUE/FLASE
*******************************************************************************/
u8 Updata_Flag(void)
{
	u8 data[sizeof(FlashEncrypt_t) + sizeof(DeviceIDSet_t)];	
	DevPivotalSet_t* pDevPivotalSet = GetDevPivotalSet();
	
	bootflag.jumpFlag	= Flash_DefaultValue();
	Program_Read(pDevPivotalSet->DevResetParaAddr,sizeof(FlashEncrypt_t) + sizeof(DeviceIDSet_t),data);
	Program_Erase(DevPartTab->boot0_flag_start_addr, DevPartTab->boot1_start_addr - DevPartTab->boot0_flag_start_addr);	//�޸�Flag����
	if(Program_Write(pDevPivotalSet->DevResetParaAddr,sizeof(FlashEncrypt_t) + sizeof(DeviceIDSet_t),data))
	{
		return Program_Write(pDevPivotalSet->Boot0FlagAddr, sizeof(Boot0Info_t), (u8*)&bootflag);
	}
	return (FALSE);
}

/*******************************************************************************
�� �� ����	Updata_Channel
����˵���� 	����433ͨ��
��	  ���� 	ch:		�޸ĺ���ŵ�ֵ
�� �� ֵ��	TRUE/FLASE
*******************************************************************************/
u8 Updata_Channel(u8 ch)
{
	DevPivotalSet_t* pDevPivotalSet = GetDevPivotalSet();
	RadioPara_t* pRadioPara = (RadioPara_t*)(pDevPivotalSet->DevRadioParaAddr);
	RADIO_PARA.channel = ch ;
	
	for (;(u32)pRadioPara < DevPartTab->user_app_start_addr; pRadioPara++)
	{
		if (*(u32*)pRadioPara == Flash_DefaultValue())		//����û���޸�
		{			
			return Program_Write((u32)pRadioPara, sizeof(RadioPara_t), (u8*)&RADIO_PARA); //�����޸Ľ��
		}
		Program_Read((u32)pRadioPara + offsetof(RadioPara_t,Reserve),\
					 sizeof(RadioPara_t) - offsetof(RadioPara_t,Reserve),\
					 (u8*)RADIO_PARA.Reserve);
	}
	Program_Read(pDevPivotalSet->DevSoftInfoAddr, sizeof(Soft_t), (u8*)&IAP_FLAG);
	Program_Erase(pDevPivotalSet->DevSoftInfoAddr, DevPartTab->user_app_start_addr - DevPartTab->boot1_flag_start_addr);//�������	
	Program_Write(pDevPivotalSet->DevSoftInfoAddr, sizeof(Soft_t), (u8*)&IAP_FLAG);	//����д��
	
	return Updata_Channel(ch);
}

/*******************************************************************************
�� �� ����	Channel_Get
����˵���� 	��ȡ433ͨ��ֵ 
��	  ���� 	��
�� �� ֵ��	433ͨ��ֵ
*******************************************************************************/
u8 Channel_Get(void)
{
	DevPivotalSet_t* pDevPivotalSet = GetDevPivotalSet();
	RadioPara_t* pRadioPara = (RadioPara_t*)(pDevPivotalSet->DevRadioParaAddr);
	
	for (;(u32)pRadioPara < DevPartTab->user_app_start_addr; pRadioPara++)
	{
		if (*(u32*)pRadioPara == Flash_DefaultValue())		//����û���޸�
		{
			break;
		}
	}
	pRadioPara--;				//ָ��ָ����Ч�ռ�
	if (((pRadioPara->channel) >= CHANNEL_MIN)&&\
		((pRadioPara->channel) <= CHANNEL_MAX)
		)
	{
		return (pRadioPara->channel);	//�������ͨ������ֵ
	}	
	return 0x53;		//����Ĭ��ͨ��ֵ
}

/**************************Copyright BestFu 2014-05-14*************************/
