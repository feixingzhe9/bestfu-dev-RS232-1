/***************************Copyright BestFu 2014-05-14*************************
文	件：	Updata.c
说	明：	升级数据处理函数
编	译：	Keil uVision4 V4.54.0.0
版	本：	v1.0
编	写：	Unarty
日	期：	2013-11-24
修　改：	2015.01.14 Unarty 添加程序空间核对，防止空间越界导致设备升级失败或死机
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
Boot0Info_t bootflag; 		//bootloader标志
RadioPara_t RADIO_PARA ; 	//433通信参数
u8 UPDATA_LEVEL = 0xEF;     //更新完成级别 F0/F1/F2/F3/F4
u8 UpgradeAim = 0;			//记录升级目标
UpgradePre_t UpgradePre = {&bootflag};

//Private function  declarartiuon
static u8 UpgradeReady(UpgradePre_t* pUpgradePre);

/*******************************************************************************
函 数 名：  void UpgradePreStage(UnitPara_t *pData)
功能说明：  升级准备F0阶段
参	  数：  data[0]: 操作子命令
			data[1]: 升级对象
			data[2~3]: 设备类型
			data[4~7]: 升级文件的总大小
			data[8]: 每个升级包的大小
			data[9~12]: 升级软件的版本号
返 回 值：  返回升级状态
*******************************************************************************/
u32 UpgradePreStage(UnitPara_t *pData)
{
	u32 result = 0u;
	u32 data = BOOTLOADER_1_JUMP_FLAG;
	DevPivotalSet_t* pDevPivotalSet = GetDevPivotalSet();
	
	if(0xF0 == UPDATA_LEVEL)
	{
		if(UPGRADE_BOOTLOADER == pData->data[1])	//检测升级目标,是否是升级bootloader程序
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
			//设备跳转到bootloader1运行
			Program_WriteEx(pDevPivotalSet->DevSoftInfoAddr,sizeof(data),(u8*)&data);
			JumpToCode(DevPartTab->boot1_start_addr);
			return (MsgResult_t)(0);			//引导程序不能升级引导程序
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
函 数 名：  UpgradeReady
功能说明：  升级准备
参	  数：  pUpgradePre：升级准备的存储指针
返 回 值：  准备结果 TRUE(完成）/FALSE(失败）
*******************************************************************************/
static u8 UpgradeReady(UpgradePre_t* pUpgradePre)
{
    u32 usersize = 0u;
	DevPivotalSet_t* pDevPivotalSet = GetDevPivotalSet();
	
	if (pUpgradePre->packetsize & 0x03)	//如果包的大小不是按4字节对齐
	{
		return PACKET_SIZE_ALIGN_ERR;
	}
	usersize = (DevPartTab->boot1_flag_start_addr - DevPartTab->boot1_start_addr);
	if (pUpgradePre->pBootInfo->appSize > usersize)	//如果应用空间大于可用空间大小
	{
		return APP_SIZE_BOUND_ERR;					//升级程序大小越界
	}	
	memset(pUpgradePre->data, 0, sizeof(UpgradePre_t) - offsetof(UpgradePre_t,data));
	Program_WriteEx(pDevPivotalSet->Boot0FlagAddr, sizeof(bootflag.jumpFlag), (u8*)&bootflag.jumpFlag);
	return Program_Erase(DevPartTab->boot1_start_addr, pUpgradePre->pBootInfo->appSize);  //对应存储空间清零
}

/******************************************************************************* 
函 数 名：  Updata_PackProcess
功能说明：  升级数据包处理
参	  数：  id:		对应包的ID
			len:	包内数据长度
			data:	数据内容
返 回 值：  FALSE/TRUE
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
	        UpgradePre.data[id >> 5] |= (1<<(id & 0x1f)); //接收包ID置1
	    }
		else
		{
			return FALSE;
		}
	}
	return TRUE;
}

/*******************************************************************************
函 数 名：	Updata_PackCheck
功能说明： 	升级数据包核对
参	  数： 	*len：	缺包数据长度
			*data:  缺包ID
返 回 值：	TRUE/FLASE
*******************************************************************************/
u8 Updata_PackCheck(u8 *num, u16 *data)
{
    u32 i;

    for (i = 0, *num = 0; i < UpgradePre.packetnum; i++)
	{
        if (!(UpgradePre.data[i >> 5]&(1 << (i & 0x1f))))//包未收到
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
函 数 名：	Updata_Calibrate
功能说明： 	设备升级结果校验
参	  数： 	len:	数据长度
			crc:	校验码
返 回 值：	TRUE/FLASE
*******************************************************************************/
u8 Updata_Calibrate(u32 len, u16 crc)
{   
	crc -= ChkCrcValue((u8*)(DevPartTab->boot1_start_addr), len);
	
    return (crc ? FALSE : TRUE);
}

/*******************************************************************************
函 数 名：	Updata_Flag
功能说明： 	升级标识
参	  数： 	无
返 回 值：	TRUE/FLASE
*******************************************************************************/
u8 Updata_Flag(void)
{
	u8 data[sizeof(FlashEncrypt_t) + sizeof(DeviceIDSet_t)];	
	DevPivotalSet_t* pDevPivotalSet = GetDevPivotalSet();
	
	bootflag.jumpFlag	= Flash_DefaultValue();
	Program_Read(pDevPivotalSet->DevResetParaAddr,sizeof(FlashEncrypt_t) + sizeof(DeviceIDSet_t),data);
	Program_Erase(DevPartTab->boot0_flag_start_addr, DevPartTab->boot1_start_addr - DevPartTab->boot0_flag_start_addr);	//修改Flag区域
	if(Program_Write(pDevPivotalSet->DevResetParaAddr,sizeof(FlashEncrypt_t) + sizeof(DeviceIDSet_t),data))
	{
		return Program_Write(pDevPivotalSet->Boot0FlagAddr, sizeof(Boot0Info_t), (u8*)&bootflag);
	}
	return (FALSE);
}

/*******************************************************************************
函 数 名：	Updata_Channel
功能说明： 	更新433通道
参	  数： 	ch:		修改后的信道值
返 回 值：	TRUE/FLASE
*******************************************************************************/
u8 Updata_Channel(u8 ch)
{
	DevPivotalSet_t* pDevPivotalSet = GetDevPivotalSet();
	RadioPara_t* pRadioPara = (RadioPara_t*)(pDevPivotalSet->DevRadioParaAddr);
	RADIO_PARA.channel = ch ;
	
	for (;(u32)pRadioPara < DevPartTab->user_app_start_addr; pRadioPara++)
	{
		if (*(u32*)pRadioPara == Flash_DefaultValue())		//数据没有修改
		{			
			return Program_Write((u32)pRadioPara, sizeof(RadioPara_t), (u8*)&RADIO_PARA); //返回修改结果
		}
		Program_Read((u32)pRadioPara + offsetof(RadioPara_t,Reserve),\
					 sizeof(RadioPara_t) - offsetof(RadioPara_t,Reserve),\
					 (u8*)RADIO_PARA.Reserve);
	}
	Program_Read(pDevPivotalSet->DevSoftInfoAddr, sizeof(Soft_t), (u8*)&IAP_FLAG);
	Program_Erase(pDevPivotalSet->DevSoftInfoAddr, DevPartTab->user_app_start_addr - DevPartTab->boot1_flag_start_addr);//清空数据	
	Program_Write(pDevPivotalSet->DevSoftInfoAddr, sizeof(Soft_t), (u8*)&IAP_FLAG);	//数据写入
	
	return Updata_Channel(ch);
}

/*******************************************************************************
函 数 名：	Channel_Get
功能说明： 	获取433通道值 
参	  数： 	无
返 回 值：	433通道值
*******************************************************************************/
u8 Channel_Get(void)
{
	DevPivotalSet_t* pDevPivotalSet = GetDevPivotalSet();
	RadioPara_t* pRadioPara = (RadioPara_t*)(pDevPivotalSet->DevRadioParaAddr);
	
	for (;(u32)pRadioPara < DevPartTab->user_app_start_addr; pRadioPara++)
	{
		if (*(u32*)pRadioPara == Flash_DefaultValue())		//数据没有修改
		{
			break;
		}
	}
	pRadioPara--;				//指针指向有效空间
	if (((pRadioPara->channel) >= CHANNEL_MIN)&&\
		((pRadioPara->channel) <= CHANNEL_MAX)
		)
	{
		return (pRadioPara->channel);	//返回最后通道设置值
	}	
	return 0x53;		//返回默认通道值
}

/**************************Copyright BestFu 2014-05-14*************************/
