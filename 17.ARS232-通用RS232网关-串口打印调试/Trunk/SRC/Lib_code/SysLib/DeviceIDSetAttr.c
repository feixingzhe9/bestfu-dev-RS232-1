/***************************Copyright BestFu 2014-05-14*************************
��	����	DeviceIDSetAttr.c
˵	����	�����豸��ַ���ļ�
��	�룺	uVision V5.12.0.0
��	����	v1.0
��	д��	Unarty
��	�ڣ�	2013-11-22
��  ��:     ����
*******************************************************************************/
#include "BF_type.h"
#include "flash.h"
#include "Program.h"
#include "EEPROM.h"
#include "Thread.h"
#include "SysHard.h"
#include "DeviceIDSetAttr.h"
#include "DevPartTab.h"


#if (DEVICE_ADDR_RESET_FEATURE_EN > 0u)	
/*******************************************************************************
�� �� ��:  	MsgResult_t Get_ResetAddr_Attr(UnitPara_t *pData, u8 *rlen, u8 *rpara)
����˵��:  	��ȡ�����ַ
��    ��:  	data[0]: Ŀ�굥Ԫ
			data[1]: ��������
			rLen:	���ز�������
			rpara:	���ز�������
�� �� ֵ:  	��Ϣִ�н��
*******************************************************************************/
MsgResult_t Get_ResetAddr_Attr(UnitPara_t *pData, u8 *rlen, u8 *rpara)
{
	DevPivotalSet_t* pDevPivotalSet = GetDevPivotalSet();
	DeviceIDSet_t* pDeviceIDSet = (DeviceIDSet_t*)(pDevPivotalSet->DevResetParaAddr);
	
	if(DEVICE_ID_IS_OK((pDeviceIDSet->SetAddrFlag),\
		(pDeviceIDSet->DeviceID),(pDeviceIDSet->DeviceIDInver)))	//��������õ��µ�ַ�Ϸ�
    {
        *(u32*)rpara = pDeviceIDSet->DeviceID;
    }
    else
    {
        *(u32*)rpara = 0xffffffff;
    }
	Get_UniqueDeviceID((u8*)(rpara+4));	
    *rlen = 16;                          						//4bytes�µ�ַ + 12bytes Chip ID��ַ 
	return COMPLETE;
}

/*******************************************************************************
�� �� ����  MsgResult_t Set_ResetAddr_Attr(UnitPara_t *pData)
����˵����  д�������ַ
��	  ����  data[0]: Ŀ�굥Ԫ
			data[1]: ��������
�� �� ֵ:  	��Ϣִ�н��
			0x01(COMPLETE):��ʾ���óɹ�
			0x02:��ʾû���ù�������ʧ��
			0x03:��ʾ�����ù�������������
*******************************************************************************/
MsgResult_t Set_ResetAddr_Attr(UnitPara_t *pData)
{
	u8 Tab_Tem[20]= {0};
    u32 DeviceID_Tem = 0xffffffff;  
	DevPivotalSet_t* pDevPivotalSet = GetDevPivotalSet();
	DeviceIDSet_t* pDeviceIDSet = (DeviceIDSet_t*)(pDevPivotalSet->DevResetParaAddr);
	
    if (pData->len < 8)                                     	//�������Ȳ��Ϸ�
	{
		return PARA_LEN_ERR;
	}
	if(DEVICE_ID_IS_OK(pDeviceIDSet->SetAddrFlag,\
		pDeviceIDSet->DeviceID,pDeviceIDSet->DeviceIDInver))	//��������õ��µ�ַ�Ϸ�	
    {
		if(pDeviceIDSet->DeviceID == (*(u32*)(&pData->data[0])))//�������ַ�����ù�����ֱ�ӷ��سɹ�
		{
			return COMPLETE; 
		}
    }
    else                                                    //����ǰ����STM32ԭʼ�豸��ַ,У�鵱ǰ�豸��ַ����λ�����͵ĵ�ǰУ���ַ�Ƿ���ͬ 
    {
        if(DEVICE_ID != (*(u32*)(&pData->data[4])))
        {
            return PARA_MEANING_ERR;                        //��������          
        }
    }
#if (FLASH_ENCRYPTION_EN > 0u)
	*(u32*)(Tab_Tem + sizeof(DeviceIDSet_t))	= ENCRYPTION_FLAG;
	*(u32*)(Tab_Tem + sizeof(DeviceIDSet_t) + offsetof(FlashEncrypt_t,EncryptFlashData))  = FlashEncryptionFunc((*(u32*)(&pData->data[0])));
#endif
	/*�����ַ�������ݸ�ʽΪ->�����ַ���(2bytes 0xaa55) + �����ַ(4bytes) + �����ַ����(4bytes) */       
	*(u32*)(Tab_Tem + 0x00) = RE_DEVICE_ID_FLAG;
	*(u32*)(Tab_Tem + offsetof(DeviceIDSet_t,DeviceID)) 		= (*(u32*)(&pData->data[0]));
	*(u32*)(Tab_Tem + offsetof(DeviceIDSet_t,DeviceIDInver))	=~(*(u32*)(&pData->data[0]));
	if(Program_WriteEx(pDevPivotalSet->DevResetParaAddr,sizeof(Tab_Tem), (u8 *)(Tab_Tem)))
	{
		DeviceID_Tem = (*(u32*)(&pData->data[0]));
		WriteDataToEEPROM(SYSDATA_START_ADD + offsetof(SysData_t, deviceID), sizeof(u32), (u8*)&DeviceID_Tem);
		Thread_Login(ONCEDELAY,0,1000,HardID_Change);
		return COMPLETE;                                    //��ʾû���ù������óɹ�
	}
	else
	{
		return ((MsgResult_t)(0x02));                       //��ʾû���ù�������ʧ��
	}
}
#endif


/**************************Copyright BestFu 2014-05-14*************************/
