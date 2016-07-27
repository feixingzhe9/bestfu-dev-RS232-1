/***************************Copyright BestFu 2014-05-14*************************
��	����	Object.c
˵	����	Ŀ������ƥ����غ���
��	�룺	Keil uVision4 V4.54.0.0
��	����	v1.0
��	д��	Unarty
��	�ڣ�	2014.06.23 
�ޡ��ģ�	����
*******************************************************************************/
#include "Object.h"
#include "command.h"
#include "UserData.h"
#include "UnitCfg.h"
#include "BestFuLib.h"
#include "SysTick.h"
#include "CMDProcess.h"
#include "Updata.h"
#include "SysHard.h"
#include "DevPartTab.h"
#include "DlyAttrProcess.h"

//public variable define
u32 DelayID;	//����ͨ��ƥ�䣨Unarty 2014.08.07)

//Private variable define
static RepeatSt RData[REPEAT_MAX]={0};//���������ظ���ˮ�ţ�����ʱ���жϡ�2015.7.30

//Private function declaration
static u8 Check_Repeat(const CMDAim_t *aim);    //��Ϣ�ظ��˶�

/*******************************************************************************
�� �� ����	Object_Check
����˵���� 	Ŀ��˶�
��	  ���� 	aim:		��ϢĿ������
			*validUnit:	Ŀ�������Ч��Ԫ
�� �� ֵ��	0(��Ϣƥ��ɹ���/ 1(�û��Ų�ͬ��/2(ָ���ظ�)/3 (Ŀ�겻ƥ�䣩
            /4(����ƥ��)/5(��Ϣ����Խ��)
*******************************************************************************/
MsgResult_t Object_Check(u8 *sdata, u8 len, CMDAim_t *aim, CMDPara_t *para, u8 *data)
{
	if ( aim->userID == gSysData.userID				//У�������û���	
		|| aim->userID == SUPER_USERID				//У�鳬���û���
		|| gSysData.userID == DEFAULT_USERID		//�û���ΪĬ��ֵ
		)
	{
		if (Check_Repeat(aim) == FALSE)  //ָ���ظ�
		{
			return MSG_REPEAT;	//����ָ��������
		}
		else
		{
#ifndef STM32L151		//���ǵ͹��Ĳ�Ʒ
			if(Check_BandAddr(aim))
			{
				//����ת�������
				EncryptCmd_put(&send433fifo,&sdata[HEAD_SIZE],sizeof(Communication_t) + para->len);
			}
#endif
		}			
		if (aim->objectType == SINGLE_ACT)	//���ڵ����
		{
			if (aim->object.id != gSysData.deviceID)
			{
				return ID_ERR;	//����Ŀ��ƥ�����
			}
			DelayID = 0;		//����ͨ��ƥ�䣨Unarty 2014.08.07)
		}
		else if (aim->objectType == DEV_ALL_ACT && aim->object.id == 0)
		{
			DelayID = aim->sourceID;		//����ͨ��ƥ�䣨Unarty 2014.08.07)
		}
        else
		{
			u32 area, validUnit;
			u8 i, len;		
			DevPivotalSet_t* pDevPivotalSet = GetDevPivotalSet();
			Soft_t* pSoft = (Soft_t*)(pDevPivotalSet->DevSoftInfoAddr);
		
#if (FLASH_ENCRYPTION_EN > 0u)
			u32 temp = 0x00;			
			temp = DecryptionFlash();		//���Ӵ������ jay add 2015.12.07
#endif
			DelayID = aim->sourceID;		//����ͨ��ƥ�䣨Unarty 2014.08.07)
			area = Check_Area(aim->object.area, &validUnit);	//ƥ���������򣬻�ȡ�����Ӧ����Ч��Ԫ
#if (FLASH_ENCRYPTION_EN > 0u)
			validUnit <<= temp;area += temp;
#endif
			if (!validUnit)	//û����Ч��Ԫ
			{
				return AREA_ERR;	//��������ƥ�����
			}
			
			if (aim->objectType == GROUP_ACT) //��㲥	
			{
#if (ALINKTIME_EN > 0)
				if((0x00 == para->unit)&&\
				   (0x91 == para->cmd))
				{
					len = para->len;
				}
				else
#endif
				{
					memmove(&data[3], &data[0], para->len); //����������������
					memcpy(&data[0], &para->unit, 3);		//���ڷ��ò������������ݣ���Ԫ�ţ����Ժţ���������
					len = para->len + 3;
					para->unit = 0;
					para->cmd = 0x00;
				}
				para->len = 0;
				for (i = 1; i < UnitCnt_Get(); i++)
				{
					if ((validUnit&(1<<i))	//��Ԫλ�ڲ�����Χ��
						&& GroupEEPROM_Get(i, area, aim->actNum)	//���ҵ�Ԫ�Ƿ���ӵ����� 
						)
					{
						memcpy(&data[para->len], &data[0], len);
						data[para->len] = i;
						para->len += len;
					}
				}
			}
			else if (aim->objectType == SCENE_ACT) //�����㲥
			{
#if (SCENCETIME_EN > 0u)
				u8 scene_data[128],scene_len = 0;				
				scene_data[0] = 0x00;		//��Ԫ��
				scene_data[1] = 0x91;		//���Ժ�
				scene_data[2] = 0x00;		//����������
#endif				
				para->unit = 0x00;
				para->cmd = 0x00;
				para->len = 0;
#if (SCENCETIME_EN > 0u)
				for (i = 1; i < UnitCnt_Get(); i++)
				{
					if ((validUnit&(1<<i)))	//��Ԫλ�ڲ�����Χ��
					{
						if(TRUE == SceneEEPROM_GetEx(i, area, aim->actNum, &data[para->len]))	//��ȡ��������ֵ
						{
							//��ʱִ�ж�����ִ�к���
							if(data[para->len] != 0)
							{
								scene_len = data[para->len];
								memcpy(&scene_data[3]+scene_data[2],&data[para->len]+1,scene_len);
								scene_data[2] += scene_len;
							}
						}
						else
						{
							if (data[para->len] != 0)	//���������������Ϊ��
							{
								len = data[para->len];	//�������������
								memmove(&data[para->len], &data[para->len] + 1, len);
								para->len += len;
							} 
						}
					}
				}
				if(0 != scene_data[2])
				{
					memcpy(&data[para->len],&scene_data,scene_data[2] + 3);
					para->len += scene_data[2] + 3;
				}
#else				
				for (i = 1; i < UnitCnt_Get(); i++)
				{
					if ((validUnit&(1<<i)))	//��Ԫλ�ڲ�����Χ��
					{
						SceneEEPROM_Get(i, area, aim->actNum, &data[para->len]);	//��ȡ��������ֵ
						if (data[para->len] != 0)	//���������������Ϊ��
						{
							len = data[para->len];	//�������������
							memmove(&data[para->len], &data[para->len] + 1, len);
							para->len += len;
						} 
					}
				}						
#endif	
				
			}
			else if (aim->objectType == DEVIVCE_TYPE_ACT)//���͹㲥
			{
				if(0x00 == para->unit)
				{
					memmove(&data[3], &data[0], para->len); //����������������
					memcpy(&data[0], &para->unit, 3);		//���ڷ��ò������������ݣ���Ԫ�ţ����Ժţ���������
					len = para->len + 3;
					para->unit = 0;
					para->cmd = 0x00;
					para->len = 0;
					for (i = 1; i < UnitCnt_Get(); i++)
					{
						if ((validUnit&(1<<i))	//��Ԫλ�ڲ�����Χ��
							&& (aim->actNum == (u16)pSoft->deviceType))	//���ҵ�Ԫ�Ƿ���ӵ����� ,Jay Add 2015.11.14
						{
							memcpy(&data[para->len], &data[0], len);
							data[para->len] = i;
							para->len += len;
						}
					}
				}
				else
				{
					if(para->unit > UnitCnt_Get() - 1) return PARA_MEANING_ERR;  //������ԪԽ����					
					if(!((validUnit&(1 << (para->unit)))&&			//��Ԫλ�ڲ�����Χ��
						((aim->actNum == (u16)pSoft->deviceType) ||		//ר���ھƵ�����,Jay Add 2015.11.14
						((0x001D == (u16)pSoft->deviceType)&&(0x001E == aim->actNum))))
					  )
					{
						return PARA_MEANING_ERR;
					}
				}
			}
			else if (aim->objectType == UNIT_USER_TYPE_ACT)//��ԪӦ�����͹㲥
			{
				memmove(&data[3], &data[0], para->len); //����������������
				memcpy(&data[0], &para->unit, 3);		//���ڷ��ò������������ݣ���Ԫ�ţ����Ժţ���������
				len = para->len + 3;
				para->unit = 0;
				para->cmd = 0x00;
				para->len = 0;
				for (i = 1; i < UnitCnt_Get(); i++)
				{
					if ((validUnit&(1<<i))	//��Ԫλ�ڲ�����Χ��
						&& (aim->actNum == gUnitData[i].type)	//���ҵ�Ԫ�Ƿ���ӵ����� 
						)
					{
						memcpy(&data[para->len], &data[0], len);
						data[para->len] = i;
						para->len += len;
					}
				}
			}
			else if (aim->objectType == UNIT_COMMON_TYPE_ACT)//��ԪӦ�����͹㲥
			{
				memmove(&data[3], &data[0], para->len); //����������������
				memcpy(&data[0], &para->unit, 3);		//���ڷ��ò������������ݣ���Ԫ�ţ����Ժţ���������
				len = para->len + 3;
				para->unit = 0;
				para->cmd = 0x00;
				para->len = 0;
				for (i = 1; i < UnitCnt_Get(); i++)
				{
					if ((validUnit&(1<<i))	//��Ԫλ�ڲ�����Χ��
						&& (aim->actNum == gUnitData[i].common)	//���ҵ�Ԫ�Ƿ���ӵ����� 
						)
					{
						memcpy(&data[para->len], &data[0], len);
						data[para->len] = i;
						para->len += len;
					}
				}
			}
			else if(aim->objectType == SPEC_AREA_DEV_TYPE_ACT)  //�ض������豸���͹㲥  Jay Add 2015.11.17
			{
				if(0x00 == para->unit)
				{
					memmove(&data[3], &data[0], para->len); //����������������
					memcpy(&data[0], &para->unit, 3);		//���ڷ��ò������������ݣ���Ԫ�ţ����Ժţ���������
					len = para->len + 3;
					para->unit = 0;
					para->cmd = 0x00;
					para->len = 0;
					for (i = 1; i < UnitCnt_Get(); i++)
					{
						if (((*(u32*)gUnitData[i].area) == aim->object.id)	//��Ԫλ�ڲ�����Χ��
							&& (aim->actNum == (u16)pSoft->deviceType))	//���ҵ�Ԫ�Ƿ���ӵ����� ,Jay Add 2015.11.14
						{
							memcpy(&data[para->len], &data[0], len);
							data[para->len] = i;
							para->len += len;
						}
					}
				}
				else
				{
					if(para->unit > UnitCnt_Get() - 1) return PARA_MEANING_ERR;     //������ԪԽ����
					if(!(((*(u32*)gUnitData[para->unit].area) == aim->object.id)&&	//��Ԫλ�ڲ�����Χ��
						((aim->actNum == (u16)pSoft->deviceType) ||						//ר���ھƵ�����,Jay Add 2015.11.14
						((0x001D == (u16)pSoft->deviceType)&&(0x001E == aim->actNum))))
					  )
					{
						return PARA_MEANING_ERR;
					}
				}
			}
			else if(aim->objectType == SPEC_AREA_UNIT_USER_TYPE_ACT) //�ض�����ԪӦ�����͹㲥  Jay Add 2015.11.17
			{
				memmove(&data[3], &data[0], para->len); //����������������
				memcpy(&data[0], &para->unit, 3);		//���ڷ��ò������������ݣ���Ԫ�ţ����Ժţ���������
				len = para->len + 3;
				para->unit = 0;
				para->cmd = 0x00;
				para->len = 0;
				for (i = 1; i < UnitCnt_Get(); i++)
				{
					if (((*(u32*)gUnitData[i].area) == aim->object.id)	//��Ԫλ�ڲ�����Χ��
						&& (aim->actNum == gUnitData[i].type)	//���ҵ�Ԫ�Ƿ���ӵ����� 
						)
					{
						memcpy(&data[para->len], &data[0], len);
						data[para->len] = i;
						para->len += len;
					}
				}
			}
			else
			{
				return ADDS_TYPE_ERR;
			}
			if(!para->len)	//���㲥û�пɲ����ĵ�Ԫ����,��ֱ�ӷ���
			{
				return NO_CMD;
			}
		}        
        aim->object.id = aim->sourceID;      //������ַ
		
        return COMPLETE;
	}

    return USER_ID_ERR;
}
#if (COMMUNICATION_SHORT_FRAME == 1)
/*******************************************************************************
�� �� ����	Short_Len
����˵���� 	��֡���Ȳ���
��	  ���� 	aim:		��ϢĿ������
			*validUnit:	Ŀ�������Ч��Ԫ
�� �� ֵ��	ÿ����һ�ֶ�֡����Ҫ�ڴ���Ӷ�Ӧ�Ķ�֡case
*******************************************************************************/
static u8 Short_Len(u8 version)
{
	switch(version)
	{
		case SYSTICK_SHORT_VERSION :
			return sizeof(ShortCMDAim_t);
		default:
			break;
	}
	return 0 ;
}
/*******************************************************************************
�� �� ����	ShortMsg_Check
����˵���� 	��֡��Ϣ���
��	  ���� 	aim:		��ϢĿ������
			*validUnit:	Ŀ�������Ч��Ԫ
�� �� ֵ��	0(��Ϣƥ��ɹ���/ 1(�û��Ų�ͬ��/2(ָ���ظ�)/3 (Ŀ�겻ƥ�䣩
            /4(����ƥ��)/5(��Ϣ����Խ��)
*******************************************************************************/
static u8 ShortMsg_Check(ShortCommunication_t *pMsg, u8 len)
{
	if(len != Short_Len(pMsg->aim.version))
		return FALSE;
	return TRUE ;
}
/*******************************************************************************
�� �� ����	ShortObject_Check
����˵���� 	��֡Ŀ��˶�
��	  ���� 	aim:		��ϢĿ������
			*validUnit:	Ŀ�������Ч��Ԫ
�� �� ֵ��	0(��Ϣƥ��ɹ���/ 1(�û��Ų�ͬ��/2(ָ���ظ�)/3 (Ŀ�겻ƥ�䣩
            /4(����ƥ��)/5(��Ϣ����Խ��)
*******************************************************************************/
MsgResult_t ShortObject_Check(ShortCommunication_t *pMsg, u8 len)
{
	u8 data[50];
	CMDAim_t *aimShort = (CMDAim_t*)data ;
	if(pMsg->aim.userID == gSysData.userID)
	{
		if(TRUE == ShortMsg_Check(pMsg,len))
		{
			aimShort->sourceID = pMsg->aim.sourceID ;
			aimShort->serialNum = pMsg->aim.serialNum ;
			aimShort->objectType = SINGLE_ACT ; //Ĭ�Ͼ�Ϊ���ڵ���Ϣ
			if (Check_Repeat(aimShort) == FALSE)  //ָ���ظ�
			{
					return MSG_REPEAT;	//����ָ��������
			}
#ifndef STM32L151		//���ǵ͹��Ĳ�Ʒ
			if(Check_BandAddr(aimShort))
			{
				//����ת�������
				short_put(&send433_shortfifo, (u8*)pMsg, len);
			}
			return COMPLETE ;
#endif
		}
	}
	return CMD_EXE_ERR ;
}
#endif
/*******************************************************************************
�� �� ����	Check_Area
����˵���� 	�˶��豸�����
��	  ���� 	area:   �����ַ����
�� �� ֵ��	��ǰƥ������ֵ
*******************************************************************************/
u8 Check_Area(const u8 *area, u32 *validUnit)
{
    u8 i, j, k;

   for (i = 4; i > 0; i--)
   {
        if (area[i-1] != AREA_VALUE)  //�Ⱥ˶�������
        {
            break;
        }
	}  
	for (j = 0, *validUnit = 0xFFFFFFFF; j < i; j++)
	{
		if (area[j] != AREA_VALUE)//�ٺ˶������� 
		{
			for (k = UnitCnt_Get(); k; )	
			{
				if (gUnitData[--k].area[j] != area[j])//�˶Ե�Ԫ��������
				{
					*validUnit &= ~(1<<k);
				}
			}
		}
	}

	return i;
}
#ifndef STM32L151		//���ǵ͹��Ĳ�Ʒ
/*******************************************************************************
�� �� ����	Check_AreaEx
����˵���� 	�˶��豸�����
��	  ���� 	area:   �����ַ����
�� �� ֵ��	��ǰƥ������ֵ
*******************************************************************************/
u8 Check_AreaEx(const u8 *area)
{
   u8 i;
   for (i = 0; i < 4 ; i++)
   {
			if(!(gUnitData[1].area[i] == area[i]
				|| area[i] == AREA_VALUE))
			{
				return FALSE;
			}
	} 
	return TRUE;
}
#endif
/*******************************************************************************
�� �� ����	Check_Repeat
����˵���� 	�˶�ָ���Ƿ��ظ�
��	  ���� 	data:	Ҫ�˶Ե�ָ������
�� �� ֵ��	TRUE(�ظ�)/FALSE(���ظ�)
*******************************************************************************/
#if 0
static u8 Check_Repeat(const CMDAim_t *aim)
{
	static u32 srcID[10] = {0};
	static u8 serialN[10] = {0}, seat;
	u8 i;

	for (i = 0; i < sizeof(srcID)/sizeof(srcID[0]); i++)
	{
		if (srcID[i] == aim->sourceID)	//����Դ��ַ���
		{
			if (serialN[i] == aim->serialNum)	//��ˮ�����
			{
				return FALSE;	//�����Ѵ���
			}
			else
			{
				serialN[i] = aim->serialNum;
				return TRUE;
			}
		}
	}
	seat = ++seat%(sizeof(srcID)/sizeof(srcID[0]));
	serialN[seat] = aim->serialNum;
	srcID[seat]  = aim->sourceID;
		
	return TRUE;
}
#endif
#if 0
static u8 Check_Repeat(const CMDAim_t *aim)
{
	static u8 seat;
	u8 i;

	for (i = 0; i < REPEAT_MAX; i++)
	{
		if (RData[i].srcID == aim->sourceID)	//����Դ��ַ���
		{
			if (RData[i].serial == aim->serialNum)	//��ˮ�����
			{
				return FALSE;	//�����Ѵ���
			}
			else
			{
				RData[i].serial = aim->serialNum;
				RData[i].time   = Time_Get();
				return TRUE;
			}
		}
	}
	seat = ++seat%REPEAT_MAX;
	RData[seat].serial = aim->serialNum;
	RData[seat].srcID  = aim->sourceID;
	RData[i].time      = Time_Get();
	
	return TRUE;
}
#endif

static u8 Check_Repeat(const CMDAim_t *aim)
{
	static u8 seat;
	u8 i,j;

	for (i = 0; i < REPEAT_MAX; i++)
	{
		if (RData[i].srcID == aim->sourceID)	//����Դ��ַ���
		{
			for(j=0; j < 5; j++)
			{
				if(RData[i].serial[j] == aim->serialNum)
				{
					return FALSE;	//�����Ѵ���
				}
			}
			
			RData[i].serialSeat = ++RData[i].serialSeat%5;   //��ˮ��ƫ��λ��
			RData[i].serial[RData[i].serialSeat] = aim->serialNum;	//��¼��ˮ��
			return TRUE;
		}
	}
	//ԭ��ַ����ͬ
	seat = ++seat%REPEAT_MAX; //ԭ��ַƫ��λ��
	RData[seat].serial[0] = aim->serialNum; //��ˮ��
	RData[seat].srcID     = aim->sourceID;  //ԭ��ַ

	return TRUE;
}
#if 0
/*******************************************************************************
�� �� ����	ClearRepeatSerialNum
����˵���� 	�˶��ظ�ָ���ʱ���Ƿ�ʱ����ʱ�������¼
��	  ���� 	void
�� �� ֵ��	void
*******************************************************************************/
void ClearRepeatSerialNum(void)
{
	u32 time=Time_Get();
	u8 i;

	for (i = 0; i < REPEAT_MAX; i++)
	{
		if ((RData[i].time - time > REPEAT_OVERT) 
			|| (time < RData[i].time))	//��ʱ10�룬����С��֮ǰ��¼ֵ
		{
			RData[i].serial = 0;
			RData[i].time   = 0;
		}
	}

	return;
}
#endif
#ifndef STM32L151		//���ǵ͹��Ĳ�Ʒ
/*******************************************************************************
�� �� ����	Check_BandAddr
����˵���� 	��ʵaim�Ƿ��а�ת����ַ������ת����û����ת��
��	  ���� 	aim
�� �� ֵ��	TRUE(��)/FALSE(û��)
*******************************************************************************/
u8 Check_BandAddr(CMDAim_t *aim)
{
	u8 i;
	
	if(gSysData.BandAreaEn == 1) //����ת�������ѿ���
	{
		if (aim->objectType >= GROUP_ACT) //����ǹ㲥���ͣ����е�ָ�����ת��
		{
			if(Check_AreaEx(aim->object.area))	//ƥ������
			{
				return TRUE;
			}
		}
	}
	
	for(i = 0; i < 5; i++)
	{
		if(!gSysData.BandAddr[i] || gSysData.BandAddr[i] == 0xFFFFFFFF)//Ϊ0�Ļ����ް󶨹��ܣ�������һ��
		{
			continue;
		}

		if((aim->sourceID == gSysData.BandAddr[i])//�жϵ�i���Ƿ���·��ת�����ܣ�
			|| (aim->object.id == gSysData.BandAddr[i]))
		{
			return TRUE;
		}
	}
	return FALSE;
}
#endif 

/**************************Copyright BestFu 2014-05-14*************************/
