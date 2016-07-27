/***************************Copyright BestFu 2014-05-14*************************
��	����    SysExeAttr.c
˵	����    ϵͳ����������ִ�б����������ʵ��
��	�룺    Keil uVision4 V4.54.0.0
��	����    v2.0
��	д��    Unarty
��	�ڣ�    2014.06.26
�ޡ��ģ�	��ŵ�� 2014.10.22 ������߰����Խӿ� B0��BF
			Unarty 2014.12.01 ɾ��0xCE ��0xCF ���ԣ����˽ӿ���Ϊ����V1.00.00�汾��Ԥ���ģ�������ȫȥ���˽ӿڣ�
*******************************************************************************/
#include "SysExeAttr.h"
#include "UnitShare_Attr.h"
#include "Scene&Group.h"
#include "FlashAttr.h"
#include "SysTestAttr.h"
#include "Upload.h"
#include "linkWRDC.h"
#include "UserDataDownload.h"
#include "Sign433Attr.h"
#include "FaultManage.h"
#include "DlyAttrProcess.h"
#include "SysHard.h"
#include "Thread.h"
#include "SysTick.h"
#include "DevPartTab.h"
#include "DeviceIDSetAttr.h"
#include "DataEncrypt.h"


/*���Զ�д�б�*/
const AttrExe_st SysAttrTable[] =
{
	{0x00, LEVEL_0, Set_ManyUnit_Attr				, Get_ManyUnit_Attr					},
	{0x90, LEVEL_0, Set_UnitChecking_Attr			, NULL								},
#if (DALAY_EXECUTE_EN > 0u)	
	{0x91, LEVEL_0, Set_ManyUnitEx_Attr	    		, NULL								},	//Jay Add 2016.02.01
#endif	
	{0x92, LEVEL_0, NULL	    					, Get_DeveiceSysClockFeature_Attr	},	//��ȡ�豸ϵͳʱ������
	{0x93, LEVEL_0, Set_DeviceChannelFlag_Attr		, Get_DeviceChannelFlag_Attr		},	//�豸��������Ϣ
	
	{0xA0, LEVEL_0, Set_Scene_Attr					, Get_Scene_Attr					},
	{0xA1, LEVEL_0, Set_Group_Attr					, Get_Group_Attr					},
	{0xA2, LEVEL_0, Set_UnitArea_Attr				, Get_UnitArea_Attr					},
#if (SCENCETIME_EN > 0u)
	{0xAE, LEVEL_0, Set_Scene_Attr					, Get_Scene_Attr					},	//Jay Add 2016.02.01
#endif	
	{0xAF, LEVEL_0, Set_RepeatBroadcast_Attr		, Get_RepeatBroadcast_Attr			},	//��������2015.9.9
	{0xB0, LEVEL_0, Set_Sign433_StartSendFrame_Attr , Get_Sign433_StartSendFrame_Attr	},  //�������Ͳ���
	{0xB1, LEVEL_0, Set_Sign433_StartRecFrame_Attr  , Get_Sign433_StartRecFrame_Attr    },  //�������ղ���
	{0xB2, LEVEL_0, Set_Sign433_RecFrameStream_Attr	, NULL					            },  //���ղ�������֡��
//{0XB3,0XB4 } Ԥ��
#ifndef   STM32L151 		//���ǵ͹��Ĳ�Ʒ	
	{0xBD, LEVEL_0, Set_DelayUpload_Attr			, NULL								},	//������ѯ�ӿ� yanhuan adding 2015/12/31
#endif	
	{0xBE, LEVEL_0, NULL							, Get_DeviceThreshold_Attr			},	//��ȡ�豸ͨ���ż�ֵ yanhuan adding 2015/12/02
	{0xBF, LEVEL_0, Set_Sign433_TestState_Attr		, Get_Sign433_TestState_Attr		},	//�жϲ���
	
	{0xC0, LEVEL_0, NULL							, Get_DeviceType_Attr				},
	{0xC1, LEVEL_0, NULL							, Get_Version_Attr					},
	{0xC2, LEVEL_0, Set_UserID_Attr					, Get_UserID_Attr					},
	{0xC3, LEVEL_0, NULL							, Get_DeviceAddr_Attr				},
	{0xC4, LEVEL_0, Set_CmdAckNum_Attr				, Get_CmdAckNum_Attr				},
	{0xC5, LEVEL_0, Set_RepeatBandAddr_Attr			, Get_RepeatBandAddr_Attr			},
	
	{0xC9, LEVEL_0, Set_UserDataFormat_Attr			, NULL								},
//	{0xCE, LEVEL_0, NULL							, Get_DeviceInfo_Attr				},	//2014.12.01 Unarty��
//	{0xCF, LEVEL_0, Set_DeviceAdd_Attr				, NULL								},
	{0xD1, LEVEL_1, Set_UploadAble_Attr				, Get_UploadAble_Attr				},	//�ϱ�ʹ�� yanhuan adding 2016/01/04
	{0xD2, LEVEL_1, Set_UnitAble_Attr				, Get_UnitAble_Attr					},
	{0xD4, LEVEL_0, Set_UnitType_Attr				, Get_UnitType_Attr					},
	{0xD5, LEVEL_0, Set_CommonType_Attr				, Get_CommonType_Attr				},

	{0xE0, LEVEL_0, Set_Link_Attr					, Get_Link_Attr						},
#if (ALINKTIME_EN > 0)
	{0xE1, LEVEL_0, Set_Link_Time_Attr				, Get_Link_Attr						},	//Ϊ�˼������豸����ȡ�ӿ���0xE0һ��
#endif
	{0xE3, LEVEL_0, Set_DelLinkForUnit_Attr			, NULL								},

	{0xEA, LEVEL_0,	UserDataDownload_Ready			, NULL								},
	{0xEB, LEVEL_0, UserDataFrame_Save				, UserDataFrame_Check				},
#if (DEVICE_ADDR_RESET_FEATURE_EN > 0u)	
	{0xEC, LEVEL_0, Set_ResetAddr_Attr				, Get_ResetAddr_Attr				},  //�����ַ Jay Add 2015.11.13
#endif	
	{0xF4, LEVEL_0, Set_IAPReady_Attr				, Get_IAPReady_Attr					},
	{0xF5, LEVEL_0, Set_IAPPackSave_Attr			, NULL								},
	
	{0xF8, LEVEL_0, Set_433Channel_Attr				, Get_433Channel_Attr				},
	{0xFA, LEVEL_0, HardWDGTest_Attr				, NULL								},
	{0xFB, LEVEL_0, SI4432Test_Attr					, NULL								},
	{0xFD, LEVEL_0, NULL			          		, Get_Eeprom_Info_Attr				},	//EEPROM���ݶ�ȡ�ӿ� yanhuan adding 2015/10/10
	{0xFE, LEVEL_0, Set_FaultFlag_Attr				, Get_FaultFlag_Attr				},
};

/*******************************************************************************
�� �� ��:  Sys_UnitAttrNum
����˵��:  ����ϵͳ��Ԫ��������
��    ��:  ��
�� �� ֵ:  ��
*******************************************************************************/
u8 Sys_UnitAttrNum(void)
{
	return ((sizeof(SysAttrTable)/sizeof(SysAttrTable[0])));
}

/*******************************************************************************
�� �� ��:  Upload_RESTART
����˵��:  �豸�ϱ�����
��    ��:  ��
�� �� ֵ:  ��
*******************************************************************************/
void Upload_RESTART(void)
{
	Upload_Fault(RESTART);	//�ϱ��豸����
}
/*******************************************************************************
�� �� ��:  SysAttr_Init
����˵��:  ϵͳ���Գ�ʼ��
��    ��:  unitID:	��Ԫ��
�� �� ֵ:  ��
*******************************************************************************/
void SysAttr_Init(u8 unitID)
{
	UserData_Init();
	UserDataDownload_Init();
	LinkInit();
	SceneData_Init();
	PropFifoInit();
	EncryptInit((u8*)&gSysData.deviceID); 			//433�źż��ܳ�ʼ��
	Fault_Upload(FAULT_2 , DEV_RESTART , NULL); 	//�豸����������¼���� yanhuan adding 2015/10/10
	Thread_Login(ONCEDELAY , 0 ,1000 , Upload_RESTART); //1��֮���ϱ�����  yanhuan adding 2015/12/02
}

/*******************************************************************************
�� �� ��:  	Get_Version_Attr
����˵��:  	��ȡ�汾��Ϣ
��    ��:  	data[0]: Ŀ�굥Ԫ
			data[1]: ��������
			rLen:	���ز�������
			rpara:	���ز�������
�� �� ֵ:  	��Ϣִ�н��
*******************************************************************************/
MsgResult_t Get_Version_Attr(UnitPara_t *pData, u8 *rlen, u8 *rpara)
{
	u8 *p = NULL; 
	DevPivotalSet_t* pDevPivotalSet = GetDevPivotalSet();
	Soft_t* pSoft = (Soft_t*)(pDevPivotalSet->DevSoftInfoAddr);
	Hard_t* pHard = (Hard_t*)(pDevPivotalSet->DevHardInfoAddr);
	Boot0Info_t* pBoot0Info = (Boot0Info_t*)(pDevPivotalSet->Boot0FlagAddr);
	
	*rlen = 0;
	*(u16*)&rpara[*rlen] = SYS_VERSION;		//ϵͳ��汾
	*rlen += 2;
	*(u32*)&rpara[*rlen] = pSoft->SoftVer;	//����汾
	*rlen += sizeof(pSoft->SoftVer);
	*(u32*)&rpara[*rlen] = (BOOTLOADER_0_JUMP_FLAG == pBoot0Info->jumpFlag)?(0xFFFFFFFF):(pBoot0Info->SoftVer);
	*rlen += sizeof(pBoot0Info->SoftVer);
	*(u32*)&rpara[*rlen] = pHard->hardVer;	//Ӳ���汾
	*rlen += sizeof(pHard->hardVer);
	
	for (p = pHard->str; (*p <= 'Z')&&(*p >= '0'); p++)	//Ӳ��������Ϣ
	{
		rpara[((*rlen)++)] = *p;		
	}
	
	return COMPLETE;
}


/*******************************************************************************
�� �� ��:  	Get_DeviceType_Attr
����˵��:  	��ȡ�豸����
��    ��:  	data[0]: Ŀ�굥Ԫ
			data[1]: ��������
			rLen:	���ز�������
			rpara:	���ز�������
�� �� ֵ:  	��Ϣִ�н��
*******************************************************************************/
MsgResult_t Get_DeviceType_Attr(UnitPara_t *pData, u8 *rlen, u8 *rpara)
{
	DevPivotalSet_t* pDevPivotalSet = GetDevPivotalSet();
	Soft_t* pSoft = (Soft_t*)(pDevPivotalSet->DevSoftInfoAddr);
	
	if (!(pData->len)	//ȫ�ֲ���
		|| (pSoft->deviceType == *(u16*)&pData->data[0])//���豸���Ͳ���
		|| (0xFFFF == *(u16*)&pData->data[0])			//ȫ���Ͳ���   
		)
	{
		*rlen = 0;
		*(u16*)&rpara[*rlen] = pSoft->deviceType;
		*rlen += 2;
		*(u32*)&rpara[*rlen] = pSoft->SoftVer;
		*rlen += 4;

		return COMPLETE;
	}
			
	return CMD_EXE_ERR;
}

/*******************************************************************************
�� �� ����  Set_UserID_Attr
����˵����  �����û���
��	  ����  data[0]: Ŀ�굥Ԫ
			data[1]: ��������
�� �� ֵ:  	��Ϣִ�н��
*******************************************************************************/
MsgResult_t Set_UserID_Attr(UnitPara_t *pData)
{
	if ((pData->len > 4)||((!pData->len)))//�������Ȳ��Ϸ�
	{
		return PARA_LEN_ERR;
	}
	gSysData.userID = *(u32*)&pData->data[0];
	if (SysEEPROMData_Set(sizeof(gSysData.userID) , (u8*)&gSysData.userID) == FALSE)
	{
		return EEPROM_ERR;
	}
	
	return COMPLETE;	
}

/*******************************************************************************
�� �� ��:  	Get_UserID_Attr
����˵��:  	��ȡ�û���
��    ��:  	data[0]: Ŀ�굥Ԫ
			data[1]: ��������
			rLen:	���ز�������
			rpara:	���ز�������
�� �� ֵ:  	��Ϣִ�н��
*******************************************************************************/
MsgResult_t Get_UserID_Attr(UnitPara_t *pData, u8 *rlen, u8 *rpara)
{	
	*(u32*)rpara = gSysData.userID;
	*rlen = sizeof(gSysData.userID);
	
	return COMPLETE;
}

/*******************************************************************************
�� �� ����  Set_DeviceChannelFlag_Attr
����˵����  �����豸������ʶ
��	  ����  data[0]: �豸������Ϣ�ı��
�� �� ֵ:  	��Ϣִ�н��
*******************************************************************************/
MsgResult_t Set_DeviceChannelFlag_Attr(UnitPara_t *pData)
{
	const u32 deviceChannel = *(u32*)&pData->data[1];
	const u8 operateLevel = pData->data[0];
	
	if((!deviceChannel)||(0xFFFFFFFF == deviceChannel))
	{
		return PARA_MEANING_ERR;
	}

	return (MsgResult_t)SetDeviceChannelFlag(deviceChannel, operateLevel);
}

/*******************************************************************************
�� �� ��:  	Get_DeviceChannelFlag_Attr
����˵��:  	��ȡ�豸������ʶ
��    ��:  	data[0]: �豸�豸������ʶ
�� �� ֵ:  	��Ϣִ�н��
*******************************************************************************/
MsgResult_t Get_DeviceChannelFlag_Attr(UnitPara_t *pData, u8 *rlen, u8 *rpara)
{	
	*(u32*)rpara = GetDeviceChannelFlag();
	*rlen = 4;	
	
	return COMPLETE;
}


/*******************************************************************************
�� �� ��:  	Get_DeviceAddr_Attr
����˵��:  	��ȡ�豸�豸��ַ
��    ��:  	data[0]: Ŀ�굥Ԫ
			data[1]: ��������
			rLen:	���ز�������
			rpara:	���ز�������
�� �� ֵ:  	��Ϣִ�н��
*******************************************************************************/
MsgResult_t Get_DeviceAddr_Attr(UnitPara_t *pData, u8 *rlen, u8 *rpara)
{
	*(u32*)rpara = gSysData.deviceID;
	*rlen = sizeof(gSysData.deviceID);
	
	return COMPLETE;
}

/*******************************************************************************
�� �� ����  Set_CmdAckNum_Attr
����˵����  ����ָ��Ӧ�����
��	  ����  data[0]: Ŀ�굥Ԫ
			data[1]: ��������
�� �� ֵ:  	��Ϣִ�н��
*******************************************************************************/
MsgResult_t Set_CmdAckNum_Attr(UnitPara_t *pData)
{
	if ((pData->data[0] > 5)||((!pData->data[0])))//�������ݲ��Ϸ�
	{
		return PARA_LEN_ERR;
	}
	gSysData.sendCnt = pData->data[0];
	if (SysEEPROMData_Set(sizeof(gSysData.sendCnt) , (u8*)&gSysData.sendCnt) == FALSE)
	{
		return EEPROM_ERR;
	}
	
	return COMPLETE;
}

/*******************************************************************************
�� �� ��:  	Get_CmdAckNum_Attr
����˵��:  	��ȡָ��Ӧ�����
��    ��:  	data[0]: Ŀ�굥Ԫ
			data[1]: ��������
			rLen:	���ز�������
			rpara:	���ز�������
�� �� ֵ:  	��Ϣִ�н��
*******************************************************************************/
MsgResult_t Get_CmdAckNum_Attr(UnitPara_t *pData, u8 *rlen, u8 *rpara)
{
	*rpara = gSysData.sendCnt;
	*rlen = sizeof(gSysData.sendCnt);

	return COMPLETE;
}

/*******************************************************************************
�� �� ����  Set_RepeatBandAddr_Attr
����˵����  ����ת���󶨵�ַ
��	  ����  data[0]: Ŀ�굥Ԫ
			data[1]: ��������
�� �� ֵ:  	��Ϣִ�н��
*******************************************************************************/
MsgResult_t Set_RepeatBandAddr_Attr(UnitPara_t *pData)
{
	u8 i,cnt;
	if ( (pData->len > 25) 
		|| ((pData->len%5)) )//�������Ȳ��Ϸ�,ֻ��5��10��15��20��25��������
	{
		return PARA_LEN_ERR;
	}
	
	cnt = pData->len / 5;
	
	for(i=0; i<cnt; i++)
	{
		if(pData->data[i*5] == 0 
			|| pData->data[i*5] > 5)
		{
			return PARA_MEANING_ERR;
		}
	}
	
	for(i=0; i<cnt; i++)
	{
		gSysData.BandAddr[pData->data[i*5]-1] = *(u32*)&pData->data[i*5+1];
	}
	
	if (SysEEPROMData_Set(sizeof(gSysData.BandAddr) , (u8*)&gSysData.BandAddr) == FALSE)
	{
		return EEPROM_ERR;
	}
	
	return COMPLETE;
}

/*******************************************************************************
�� �� ��:  	Get_RepeatBandAddr_Attr
����˵��:  	��ȡת���󶨵�ַ
��    ��:  	data[0]: Ŀ�굥Ԫ
			data[1]: ��������
			rLen:	���ز�������
			rpara:	���ز�������
�� �� ֵ:  	��Ϣִ�н��
*******************************************************************************/
MsgResult_t Get_RepeatBandAddr_Attr(UnitPara_t *pData, u8 *rlen, u8 *rpara)
{
	u8 i,j=0;

	if(pData->data[0] == 0xFF)
	{
		//ȫ��
		*rlen = 0;
		for(i=0; i<5; i++)
		{
			if(gSysData.BandAddr[i] 
				&& gSysData.BandAddr[i] != 0xFFFFFFFF)
			{
				*(rpara + j*5) = i + 1;
				*(u32 *)(rpara + 1 + j*5) = gSysData.BandAddr[i];
				*rlen += 5;
				j++;
			}
		}
	}
	else if(pData->data[0] == 0 || pData->data[0] > 5)
	{
		return PARA_LEN_ERR;
	}
	else
	{
		*rpara = pData->data[0];
		*(u32 *)(rpara+1) = gSysData.BandAddr[pData->data[0]-1];
		*rlen = 5;
	}
	
	return COMPLETE;
}

/*******************************************************************************
�� �� ����  Set_RepeatBroadcast_Attr
����˵����  ����ת�����ϱ�����Ĺ㲥���ܣ���1Ϊʹ�ܣ�0Ϊ����
��	  ����  data[0]: Ŀ�굥Ԫ
			data[1]: ��������
�� �� ֵ:  	��Ϣִ�н��
*******************************************************************************/
MsgResult_t Set_RepeatBroadcast_Attr(UnitPara_t *pData)
{
	gSysData.BandAreaEn = pData->data[0] ? 1 : 0;
	
	if (SysEEPROMData_Set(sizeof(gSysData.BandAreaEn) , (u8*)&gSysData.BandAreaEn) == FALSE)
	{
		return EEPROM_ERR;
	}
	
	return COMPLETE;
}

/*******************************************************************************
�� �� ��:  	Get_RepeatBroadcast_Attr
����˵��:  	��ȡת�����ϱ�����Ĺ㲥���ܣ�1Ϊʹ�ܣ�0Ϊ����
��    ��:  	data[0]: Ŀ�굥Ԫ
			data[1]: ��������
			rLen:	���ز�������
			rpara:	���ز�������
�� �� ֵ:  	��Ϣִ�н��
*******************************************************************************/
MsgResult_t Get_RepeatBroadcast_Attr(UnitPara_t *pData, u8 *rlen, u8 *rpara)
{
	*rpara = (gSysData.BandAreaEn == 1) ? 1 : 0;
	*rlen = sizeof(gSysData.BandAreaEn);

	return COMPLETE;
}

/*******************************************************************************
�� �� ����  Set_Area_Attr
����˵����  ���õ�Ԫ����
��	  ����  data[0]: Ŀ�굥Ԫ
			data[1]: ��������
�� �� ֵ:  	��Ϣִ�н��
*******************************************************************************/
MsgResult_t Set_UnitArea_Attr(UnitPara_t *pData)
{
	if ((pData->len > 5)||((!pData->len)))//�������Ȳ��Ϸ�
	{
		return PARA_LEN_ERR;
	}
	else if (pData->data[0] == 0x00)	//ȫ��Ԫ����
	{
		u8 i;
		for (i = 1; i < UnitCnt_Get(); i++)
		{
			*(u32*)gUnitData[i].area = *(u32*)&pData->data[1];
			if (UnitPublicEEPROMData_Set(sizeof(gUnitData[i].area) , gUnitData[i].area) == FALSE)
			{
				return EEPROM_ERR;
			}
		}
	}
	else if (pData->data[0] >= UnitCnt_Get())	//�������ݲ��Ϸ�
	{
		return PARA_MEANING_ERR;
	}
	else
	{
		*(u32*)gUnitData[pData->data[0]].area = *(u32*)&pData->data[1];
		if (UnitPublicEEPROMData_Set(sizeof(gUnitData[pData->data[0]].area) , gUnitData[pData->data[0]].area) == FALSE)
		{
			return EEPROM_ERR;
		}
	}
	
	return COMPLETE;
}

/*******************************************************************************
�� �� ��:  	Get_Area_Attr
����˵��:  	��ȡ��Ԫ����
��    ��:  	data[0]: Ŀ�굥Ԫ
			data[1]: ��������
			rLen:	���ز�������
			rpara:	���ز�������
�� �� ֵ:  	��Ϣִ�н��
*******************************************************************************/
MsgResult_t Get_UnitArea_Attr(UnitPara_t *pData, u8 *rlen, u8 *rpara)
{
	if ((pData->len > 3)||((!pData->len)))//�������Ȳ��Ϸ�
	{
		return PARA_LEN_ERR;
	}
	else if (pData->data[0] == 0x00)	//ȫ��Ԫ����
	{
		u8 i;
		for (i = 1, *rlen = 0; i < UnitCnt_Get(); i++)
		{
			*(u32*)&rpara[*rlen] = *(u32*)gUnitData[i].area;
			*rlen += sizeof(gUnitData[i].area);
		}
	}
	else if (pData->data[0] >= UnitCnt_Get())	//�������ݲ��Ϸ�
	{
		return PARA_MEANING_ERR;
	}		
	else
	{
		*(u32*)rpara = *(u32*)gUnitData[pData->data[0]].area;
		*rlen = sizeof(gUnitData[pData->data[0]].area);
	}
	
	return COMPLETE;
}
	
/*******************************************************************************
�� �� ����  Set_UnitType_Attr
����˵����  ���õ�Ԫ����
��	  ����  data[0]: Ŀ�굥Ԫ
			data[1]: ��������
�� �� ֵ:  	��Ϣִ�н��
*******************************************************************************/
MsgResult_t Set_UnitType_Attr(UnitPara_t *pData)
{
	if ((pData->len > 3)||(!pData->len))//�������Ȳ��Ϸ�
	{
		return PARA_LEN_ERR;
	}
	else if (pData->data[0] == 0x00)	//ȫ��Ԫ����
	{
		u8 i;
		for (i = 1; i < UnitCnt_Get(); i++)
		{
			gUnitData[i].type = *(u16*)&pData->data[1];
			if (UnitPublicEEPROMData_Set(sizeof(gUnitData[i].type) , (u8*)&gUnitData[i].type) == FALSE)
			{
				return EEPROM_ERR;
			}
		}
	}
	else if (pData->data[0] >= UnitCnt_Get())	//�������ݲ��Ϸ�
	{
		return PARA_MEANING_ERR;
	}
	else
	{
		gUnitData[pData->data[0]].type = *(u16*)&pData->data[1];
		if (FALSE == (UnitPublicEEPROMData_Set(sizeof(gUnitData[pData->data[0]].type) , (u8*)&gUnitData[pData->data[0]].type)))
		{
			return EEPROM_ERR;
		}
	}
		
	return COMPLETE;
}

/*******************************************************************************
�� �� ��:  	Get_UnitType_Attr
����˵��:  	��ȡ��Ԫ����
��    ��:  	data[0]: Ŀ�굥Ԫ
			data[1]: ��������
			rLen:	���ز�������
			rpara:	���ز�������
�� �� ֵ:  	��Ϣִ�н��
*******************************************************************************/
MsgResult_t Get_UnitType_Attr(UnitPara_t *pData, u8 *rlen, u8 *rpara)
{
	if ((pData->len > 3)||((!pData->len)))//�������Ȳ��Ϸ�
	{
		return PARA_LEN_ERR;
	}
	else if (pData->data[0] == 0x00)	//ȫ��Ԫ����
	{
		u8 i;
		for (i = 1, *rlen = 0; i < UnitCnt_Get(); i++)
		{
			*(u16*)&rpara[*rlen] = gUnitData[i].type;
			*rlen += sizeof(gUnitData[i].type);
		}
	}
	else if (pData->data[0] >= UnitCnt_Get())	//�������ݲ��Ϸ�
	{
		return PARA_MEANING_ERR;
	}		
	else
	{
		*(u16*)rpara = gUnitData[pData->data[0]].type;
		*rlen = sizeof(gUnitData[pData->data[0]].type);
	}
	
	return COMPLETE;
}

/*******************************************************************************
�� �� ����  Set_CommonType_Attr
����˵����  ���õ�ԪӦ�ô���
��	  ����  data[0]: Ŀ�굥Ԫ
			data[1]: ��������
�� �� ֵ:  	��Ϣִ�н��
*******************************************************************************/
MsgResult_t Set_CommonType_Attr(UnitPara_t *pData)
{
	if ((pData->len > 2)||((!pData->len)))//�������Ȳ��Ϸ�
	{
		return PARA_LEN_ERR;
	}
	else if (pData->data[0] == 0x00)	//ȫ��Ԫ����
	{
		u8 i;
		for (i = 1; i < UnitCnt_Get(); i++)
		{
			gUnitData[i].common = pData->data[1];
			if (FALSE == (UnitPublicEEPROMData_Set(sizeof(gUnitData[i].common) , (u8*)&gUnitData[i].common)))
			{
				return EEPROM_ERR;
			}
		}
	}
	else if (pData->data[0] >= UnitCnt_Get())	//�������ݲ��Ϸ�
	{
		return PARA_MEANING_ERR;
	}
	else
	{
		gUnitData[pData->data[0]].common = pData->data[1];
		if (UnitPublicEEPROMData_Set(sizeof(gUnitData[pData->data[0]].common) , (u8*)&gUnitData[pData->data[0]].common) == FALSE)
		{
			return EEPROM_ERR;
		}
	}
		
	return COMPLETE;
}

/*******************************************************************************
�� �� ��:  	Get_CommonType_Attr
����˵��:  	��ȡ��ԪӦ�ô���
��    ��:  	data[0]: Ŀ�굥Ԫ
			data[1]: ��������
			rLen:	���ز�������
			rpara:	���ز�������
�� �� ֵ:  	��Ϣִ�н��
*******************************************************************************/
MsgResult_t Get_CommonType_Attr(UnitPara_t *pData, u8 *rlen, u8 *rpara)
{
	if ((pData->len > 2)||((!pData->len)))//�������Ȳ��Ϸ�
	{
		return PARA_LEN_ERR;
	}
	else if (pData->data[0] == 0x00)	//ȫ��Ԫ����
	{
		u8 i;
		for (i = 1, *rlen = 0; i < UnitCnt_Get(); i++)
		{
			rpara[*rlen] = gUnitData[i].common;
			*rlen += sizeof(gUnitData[i].common);
		}
	}
	else if (pData->data[0] >= UnitCnt_Get())	//�������ݲ��Ϸ�
	{
		return PARA_MEANING_ERR;
	}		
	else
	{
		*rpara = gUnitData[pData->data[0]].common;
		*rlen = sizeof(gUnitData[pData->data[0]].common);
	}
	
	return COMPLETE;
}
/*******************************************************************************
�� �� ����  Set_UploadAble_Attr
����˵����  ���õ�Ԫ�ϱ�ʹ�ܱ��
��	  ����  data[0]: Ŀ�굥Ԫ
			data[1]: ��������
�� �� ֵ:  	��Ϣִ�н��
*******************************************************************************/
MsgResult_t Set_UploadAble_Attr(UnitPara_t *pData)
{
	if ((pData->len > 3)||((!pData->len)))//�������Ȳ��Ϸ�
	{
		return PARA_LEN_ERR;
	}
	else if (pData->data[0] == 0x00)	//ȫ��Ԫ����
	{
		u8 i;
		for (i = 1; i < UnitCnt_Get(); i++)
		{
			gUnitData[i].UploadAble = pData->data[1];
			gUnitData[i].UploadSetTime = pData->data[2];
			if (UnitPublicEEPROMData_Set(sizeof(gUnitData[i].UploadAble)+sizeof(gUnitData[i].UploadSetTime),\
				(u8*)&gUnitData[i].UploadAble) == FALSE)
			{
				return EEPROM_ERR;
			}
		}
	}
	else if (pData->data[0] >= UnitCnt_Get())	//�������ݲ��Ϸ�
	{
		return PARA_MEANING_ERR;
	}	
	else 
	{
		gUnitData[pData->data[0]].UploadAble = pData->data[1];
		gUnitData[pData->data[0]].UploadSetTime = pData->data[2];
		if (UnitPublicEEPROMData_Set(sizeof(gUnitData[pData->data[0]].UploadAble)+sizeof(gUnitData[pData->data[0]].UploadSetTime),\
				(u8*)&gUnitData[pData->data[0]].UploadAble) == FALSE)
		{
			return EEPROM_ERR;
		}
	}		
	return COMPLETE;
}
/*******************************************************************************
�� �� ��:  	Get_UploadAble_Attr
����˵��:  	��ȡ��Ԫ�ϱ�ʹ�ܱ��
��    ��:  	data[0]: Ŀ�굥Ԫ
			data[1]: ��������
			rLen:	���ز�������
			rpara:	���ز�������
�� �� ֵ:  	��Ϣִ�н��
*******************************************************************************/
MsgResult_t Get_UploadAble_Attr(UnitPara_t *pData, u8 *rlen, u8 *rpara)
{
	if ((pData->len > 3))//�������Ȳ��Ϸ�
	{
		return PARA_LEN_ERR;
	}
	else if ((pData->data[0] == 0x00)||(!pData->len))	//ȫ��Ԫ����
	{
		u8 i;
		for (i = 1, *rlen = 0; i < UnitCnt_Get(); i++)
		{
			rpara[*rlen] = gUnitData[i].UploadAble;
			rpara[*rlen+1] = gUnitData[i].UploadSetTime;
			*rlen += sizeof(gUnitData[i].UploadAble)+sizeof(gUnitData[i].UploadSetTime);
		}
	}
	else if (pData->data[0] >= UnitCnt_Get())	//�������ݲ��Ϸ�
	{
		return PARA_MEANING_ERR;
	}		
	else
	{
		*rpara = gUnitData[pData->data[0]].UploadAble;
		*(rpara+1) = gUnitData[pData->data[0]].UploadSetTime;
		*rlen = sizeof(gUnitData[pData->data[0]].UploadAble)+sizeof(gUnitData[pData->data[0]].UploadSetTime);
	}	
	return COMPLETE;
}
/*******************************************************************************
�� �� ����  Set_UnitAble_Attr
����˵����  ���õ�Ԫʹ�ܱ��
��	  ����  data[0]: Ŀ�굥Ԫ
			data[1]: ��������
�� �� ֵ:  	��Ϣִ�н��
*******************************************************************************/
MsgResult_t Set_UnitAble_Attr(UnitPara_t *pData)
{
	if ((pData->len > 5)||((!pData->len)))//�������Ȳ��Ϸ�
	{
		return PARA_LEN_ERR;
	}
	else if (pData->data[0] == 0x00)	//ȫ��Ԫ����
	{
		u8 i;
		for (i = 1; i < UnitCnt_Get(); i++)
		{
			gUnitData[i].able = pData->data[1];
			if (UnitPublicEEPROMData_Set(sizeof(gUnitData[i].able) , (u8*)&gUnitData[i].able) == FALSE)
			{
				return EEPROM_ERR;
			}
		}
	}
	else if (pData->data[0] >= UnitCnt_Get())	//�������ݲ��Ϸ�
	{
		return PARA_MEANING_ERR;
	}	
	else 
	{
		gUnitData[pData->data[0]].able = pData->data[1];
		if (UnitPublicEEPROMData_Set(sizeof(gUnitData[pData->data[0]].able) , (u8*)&gUnitData[pData->data[0]].able) == FALSE)
		{
			return EEPROM_ERR;
		}
	}
		
	return COMPLETE;
}

/*******************************************************************************
�� �� ��:  	Get_UnitAble_Attr
����˵��:  	��ȡ��Ԫʹ�ܱ��
��    ��:  	data[0]: Ŀ�굥Ԫ
			data[1]: ��������
			rLen:	���ز�������
			rpara:	���ز�������
�� �� ֵ:  	��Ϣִ�н��
*******************************************************************************/
MsgResult_t Get_UnitAble_Attr(UnitPara_t *pData, u8 *rlen, u8 *rpara)
{
	if ((pData->len > 3))//�������Ȳ��Ϸ�
	{
		return PARA_LEN_ERR;
	}
	else if ((pData->data[0] == 0x00)||(!pData->len))	//ȫ��Ԫ����
	{
		u8 i;
		for (i = 1, *rlen = 0; i < UnitCnt_Get(); i++)
		{
			rpara[*rlen] = gUnitData[i].able;
			*rlen += sizeof(gUnitData[i].able);
		}
	}
	else if (pData->data[0] >= UnitCnt_Get())	//�������ݲ��Ϸ�
	{
		return PARA_MEANING_ERR;
	}		
	else
	{
		*rpara = gUnitData[pData->data[0]].able;
		*rlen = sizeof(gUnitData[pData->data[0]].able);
	}
	
	return COMPLETE;
}

/*******************************************************************************
�� �� ��:  	Get_Eeprom_Info_Attr
����˵��:  	��ȡEEPROM���ݵĽӿ�
��    ��:  	data[0]: Ŀ�굥Ԫ
			data[1]: ��������
			rLen:	���ز�������
			rpara:	���ز�������
�� �� ֵ:  	��Ϣִ�н��
ע    �⣺  EEPROM����256kb���㣬��ַ��0x0000 - 0x7FFF
*******************************************************************************/
MsgResult_t Get_Eeprom_Info_Attr(UnitPara_t *data, u8 *rlen, u8 *rpara)
{
	u16 addr ;
	addr = data->data[0]+ data->data[1]*256 ;
	
	if(data->data[2] > 130) return PARA_MEANING_ERR ; //ÿ������ȡ130bytes
	else if(addr > 0x7FFF) return EEPROM_RAND_ERR ;
	else if((addr + data->data[2]) > 0x8000)
	{
		*rlen = (0x8000-addr) ;		
	}
	else
	{
		*rlen = data->data[2] ;
	}
	if (ReadDataFromEEPROM(addr, *rlen, rpara) == FALSE)
	{
		return EEPROM_ERR;
	}	
	return COMPLETE ;
}
/*******************************************************************************
�� �� ����  Set_FaultFlag_Attr
����˵����  �����쳣�������
��	  ����  data[0]: Ŀ�굥Ԫ
			data[1]: ��������
�� �� ֵ:  	��Ϣִ�н��
*******************************************************************************/
MsgResult_t Set_FaultFlag_Attr(UnitPara_t *pData)
{	
	if(pData->len == 0)
	{
		FaultData_Init();	
	}
	else if(pData->len == 5)
	{
		WriteDataToEEPROM(FAULT_START_ADD + sizeof(u32)*(pData->data[0]), 
							sizeof(u32), (u8*)&pData->data[1]);  	//��ȡ��ʶ
	}      //ʹ��EEPROM_W���������洢����ֵ  yanhuan adding 2015/11/10
	return COMPLETE;
}

/*******************************************************************************
�� �� ��:  	Get_FaultFlag_Attr
����˵��:  	��ȡ�쳣�������
��    ��:  	data[0]: Ŀ�굥Ԫ
			data[1]: ��������
			rLen:	���ز�������
			rpara:	���ز�������
�� �� ֵ:  	��Ϣִ�н��
*******************************************************************************/
MsgResult_t Get_FaultFlag_Attr(UnitPara_t *pData, u8 *rlen, u8 *rpara)
{
	FaultData_Get(rlen, rpara);	
	return COMPLETE;
}

/*******************************************************************************
�� �� ����  Set_UserDataFormat_Attr
����˵����  �����û����ݻָ���������
��	  ����  data[0]: Ŀ�굥Ԫ
			data[1]: ��������
�� �� ֵ:  	��Ϣִ�н��
*******************************************************************************/
MsgResult_t Set_UserDataFormat_Attr(UnitPara_t *pData)
{
	if (pData->len > 1)	//���ݳ��ȴ���
	{
		return PARA_LEN_ERR;
	}
	if((1 == pData->len)
		&& (0 == pData->data[0]))  //�ָ��������� ��Ҫ�ָ�433�ŵ�
	{
		UserEEPROMData_Init(SecondLevel);
	}
	else if (0 == pData->len)	//���ݳ���Ϊ��  Ĭ�ϴ���ԪΪ0
	{
		UserEEPROMData_Init(FristLevel);
	}
	else if (pData->data[0] < UnitCnt_Get()) //������ȷ
	{
		UnitPublicEEPROMUnitData_Init(pData->data[0]);
		UnitPrivateEEPROMUnitData_Init(pData->data[0]);
		GroupEEPROMUnit_Init(pData->data[0]);
		SceneEEPROMUnit_Init(pData->data[0]);
		DelLinkForUnit(pData->data[0]);
	}
	else 	
	{
		return PARA_MEANING_ERR;
	}
	
	return COMPLETE;
}
#ifndef   STM32L151 		//���ǵ͹��Ĳ�Ʒ
/*******************************************************************************
�� �� ����  Set_DelayUpload_Attr
����˵����  �����û������ӳ��ϱ�����Ϊ�������жϹ���
��	  ����  data[0]: Ŀ�굥Ԫ
			data[1]: ��������
�� �� ֵ:  	��Ϣִ�н��
ע    ��:  4bytes,��λΪ��
*******************************************************************************/
MsgResult_t Set_DelayUpload_Attr(UnitPara_t *pData)
{
	u32 time = *(u32*)&pData->data[0];
	if(pData->len == 4)
	{
		Thread_Login(ONCEDELAY ,0 ,Time_Get()%(time*1000),SysTickUpload);
		return COMPLETE ;
	}
	return PARA_LEN_ERR ;
}
#endif
///*******************************************************************************
//�� �� ��:  	Get_DeviceInfo_Attr
//����˵��:  	��ȡ�豸��Ϣ V1.8 ADD
//��    ��:  	data[0]: Ŀ�굥Ԫ
//			data[1]: ��������
//			rLen:	���ز�������
//			rpara:	���ز�������
//�� �� ֵ:  	��Ϣִ�н��
//*******************************************************************************/
//MsgResult_t Get_DeviceInfo_Attr(UnitPara_t *pData, u8 *rlen, u8 *rpara)
//{
//	*(u16*)&rpara[0] = SOFT->name; 
//	*(u32*)&rpara[2] = SOFT->version;
//	*(u32*)&rpara[6] = HARD->hardVer;
//	*rlen = 10;
////		str->ucpData[10] = g_stEeEx.Sys.Board.DevLinckEn;
////		*(u32*)&str->ucpData[11] = HARDINFO->data;
////		*(u32*)&str->ucpData[15] = HARDINFO->time;	
////		for ( i = 0; i < 100 && HARDINFO->str[i] != '\0'; i++)
////		{
////			str->ucpData[i+19] = HARDINFO->str[i];
////		}
////		str->ucDataLen = i + 19;
//	
//	return COMPLETE;
//}

///*******************************************************************************
//�� �� ����  Set_DeviceAdd_Attr
//����˵����  �����豸�������
//��	  ����  data[0]: Ŀ�굥Ԫ
//			data[1]: ��������
//�� �� ֵ:  	��Ϣִ�н��
//*******************************************************************************/
//MsgResult_t Set_DeviceAdd_Attr(UnitPara_t *pData)
//{
//	u8 i;
//	
//	if (!pData->len)//�������Ȳ��Ϸ�
//	{
//		return PARA_LEN_ERR;
//	}
//	UserEEPROMData_Init();
//	for (i = 1; i < UnitCnt_Get(); i++)
//	{
//		*(u32*)gUnitData[i].area = *(u32*)&pData->data[0];
//		if (UnitPublicEEPROMData_Set(sizeof(gUnitData[i].area) , gUnitData[i].area) == FALSE)
//		{
//			return EEPROM_ERR;
//		}
//	}
//	gSysData.userID = *(u32*)&pData->data[4];
//	if (SysEEPROMData_Set(sizeof(gSysData.userID) , (u8*)&gSysData.userID) == FALSE)
//	{
//		return EEPROM_ERR;
//	}
//	
//	return COMPLETE;
//}

/*******************************************************************************
�� �� ����  Set_link_Attr
����˵����  ��������
��	  ����  data[0]: Ŀ�굥Ԫ
			data[1]: ��������
�� �� ֵ:  	��Ϣִ�н��
*******************************************************************************/
MsgResult_t Set_Link_Attr(UnitPara_t *pData)
{
	MsgResult_t result;
	
	if (!pData->len)//�������Ȳ��Ϸ�
	{
		return PARA_LEN_ERR;
	}
	switch(pData->data[0])
	{
		case WRITELINK:
			result = (MsgResult_t)WriteLinkItem(pData->data[1], (u8*)&pData->data[2]);
			if (result == COMPLETE)
			{
				pData->len = 1;
				return OWN_RETURN_PARA;
			}
			return result;
		case DELLINK:
			if (DelLinkItem(pData->data[1], pData->data[2]) == 1)
			{
				return COMPLETE;
			}
			return LINKMAP_ERR;
		case MODIFYLINK:
			if((ModifyLinkItem(pData->data[1], pData->data[2], (u8*)&pData->data[3])) == 1)
			{
				return COMPLETE;
			}
			return LINK_MODIFY_ERR;

		default: break;
	}
	return PARA_MEANING_ERR;
}

/*******************************************************************************
�� �� ��:  	Get_Link_Attr
����˵��:  	��ȡ����
��    ��:  	data[0]: Ŀ�굥Ԫ
			data[1]: ��������
			rLen:	���ز�������
			rpara:	���ز�������
�� �� ֵ:  	��Ϣִ�н��
*******************************************************************************/
MsgResult_t Get_Link_Attr(UnitPara_t *pData, u8 *rlen, u8 *rpara)
{
	switch(pData->data[0])
	{
		case READLINK:
			if (ReadLinkItem(pData->data[1], pData->data[2], rlen, rpara) == 1)
			{
				return COMPLETE;
			}
			break;
		case QUERYLINK:
			if (QueryLinkItem(pData->data[1], pData->data[2], rlen, rpara))
			{
				return COMPLETE;
			}
			break;
		default: break;
	}

	return LINKMAP_ERR;
}
#if (ALINKTIME_EN > 0)
/*******************************************************************************
�� �� ����  Set_Link_Time_Attr
����˵����  ������������ʱ������
��	  ����  data[0]: Ŀ�굥Ԫ
			data[1]: ��������
�� �� ֵ:  	��Ϣִ�н��
*******************************************************************************/
MsgResult_t Set_Link_Time_Attr(UnitPara_t *pData)
{
	MsgResult_t result;
	
	if (!pData->len)//�������Ȳ��Ϸ�
	{
		return PARA_LEN_ERR;
	}
	switch(pData->data[0])
	{
		case WRITELINK:
			result = (MsgResult_t)WriteLinkItemEx(pData->data[1], (u8*)&pData->data[2]);
			if (result == COMPLETE)
			{
				pData->len = 1;
				return OWN_RETURN_PARA;
			}
			return result;
		case DELLINK:
			if (DelLinkItem(pData->data[1], pData->data[2]) == 1)
			{
				return COMPLETE;
			}
			return LINKMAP_ERR;
		case MODIFYLINK:
			if((ModifyLinkItemEx(pData->data[1], pData->data[2], (u8*)&pData->data[3])) == 1)
			{
				return COMPLETE;
			}
			return LINK_MODIFY_ERR;

		default: break;
	}
	return PARA_MEANING_ERR;
}
#endif
///*******************************************************************************
//�� �� ��:  	Get_Link_Time_Attr
//����˵��:  	��ȡ����
//��    ��:  	data[0]: Ŀ�굥Ԫ
//			data[1]: ��������
//			rLen:	���ز�������
//			rpara:	���ز�������
//�� �� ֵ:  	��Ϣִ�н��
//*******************************************************************************/
//MsgResult_t Get_Link_Time_Attr(UnitPara_t *pData, u8 *rlen, u8 *rpara)
//{
//	switch(pData->data[0])
//	{
//		case READLINK:
//			if (ReadLinkItem(pData->data[1], pData->data[2], rlen, rpara) == 1)
//			{
//				return COMPLETE;
//			}
//			break;
//		case QUERYLINK:
//			if (QueryLinkItem(pData->data[1], pData->data[2], rlen, rpara))
//			{
//				return COMPLETE;
//			}
//			break;
//		default: break;
//	}

//	return LINKMAP_ERR;
//}


/*******************************************************************************
�� �� ����  Set_DelLinkForUnit_Attr
����˵����  ����Ԫɾ������
��	  ����  data[0]: Ŀ�굥Ԫ
			data[1]: ��������
�� �� ֵ:  	��Ϣִ�н��
*******************************************************************************/
MsgResult_t Set_DelLinkForUnit_Attr(UnitPara_t *pData)
{
	if (!pData->len)//�������Ȳ��Ϸ�
	{
		return PARA_LEN_ERR;
	}
	if (pData->data[0] == 0)		//Unarty�����ģġ�ɾ���豸��������    
	{
		DelAllLinkFromTable(SIMPLESTLINK);
	}
	else if(DelLinkForUnit(pData->data[0]) == 0)
	{
		return LINK_WRITE_ERR;
	}
	
	return COMPLETE;
}


/*******************************************************************************
�� �� ��:  	MsgResult_t Get_DeveiceSysClockFeature_Attr(UnitPara_t *pData, u8 *rlen, u8 *rpara)
����˵��:  	��ȡ��ǰ�豸ϵͳʱ������
��    ��:  	rLen:	���ز�������
			rpara:	���ز�������
�� �� ֵ:  	��Ϣִ�н��
*******************************************************************************/
MsgResult_t Get_DeveiceSysClockFeature_Attr(UnitPara_t *pData, u8 *rlen, u8 *rpara)
{
	u32 frequency = 0u;
	u8 clock_source = 0xff;
	
	if(1 == Get_SysClk_FrqAndSrc (&frequency,&clock_source))
	{
		*(u32*)rpara 		= frequency;		//��ǰϵͳʱ�ӵ�Ƶ��
		*(u8*)(rpara + 4) 	= clock_source;		//��ǰʹ�õ�ʱ��Դ(�ڲ�->0/�ⲿ->1)
		*(u8*)rlen			= sizeof(frequency) + sizeof(clock_source);
		return COMPLETE;
	}
	return CMD_EXE_ERR;
}
/**************************Copyright BestFu 2014-05-14*************************/
