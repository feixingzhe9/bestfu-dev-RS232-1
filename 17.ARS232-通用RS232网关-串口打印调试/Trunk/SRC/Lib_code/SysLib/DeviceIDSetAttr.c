/***************************Copyright BestFu 2014-05-14*************************
文	件：	DeviceIDSetAttr.c
说	明：	重设设备地址的文件
编	译：	uVision V5.12.0.0
版	本：	v1.0
编	写：	Unarty
日	期：	2013-11-22
修  改:     暂无
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
函 数 名:  	MsgResult_t Get_ResetAddr_Attr(UnitPara_t *pData, u8 *rlen, u8 *rpara)
功能说明:  	获取重设地址
参    数:  	data[0]: 目标单元
			data[1]: 参数长度
			rLen:	返回参数长度
			rpara:	返回参数内容
返 回 值:  	消息执行结果
*******************************************************************************/
MsgResult_t Get_ResetAddr_Attr(UnitPara_t *pData, u8 *rlen, u8 *rpara)
{
	DevPivotalSet_t* pDevPivotalSet = GetDevPivotalSet();
	DeviceIDSet_t* pDeviceIDSet = (DeviceIDSet_t*)(pDevPivotalSet->DevResetParaAddr);
	
	if(DEVICE_ID_IS_OK((pDeviceIDSet->SetAddrFlag),\
		(pDeviceIDSet->DeviceID),(pDeviceIDSet->DeviceIDInver)))	//如果已设置的新地址合法
    {
        *(u32*)rpara = pDeviceIDSet->DeviceID;
    }
    else
    {
        *(u32*)rpara = 0xffffffff;
    }
	Get_UniqueDeviceID((u8*)(rpara+4));	
    *rlen = 16;                          						//4bytes新地址 + 12bytes Chip ID地址 
	return COMPLETE;
}

/*******************************************************************************
函 数 名：  MsgResult_t Set_ResetAddr_Attr(UnitPara_t *pData)
功能说明：  写入重设地址
参	  数：  data[0]: 目标单元
			data[1]: 参数长度
返 回 值:  	消息执行结果
			0x01(COMPLETE):表示设置成功
			0x02:表示没设置过，设置失败
			0x03:表示已设置过，无需再设置
*******************************************************************************/
MsgResult_t Set_ResetAddr_Attr(UnitPara_t *pData)
{
	u8 Tab_Tem[20]= {0};
    u32 DeviceID_Tem = 0xffffffff;  
	DevPivotalSet_t* pDevPivotalSet = GetDevPivotalSet();
	DeviceIDSet_t* pDeviceIDSet = (DeviceIDSet_t*)(pDevPivotalSet->DevResetParaAddr);
	
    if (pData->len < 8)                                     	//参数长度不合法
	{
		return PARA_LEN_ERR;
	}
	if(DEVICE_ID_IS_OK(pDeviceIDSet->SetAddrFlag,\
		pDeviceIDSet->DeviceID,pDeviceIDSet->DeviceIDInver))	//如果已设置的新地址合法	
    {
		if(pDeviceIDSet->DeviceID == (*(u32*)(&pData->data[0])))//若重设地址已设置过，则直接返回成功
		{
			return COMPLETE; 
		}
    }
    else                                                    //若当前运行STM32原始设备地址,校验当前设备地址与上位机发送的当前校验地址是否相同 
    {
        if(DEVICE_ID != (*(u32*)(&pData->data[4])))
        {
            return PARA_MEANING_ERR;                        //参数错误          
        }
    }
#if (FLASH_ENCRYPTION_EN > 0u)
	*(u32*)(Tab_Tem + sizeof(DeviceIDSet_t))	= ENCRYPTION_FLAG;
	*(u32*)(Tab_Tem + sizeof(DeviceIDSet_t) + offsetof(FlashEncrypt_t,EncryptFlashData))  = FlashEncryptionFunc((*(u32*)(&pData->data[0])));
#endif
	/*重设地址功能数据格式为->重设地址标记(2bytes 0xaa55) + 重设地址(4bytes) + 重设地址反码(4bytes) */       
	*(u32*)(Tab_Tem + 0x00) = RE_DEVICE_ID_FLAG;
	*(u32*)(Tab_Tem + offsetof(DeviceIDSet_t,DeviceID)) 		= (*(u32*)(&pData->data[0]));
	*(u32*)(Tab_Tem + offsetof(DeviceIDSet_t,DeviceIDInver))	=~(*(u32*)(&pData->data[0]));
	if(Program_WriteEx(pDevPivotalSet->DevResetParaAddr,sizeof(Tab_Tem), (u8 *)(Tab_Tem)))
	{
		DeviceID_Tem = (*(u32*)(&pData->data[0]));
		WriteDataToEEPROM(SYSDATA_START_ADD + offsetof(SysData_t, deviceID), sizeof(u32), (u8*)&DeviceID_Tem);
		Thread_Login(ONCEDELAY,0,1000,HardID_Change);
		return COMPLETE;                                    //表示没设置过，设置成功
	}
	else
	{
		return ((MsgResult_t)(0x02));                       //表示没设置过，设置失败
	}
}
#endif


/**************************Copyright BestFu 2014-05-14*************************/
