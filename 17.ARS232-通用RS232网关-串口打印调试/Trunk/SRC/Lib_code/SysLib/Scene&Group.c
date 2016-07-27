/***************************Copyright BestFu 2014-05-14*************************
文	件：    Scene&Group.c
说	明：    场景与组相关函数
编	译：    Keil uVision4 V4.54.0.0
版	本：    v1.0
编	写：    Unarty
日	期：    2014.06.26 
修　改：	暂无
*******************************************************************************/
#include "Scene&Group.h"
#include "UserData.h"
#include "object.h"
#include "SysHard.h"
#include "EEPROM.h"
#include <stdlib.h>

//Variable declaration


/*******************************************************************************
函 数 名:  	Set_Scene_Attr
功能说明:  	设置场景
参    数:  	data[0]: 目标单元
			data[1]: 参数长度
返 回 值:  	消息执行结果
*******************************************************************************/
MsgResult_t Set_Scene_Attr(UnitPara_t *pData)
{
	u32 valArea;
	u8 area, i;

#if (FLASH_ENCRYPTION_EN > 0u)
	u32 temp =0x00;	
	temp = DecryptionFlash();		//增加代码混淆 jay add 2015.12.07
	pData->len >>= temp;
#endif
	area = Check_Area(&pData->data[1], &valArea);		//获取区域地址	
#if (FLASH_ENCRYPTION_EN > 0u)	
	area += temp;
#endif
	
	if (pData->len < 3)				//参数长度错误
	{
		return PARA_LEN_ERR;
	}
#if (FLASH_ENCRYPTION_EN > 0u)	
	else if (((u8)(pData->data[0] + 1 + (u8)temp) > UnitCnt_Get())	//单元号不正确	（注意：清除指令为：0xFF, 所以加1后变成了零，符合条件）
			|| ((u8)(pData->data[5] + 1 + (u8)temp) > AREA_SCENE_NUM)//场景号不正确
			)
#else
	else if (((u8)(pData->data[0] + 1) > UnitCnt_Get())	//单元号不正确	（注意：清除指令为：0xFF, 所以加1后变成了零，符合条件）
			|| ((u8)(pData->data[5] + 1) > AREA_SCENE_NUM)//场景号不正确
			)	
#endif
	{
		return PARA_MEANING_ERR;
	}
	
	if (pData->data[7] == 1)	//增加一个场景
	{
		if (0 == pData->data[0] || CLEAR == pData->data[0] || CLEAR == pData->data[5])		//参数内容错误
		{
			return PARA_MEANING_ERR;
		}
		else if (valArea&(1<<pData->data[0]))//单元属于此区域
		{
			*(u8*)&pData->data[8] = pData->len - 9;		// 获取场景参数长度
#if (SCENCETIME_EN > 0u)			
			if((0xA0 == pData->cmd)&&(pData->data[8] < SCENE_DATA_SPACE3))
			{
				return ((MsgResult_t)SceneEEPROM_Set(pData->data[0] , area, pData->data[5], (u8*)&pData->data[8]));
			}
			else if((0xAE == pData->cmd)&&(pData->data[8] < SCENE_DATA_SPACE3 + SCENE_TIME_DATA_SPACE3))
			{
				return (MsgResult_t)SceneEEPROM_SetEx(pData->data[0], area, pData->data[5], (u8*)&pData->data[8]);
			}
#else
			if (pData->data[8] < SCENE_DATA_SPACE3)	//空间长度合法
			{
				return ((MsgResult_t)SceneEEPROM_Set(pData->data[0] , area, pData->data[5], (u8*)&pData->data[8]));
			}
#endif			
			else
			{
				return PARA_LEN_ERR;
			}			
		}
		else
		{
			return AREA_ERR;
		}
	}
	else if (pData->data[7] == 0) 			//删除场景
	{		
		if (CLEAR == pData->data[0])		//所有单元
		{
			if ((0 == area)&&(CLEAR == pData->data[5]))
			{
				SceneEEPROM_Init();
			}
			else
			{
				for (i = 1; i < UnitCnt_Get(); i++)
				{
					if (valArea&(1<<i))
					{						
						SceneEEPROM_Clr(i, area, pData->data[5]);
					}
				}
			}
		}
		else if (valArea&(1<<pData->data[0]))
		{
			if ((0 == area)&&(CLEAR == pData->data[5]))					//所有区域
			{
				SceneEEPROMUnit_Init(pData->data[0]);
			}
			else
			{
				SceneEEPROM_Clr(pData->data[0], area, pData->data[5]);
			}
		}
		else
		{
			return AREA_ERR;
		}
	}
	
	return COMPLETE;	
}

/*******************************************************************************
函 数 名:  	Get_Scene_Attr
功能说明:  	获取组
参    数:  	data[0]: 目标单元
			data[1]: 参数长度
			rLen:	返回参数长度
			rpara:	返回参数内容
返 回 值:  	消息执行结果
*******************************************************************************/
MsgResult_t Get_Scene_Attr(UnitPara_t *pData, u8 *rlen, u8 *rpara)
{
	u8 	area;
	u32 valArea;
	
	area = Check_Area(&pData->data[1], &valArea);		//获取区域地址
	
	if (pData->len < 3)	//参数长度错误
	{
		return PARA_LEN_ERR;
	}
	else if ((pData->data[0] == 0)||(pData->data[0] >= UnitCnt_Get())	//单元号错误	
			 || (*(u16*)&pData->data[5] > AREA_SCENE_NUM)				//区域号错误
			)
	{
		return PARA_MEANING_ERR;
	}
	else if (valArea&(1<<pData->data[0]))//单元属于此区域
	{	
#if (SCENCETIME_EN > 0u)
		SceneEEPROM_GetEx(pData->data[0], area, pData->data[5], rpara);
#else
		SceneEEPROM_Get(pData->data[0], area, pData->data[5], rpara);	//获取到组号标识
#endif
		*rlen = rpara[0] + 1;
		if(rpara[0] > 0u)
		{
			rpara[0] = 1;
		}
		else
		{
			rpara[0] = 0;
		}	
//		memmove(&rpara[0], &rpara[1], *rlen);
	}
	else
	{
		*rlen = 0;
		return AREA_ERR;
	}
	
	return COMPLETE;
}

/*******************************************************************************
函 数 名:  	Set_Group_Attr
功能说明:  	设置组
参    数:  	data[0]: 目标单元
			data[1]: 参数长度
返 回 值:  	消息执行结果
*******************************************************************************/
MsgResult_t Set_Group_Attr(UnitPara_t *pData)
{
	u32 valArea;
	u8 area, i;
	
#if (FLASH_ENCRYPTION_EN > 0u)	
	u32 temp = 0x00;
	temp = DecryptionFlash();  						//增加代码混淆 jay add 2015.12.07
	pData->len += temp;
#endif
	area = Check_Area(&pData->data[1], &valArea);		//获取区域地址
	
#if (FLASH_ENCRYPTION_EN > 0u)
	area |= temp;
#endif
	
	if (pData->len < 3)	//参数长度错误
	{
		return PARA_LEN_ERR;
	}
#if (FLASH_ENCRYPTION_EN > 0u)	
	else if (((u8)(pData->data[0] + 1 + (u8)temp) > UnitCnt_Get())	//单元号不正确	（注意：清除指令为：0xFF, 所以加1后变成了零，符合条件）
			|| ((u8)(((*(u16*)&pData->data[5]) + 1) + (u8)temp) > AREA_GROUP_NUM)//场景号不正确
			)
#else
	else if (((u8)(pData->data[0] + 1) > UnitCnt_Get())	//单元号不正确	（注意：清除指令为：0xFF, 所以加1后变成了零，符合条件）
			|| ((u8)((*(u16*)&pData->data[5]) + 1) > AREA_GROUP_NUM)//场景号不正确
			)	
#endif
	{
		return PARA_MEANING_ERR;  
	}
	
	if (pData->data[7] == 1)	//增加一个组
	{
		if (0 == pData->data[0] || CLEAR == pData->data[0] || CLEAR == pData->data[5])		//参数内容错误
		{
			return PARA_MEANING_ERR;
		}
		
		else if (valArea&(1<<pData->data[0]))//单元属于此区域
		{
			GroupEEPROM_Set(pData->data[0], area, pData->data[5]);
		}
		else
		{
			return AREA_ERR;
		}
	}
	else if (pData->data[7] == 0) //删除组
	{			
		if (CLEAR == pData->data[0])		//所有单元
		{
			if ((0 == area) && (CLEAR == pData->data[5]))		//删除所有单元，所有区域,所有组
			{
				GroupEEPROM_Init();
			}
			else							//确定区域
			{
				for (i = 1; i < UnitCnt_Get(); i++)
				{
					if (valArea&(1<<i))
					{
						GroupEEPROM_Clr(i, area, pData->data[5]);
					}
				}
			}
		}
		else if (valArea&(1<<pData->data[0]))	//单元区域正确
		{
			if ((0 == area)	&& (CLEAR == pData->data[5])) //删除特定单元所有区域所有场景
			{
				GroupEEPROMUnit_Init(pData->data[0]);
			}
			else
			{
				GroupEEPROM_Clr(pData->data[0], area, pData->data[5]);
			}
		}
		else
		{
			return AREA_ERR;
		}
	}
	
	return COMPLETE;	
}

/*******************************************************************************
函 数 名:  	Get_Group_Attr
功能说明:  	获取组
参    数:  	pData->unit: 目标单元
			pData->cmd
			pData->len: 参数长度
			rLen:	返回参数长度
			rpara:	返回参数内容
返 回 值:  	消息执行结果
*******************************************************************************/
MsgResult_t Get_Group_Attr(UnitPara_t *pData, u8 *rlen, u8 *rpara)
{
	u8 	area;
	u32 valArea;
	
	area = Check_Area(&pData->data[1], &valArea);		//获取区域地址
	
	if (pData->len < 3)	//参数长度错误
	{
		return PARA_LEN_ERR;
	}
	else if ((pData->data[0] == 0)||(pData->data[0] >= UnitCnt_Get())	//单元号错误	
			 || (pData->data[5] > AREA_GROUP_NUM)						//区域号错误
			)
	{
		return PARA_MEANING_ERR;
	}
	else if (valArea&(1 << pData->data[0]))//单元属于此区域
	{
		*rlen = 1;
		*rpara = GroupEEPROM_Get(pData->data[0], area, pData->data[5]);
	}
	else
	{
		*rlen = 0;
		return AREA_ERR;
	}
	
	return COMPLETE;
}	
/**************************Copyright BestFu 2014-05-14*************************/	
