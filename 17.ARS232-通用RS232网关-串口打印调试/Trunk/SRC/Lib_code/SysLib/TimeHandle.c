/***************************Copyright BestFu 2016-01-28*************************
�� ����    TimeHandle.c
˵ ����    ʱ�䴦����
�� �룺    Keil uVision V5.12.0.0
�� ����    v1.0
�� д��    jay
�� �ڣ�    2016-03-09
��  ��:    ��
*******************************************************************************/
#include "TimeHandle.h"

#if ((ALINKTIME_EN > 0u) || (SCENCETIME_EN > 0u))
//variable define

//function declaration
static u8 CheckTimePara(u16 time);
static u8 CheckTimeParaIsOK(u16 delaytime,u16 keeptime,u8 repeat);

/*******************************************************************************
�� �� ����  u32 TransformTimePara(u16* time)
����˵����  ����ʱ����ʱ��ת������
��   ����   time :��ʱ����ʱ��
�� �� ֵ��  ת�������ʱ��
˵    ����  ��
*******************************************************************************/
u32 TransformTimePara(u16* time)
{
	u32 return_result = 0u;
	TimeAnalyze_t *pTimeAnalyze = (TimeAnalyze_t*)(time);
	
	switch(pTimeAnalyze->SecAnalyze.flag)
	{
		case 0:
		{
			return_result = (pTimeAnalyze->SecAnalyze.second) * 10 +\
							(pTimeAnalyze->SecAnalyze.minute * 600) +\
							(pTimeAnalyze->SecAnalyze.hour * 36000);
		}break;
		case 1:
		{
			return_result = (pTimeAnalyze->MinAnalyze.minute * 600) +\
							(pTimeAnalyze->MinAnalyze.hour * 36000) +\
							(pTimeAnalyze->MinAnalyze.day * 864000);
		}break;
		case 2:
		{
			return_result = (pTimeAnalyze->HourAnalyze.hour * 36000) +\
							(pTimeAnalyze->HourAnalyze.day * 864000);
		}break;	
		case 3:
		{
			return_result = (pTimeAnalyze->MsecAnalyze.msec) +\
							(pTimeAnalyze->MsecAnalyze.second * 10) + 
							(pTimeAnalyze->MsecAnalyze.minute * 600);
		}
		default:
			break;
	}
	return (return_result);
}

/*******************************************************************************
�� �� ����  static u8 CheckTimePara(u16 time)
����˵����  ���ʱ������Ϸ���
��   ����   time	: ������ʱ�����
�� �� ֵ��  �����(TRUE:��ʾʱ�������ȷ,FALSE����ʾʱ��������Ϸ�)
*******************************************************************************/
static u8 CheckTimePara(u16 time)
{
	TimeAnalyze_t* pTimeAnalyze = (TimeAnalyze_t *)(&time);
	switch(((TimeAnalyze_t *)(&time))->SecAnalyze.flag)
	{
		case 0:
		{
			if((pTimeAnalyze->SecAnalyze.second > 59)||\
				(pTimeAnalyze->SecAnalyze.minute > 59))
			{
				return FALSE;
			}
		}break;
		case 1:
		{
			if((pTimeAnalyze->MinAnalyze.hour > 23)||\
				(pTimeAnalyze->MinAnalyze.minute > 59))
			{
				return FALSE;
			}
		}break;
		case 2:
		{
			if((pTimeAnalyze->HourAnalyze.hour > 23))
			{
				return FALSE;
			}
		}break;
		case 3:
		{
			if((pTimeAnalyze->MsecAnalyze.msec > 9)||\
				(pTimeAnalyze->MsecAnalyze.second > 59))
			{
				return FALSE;
			}
		}break;
		default:
			return FALSE;
	}
	return TRUE;
}

/*******************************************************************************
�� �� ����  u16 CheckTimeParaIsZero(u16 timepara)
����˵����  ���ȫ��ʱ������Ƿ�Ϊ��
��   ����   timepara	: ������ʱ�����
�� �� ֵ��  �����(��ʱ���ڲ���Ϊ���򷵻�Ϊ��)
*******************************************************************************/
u16 CheckTimeParaIsZero(u16 timepara)
{
	return ((timepara & 0x3fff) > 0);
}

/*******************************************************************************
�� �� ����  static u8 CheckTimeParaIsOK(u16 delaytime,u16 keeptime,u8 repeat)
����˵����  ���ȫ��ʱ������Ϸ���
��   ����   delaytime	: ��������ʱʱ�����
			keeptime	: �����ı���ʱ�����
			repeat		: �������ظ���������
�� �� ֵ��  �����(TRUE:��ʾʱ�������ȷ,FALSE����ʾʱ��������Ϸ�)
*******************************************************************************/
static u8 CheckTimeParaIsOK(u16 delaytime,u16 keeptime,u8 repeat)
{
	if((0 == CheckTimeParaIsZero(delaytime))&&\
		((CheckTimeParaIsZero(keeptime) > 0)||(repeat > 0))
	  )
	{
		return FALSE;
	}
	if(FALSE == CheckTimePara(delaytime))
	{
		return FALSE;
	}
	if(FALSE == CheckTimePara(keeptime))
	{
		return FALSE;
	}
	return TRUE;
}

/*******************************************************************************
�� �� ����  u8 CheckTimeParaIsOK(u16 delaytime,u16 keeptime,u8 repeat)
����˵����  ���ȫ��ʱ������Ϸ���
��   ����   time		: ��������ʱ����ʱ�����ָ��
			cnt			: �����Ĳ����ĸ���
�� �� ֵ��  �����(TRUE:��ʾʱ�������ȷ,FALSE����ʾʱ��������Ϸ�)
*******************************************************************************/
u8 CheckAllTimeParaIsOK(DlyTimeData_t* time,u8 cnt)
{
	u8 i;
	for(i = 0;i < cnt;i ++)
	{
		if(FALSE == CheckTimeParaIsOK(*(u16*)time->delaytime,*(u16*)time->keeptime,*(u8*)&time->repeat))
		{
			return FALSE;
		}
		time ++;
	}
	return TRUE;
}


#endif
/******************************End of File************************************/
