/***************************Copyright BestFu 2014-05-14*************************
��	����    Scene&Group.c
˵	����    ����������غ���
��	�룺    Keil uVision4 V4.54.0.0
��	����    v1.0
��	д��    Unarty
��	�ڣ�    2014.06.26 
�ޡ��ģ�	����
*******************************************************************************/
#include "Scene&Group.h"
#include "UserData.h"
#include "object.h"
#include "SysHard.h"
#include "EEPROM.h"
#include <stdlib.h>

//Variable declaration


/*******************************************************************************
�� �� ��:  	Set_Scene_Attr
����˵��:  	���ó���
��    ��:  	data[0]: Ŀ�굥Ԫ
			data[1]: ��������
�� �� ֵ:  	��Ϣִ�н��
*******************************************************************************/
MsgResult_t Set_Scene_Attr(UnitPara_t *pData)
{
	u32 valArea;
	u8 area, i;

#if (FLASH_ENCRYPTION_EN > 0u)
	u32 temp =0x00;	
	temp = DecryptionFlash();		//���Ӵ������ jay add 2015.12.07
	pData->len >>= temp;
#endif
	area = Check_Area(&pData->data[1], &valArea);		//��ȡ�����ַ	
#if (FLASH_ENCRYPTION_EN > 0u)	
	area += temp;
#endif
	
	if (pData->len < 3)				//�������ȴ���
	{
		return PARA_LEN_ERR;
	}
#if (FLASH_ENCRYPTION_EN > 0u)	
	else if (((u8)(pData->data[0] + 1 + (u8)temp) > UnitCnt_Get())	//��Ԫ�Ų���ȷ	��ע�⣺���ָ��Ϊ��0xFF, ���Լ�1�������㣬����������
			|| ((u8)(pData->data[5] + 1 + (u8)temp) > AREA_SCENE_NUM)//�����Ų���ȷ
			)
#else
	else if (((u8)(pData->data[0] + 1) > UnitCnt_Get())	//��Ԫ�Ų���ȷ	��ע�⣺���ָ��Ϊ��0xFF, ���Լ�1�������㣬����������
			|| ((u8)(pData->data[5] + 1) > AREA_SCENE_NUM)//�����Ų���ȷ
			)	
#endif
	{
		return PARA_MEANING_ERR;
	}
	
	if (pData->data[7] == 1)	//����һ������
	{
		if (0 == pData->data[0] || CLEAR == pData->data[0] || CLEAR == pData->data[5])		//�������ݴ���
		{
			return PARA_MEANING_ERR;
		}
		else if (valArea&(1<<pData->data[0]))//��Ԫ���ڴ�����
		{
			*(u8*)&pData->data[8] = pData->len - 9;		// ��ȡ������������
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
			if (pData->data[8] < SCENE_DATA_SPACE3)	//�ռ䳤�ȺϷ�
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
	else if (pData->data[7] == 0) 			//ɾ������
	{		
		if (CLEAR == pData->data[0])		//���е�Ԫ
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
			if ((0 == area)&&(CLEAR == pData->data[5]))					//��������
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
�� �� ��:  	Get_Scene_Attr
����˵��:  	��ȡ��
��    ��:  	data[0]: Ŀ�굥Ԫ
			data[1]: ��������
			rLen:	���ز�������
			rpara:	���ز�������
�� �� ֵ:  	��Ϣִ�н��
*******************************************************************************/
MsgResult_t Get_Scene_Attr(UnitPara_t *pData, u8 *rlen, u8 *rpara)
{
	u8 	area;
	u32 valArea;
	
	area = Check_Area(&pData->data[1], &valArea);		//��ȡ�����ַ
	
	if (pData->len < 3)	//�������ȴ���
	{
		return PARA_LEN_ERR;
	}
	else if ((pData->data[0] == 0)||(pData->data[0] >= UnitCnt_Get())	//��Ԫ�Ŵ���	
			 || (*(u16*)&pData->data[5] > AREA_SCENE_NUM)				//����Ŵ���
			)
	{
		return PARA_MEANING_ERR;
	}
	else if (valArea&(1<<pData->data[0]))//��Ԫ���ڴ�����
	{	
#if (SCENCETIME_EN > 0u)
		SceneEEPROM_GetEx(pData->data[0], area, pData->data[5], rpara);
#else
		SceneEEPROM_Get(pData->data[0], area, pData->data[5], rpara);	//��ȡ����ű�ʶ
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
�� �� ��:  	Set_Group_Attr
����˵��:  	������
��    ��:  	data[0]: Ŀ�굥Ԫ
			data[1]: ��������
�� �� ֵ:  	��Ϣִ�н��
*******************************************************************************/
MsgResult_t Set_Group_Attr(UnitPara_t *pData)
{
	u32 valArea;
	u8 area, i;
	
#if (FLASH_ENCRYPTION_EN > 0u)	
	u32 temp = 0x00;
	temp = DecryptionFlash();  						//���Ӵ������ jay add 2015.12.07
	pData->len += temp;
#endif
	area = Check_Area(&pData->data[1], &valArea);		//��ȡ�����ַ
	
#if (FLASH_ENCRYPTION_EN > 0u)
	area |= temp;
#endif
	
	if (pData->len < 3)	//�������ȴ���
	{
		return PARA_LEN_ERR;
	}
#if (FLASH_ENCRYPTION_EN > 0u)	
	else if (((u8)(pData->data[0] + 1 + (u8)temp) > UnitCnt_Get())	//��Ԫ�Ų���ȷ	��ע�⣺���ָ��Ϊ��0xFF, ���Լ�1�������㣬����������
			|| ((u8)(((*(u16*)&pData->data[5]) + 1) + (u8)temp) > AREA_GROUP_NUM)//�����Ų���ȷ
			)
#else
	else if (((u8)(pData->data[0] + 1) > UnitCnt_Get())	//��Ԫ�Ų���ȷ	��ע�⣺���ָ��Ϊ��0xFF, ���Լ�1�������㣬����������
			|| ((u8)((*(u16*)&pData->data[5]) + 1) > AREA_GROUP_NUM)//�����Ų���ȷ
			)	
#endif
	{
		return PARA_MEANING_ERR;  
	}
	
	if (pData->data[7] == 1)	//����һ����
	{
		if (0 == pData->data[0] || CLEAR == pData->data[0] || CLEAR == pData->data[5])		//�������ݴ���
		{
			return PARA_MEANING_ERR;
		}
		
		else if (valArea&(1<<pData->data[0]))//��Ԫ���ڴ�����
		{
			GroupEEPROM_Set(pData->data[0], area, pData->data[5]);
		}
		else
		{
			return AREA_ERR;
		}
	}
	else if (pData->data[7] == 0) //ɾ����
	{			
		if (CLEAR == pData->data[0])		//���е�Ԫ
		{
			if ((0 == area) && (CLEAR == pData->data[5]))		//ɾ�����е�Ԫ����������,������
			{
				GroupEEPROM_Init();
			}
			else							//ȷ������
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
		else if (valArea&(1<<pData->data[0]))	//��Ԫ������ȷ
		{
			if ((0 == area)	&& (CLEAR == pData->data[5])) //ɾ���ض���Ԫ�����������г���
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
�� �� ��:  	Get_Group_Attr
����˵��:  	��ȡ��
��    ��:  	pData->unit: Ŀ�굥Ԫ
			pData->cmd
			pData->len: ��������
			rLen:	���ز�������
			rpara:	���ز�������
�� �� ֵ:  	��Ϣִ�н��
*******************************************************************************/
MsgResult_t Get_Group_Attr(UnitPara_t *pData, u8 *rlen, u8 *rpara)
{
	u8 	area;
	u32 valArea;
	
	area = Check_Area(&pData->data[1], &valArea);		//��ȡ�����ַ
	
	if (pData->len < 3)	//�������ȴ���
	{
		return PARA_LEN_ERR;
	}
	else if ((pData->data[0] == 0)||(pData->data[0] >= UnitCnt_Get())	//��Ԫ�Ŵ���	
			 || (pData->data[5] > AREA_GROUP_NUM)						//����Ŵ���
			)
	{
		return PARA_MEANING_ERR;
	}
	else if (valArea&(1 << pData->data[0]))//��Ԫ���ڴ�����
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
