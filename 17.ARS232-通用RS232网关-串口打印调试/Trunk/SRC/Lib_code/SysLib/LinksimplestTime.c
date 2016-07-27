/***************************Copyright BestFu 2014-05-14*************************
��	����	LinksimplestTime.c
˵	����	�����������ʱԴ�ļ�
��	�룺	Keil uVision4 V5.12.0.0
��	����	v1.0
��	д��	Joey
��	�ڣ�	2016.1.29 
�ޡ��ģ�	
*******************************************************************************/

#include "Linkall.h"

#if (ALINKTIME_EN > 0)
ALinkTimeTable   ALinkTime;   //���������RAMӳ�����

/*******************************************************************************
�� �� ����	ALinkTimeInit
����˵���� 	�����������ʱ���ܵĳ�ʼ��
��	  ���� 	void
�� �� ֵ��	void
*******************************************************************************/
void ALinkTimeInit(void)
{
	if (FALSE == ReadDataFromEEPROM(GetALinkTimeStartAddr(), sizeof(ALinkTimeTable), 
							(u8 *)&ALinkTime))
	{
		return;
	}
	if(ALinkTime.Cnt > MAX_ALINKTIMEITEM)
	{
		AllALinkTimeClr();
	}
}

/*******************************************************************************
�� �� ����	AllALinkTimeClr
����˵���� 	���������������ʱʱ��
��	  ���� 	void
�� �� ֵ��	void
*******************************************************************************/
void AllALinkTimeClr(void)
{
	if(ALinkTime.Cnt != 0)
	{
		memset((u8 *)&ALinkTime, 0, sizeof(ALinkTimeTable));
		WriteDataToEEPROM(GetALinkTimeStartAddr(), sizeof(ALinkTimeTable), (u8 *)&ALinkTime);
	}
}

/*******************************************************************************
�� �� ����	QueryALinkTime
����˵���� 	��ѯnum����������ʱʱ����е�λ��
��	  ���� 	num-Ҫ��ѯ��������
�� �� ֵ��	EXEERROR��ʾû�ҵ���0-254��ʾ�ҵ���ʵ��λ��
*******************************************************************************/
u8 QueryALinkTime(u8 num)
{
	u8 i;
	for(i=0;i<ALinkTime.Cnt;i++)
	{
		if(ALinkTime.Item[i].UseFlag)
		{
			if(ALinkTime.Item[i].LinkNum == num)
			{
				return i;
			}
		}
	}
	return EXEERROR;
}

/*******************************************************************************
�� �� ����	DelALinkTime
����˵���� 	����ʱʱ�����ʱʱ�����ɾ��
��	  ���� 	num-Ҫɾ��������λ��
�� �� ֵ��	1��ʾɾ���ɹ���0��ʾɾ��ʧ��
*******************************************************************************/
u8 DelALinkTime(u8 num)
{
	if(num != (ALinkTime.Cnt-1))
	{
		memcpy((u8 *)&ALinkTime.Item[num],
			(u8 *)&ALinkTime.Item[ALinkTime.Cnt-1],sizeof(ALinkTimeItem));
		WriteDataToEEPROM(GetALinkTimeStartAddr() + sizeof(ALinkTimeItem)*num, 
				sizeof(ALinkTimeItem), (u8 *)&ALinkTime.Item[num]);
		
		memset((u8 *)&ALinkTime.Item[ALinkTime.Cnt-1], 0, sizeof(ALinkTimeItem));
		WriteDataToEEPROM(GetALinkTimeStartAddr() + sizeof(ALinkTimeItem)*(ALinkTime.Cnt-1), 
				sizeof(ALinkTimeItem), (u8 *)&ALinkTime.Item[num]);
	}
	else
	{
		memset((u8 *)&ALinkTime.Item[num], 0, sizeof(ALinkTimeItem));
		WriteDataToEEPROM(GetALinkTimeStartAddr() + sizeof(ALinkTimeItem)*num, 
				sizeof(ALinkTimeItem), (u8 *)&ALinkTime.Item[num]);
	}
	
	if (ALinkTime.Cnt > 0)
	{
		ALinkTime.Cnt--;
		WriteDataToEEPROM(GetALinkTimeStartAddr() + ((u32)&ALinkTime.Cnt - (u32)&ALinkTime), 
				sizeof(u8), (u8 *)&ALinkTime.Cnt);
		return TRUE;
	}

	return FALSE;
}

/*******************************************************************************
�� �� ����	WriteALinkTime
����˵���� 	����ʱʱ��д����ʱʱ�����
��	  ���� 	num-Ҫд���λ��
						linknum-������
            addflag-��ʾ����
            *data-Ҫд�������
�� �� ֵ��	1��ʾд��ɹ���0��ʾд��ʧ��
*******************************************************************************/
u8 WriteALinkTime(u8 num, u8 linknum, u8 addflag, u8 *data)
{
	if(num <= MAX_ALINKTIMEITEM)
	{
		memcpy(&ALinkTime.Item[num].Time.delaytime[0], data, sizeof(DlyTimeData_t));
		ALinkTime.Item[num].UseFlag = 1;
		ALinkTime.Item[num].LinkNum = linknum;
		
		WriteDataToEEPROM(GetALinkTimeStartAddr() + sizeof(ALinkTimeItem)*num, 
				sizeof(ALinkTimeItem), (u8 *)&ALinkTime.Item[num]);

		if(addflag)
		{
			ALinkTime.Cnt++;
			WriteDataToEEPROM(GetALinkTimeStartAddr() + ((u32)&ALinkTime.Cnt - (u32)&ALinkTime), 
							sizeof(u8), &ALinkTime.Cnt);
		}
		return TRUE;
	}
	return FALSE;
}

/*******************************************************************************
�� �� ����	AddALinkTimeCmdX1
����˵���� 	��ʵ�Ƿ�����ʱִ�еĶ����������ƴ��0x91ָ����ⷢ��
��	  ���� 	num-��ʾλ��
            *data-Ҫƴ��0X91ָ�����������
�� �� ֵ��	1��ʾ��ȡ�ɹ���0��ʾ��ȡʧ��
*******************************************************************************/
u8 AddALinkTimeCmdX1(u8 num, u8 *data)
{
	u8 cnt,time[5]={0};
	ALinkItem *pLink;
	pLink = (ALinkItem *)data;
	
	memmove(&pLink->data[0], &pLink->UnitID, pLink->DestLen + 3);
	
	ReadALinkTime(num , &cnt, &time[0]);
	memcpy(&pLink->data[pLink->DestLen + 3], &time[0], sizeof(DlyTimeData_t));
	
	pLink->UnitID  = 0;
	pLink->DestCmd = 0x91;
	pLink->DestLen += 3;
	return TRUE;
}

/*******************************************************************************
�� �� ����	ReadALinkTime
����˵���� 	������������ж�ȡ����
��	  ���� 	num-Ҫ��ѯ������λ�ã���ȡ�����������ݴ����linkָ��
            *len-��ȡ�������ݳ���
            *data-��ȡ��������
�� �� ֵ��	1��ʾ��ȡ�ɹ���0��ʾ��ȡʧ��
*******************************************************************************/
u8 ReadALinkTime(u8 num , u8 *len, u8 *data)
{
	
	memcpy(data, (u8 *)&ALinkTime.Item[num].Time.delaytime[0], 5);
	*len = 5;
	return TRUE;
}

/*******************************************************************************
�� �� ����	WriteALinkTimeToTable
����˵���� 	����ʱʱ��д����ʱʱ����е�numλ��
��	  ���� 	type����������
            num��ʾ����λ�ã�Ҫд������������dataָ��
�� �� ֵ��	255��ʾ�쳣��1��ʾ�ɹ���0��ʾʧ��
*******************************************************************************/
u8 WriteALinkTimeToTable(u8 num, u8 linknum, u8 addflag, u8 *data)
{
	return WriteALinkTime(((num==EXEERROR)?ALinkTime.Cnt:num), linknum, addflag, data);
}

/*******************************************************************************
�� �� ����	DelALinkTimeFromTable
����˵���� 	����ʱʱ�����ʱʱ����е�numλ��ɾ��
��	  ���� 	type����������
            num��ʾ��ʱʱ�������λ��
�� �� ֵ��	255��ʾ�쳣��1��ʾ�ɹ���0��ʾʧ��
*******************************************************************************/
u8 DelALinkTimeFromTable(u8 num)
{
	return DelALinkTime(num);
}

///*******************************************************************************
//�� �� ����	CheckALinkTimeFull
//����˵���� 	������������ʱ���Ƿ���
//��	  ���� 	void
//�� �� ֵ��	1��ʾ������0��ʾ����
//*******************************************************************************/
//u8 CheckALinkTimeFull(void)
//{
//	if(ALinkTime.Cnt >= MAX_ALINKTIMEITEM)
//		return TRUE;
//	else
//		return FALSE;
//}


///*******************************************************************************
//�� �� ����	ChkALinkIsExist
//����˵���� 	ƥ���������������������Ƿ���ڣ�������ڰ������Ŵ���linknumָ��
//��	  ���� 	linknum-Ҫƥ��ʱ���������
//             *data-��������
//�� �� ֵ��	1��ʾ�ɹ���0��ʾʧ��
//*******************************************************************************/
//u8 ChkALinkTimeIsExist(u8 linknum, u8 *data)
//{
//	u8 i,cnt;
//	
//	for(i=0;i<ALinkTime.Cnt;i++)
//	{
//		if(ALinkTime.Item[i].UseFlag)
//		{
//			if(ALinkTime.Item[i].linknum == linknum)
//			{
//				if(!memcmp(data, (u8 *)&ALinkTime.Item[i].Time, sizeof(DlyTimeData_t)))
//				{
//					memcpy((u8 *)&ALinkTime.Item[i].Time, data, sizeof(DlyTimeData_t));
//					return TRUE;
//				}
//			}
//		}
//	}
//	return FALSE;
//}
///*******************************************************************************
//�� �� ����	ALinkTimeClrForLinknum
//����˵���� 	����ʱʱ����������������ȵ���ʱʱ��
//��	  ���� 	linknum-Ҫƥ��ʱ���������
//�� �� ֵ��	void
//*******************************************************************************/
//void ALinkTimeClr(void)
//{
//	if(ALinkTime.Cnt != 0)
//	{
//		memset((u8 *)&ALinkTime, 0, sizeof(ALinkTimeTable));
//		WriteDataToEEPROM(GetALinkTimeStartAddr(), sizeof(ALinkTimeTable), (u8 *)&ALinkTime);
//	}
//	
//	u8 i,cnt;
//	
//	for(i=0;i<ALinkTime.Cnt;i++)
//	{
//		if(ALinkTime.Item[i].UseFlag)
//		{
//			if(ALinkTime.Item[i].linknum == linknum)
//			{
//				if(!memcmp(data, (u8 *)&ALinkTime.Item[i].Time, sizeof(DlyTimeData_t)))
//				{
//					memcpy((u8 *)&ALinkTime.Item[i].Time, data, sizeof(DlyTimeData_t));
//					return TRUE;
//				}
//			}
//		}
//	}
//	return FALSE;
//}
///*******************************************************************************
//�� �� ����	ReadALinkTime
//����˵���� 	���������ʱ����ж�ȡ����ʱ��
//��	  ���� 	linknum-Ҫ��ѯ��������
//            *len-��ȡ�������ݳ���,�̶�Ϊ5���ֽڣ����ڱ������ݳ���
//            *data-��ȡ��������
//�� �� ֵ��	1��ʾ��ȡ�ɹ���0��ʾ��ȡʧ��
//*******************************************************************************/
//u8 ReadALinkTime(u8 linknum , u8 *len, u8 *data)
//{
//	u8 i;
//	
//	for(i=0; i<ALinkTime.Cnt; i++)
//	{
//		if(ALinkTime.Item[i].UseFlag && ALinkTime.Item[i].linknum == linknum)
//		{
//				*data = TIMEFLAG;
//				memcpy(data+1, (u8 *)&ALinkTime.Item[i].time, sizeof(DlyTimeData_t));
//				break;
//		}
//	}
//	*len = 6;
//	return TRUE;
//}



///*******************************************************************************
//�� �� ����	DelALinkTimeItem
//����˵���� 	��������ɾ����ʱʱ����ж�Ӧ����ʱʱ��
//��	  ���� 	
//            num-�����ţ�Ψһ
//�� �� ֵ��	1��ʾ�ɹ���0��ʾʧ��
//*******************************************************************************/
//u8 DelALinkTimeItem(u8 num)
//{
//	u8 number;
//	
//	if(num == 0xFF)
//	{
//		return ALinkTimeClr();
//	}
//	
//	if((number = QueryALinkTime(num)) != 0xFF)
//	{
//		return DelALinkTimeFromTable(type, number);
//	}
//	else 		//δ�ҵ���������
//	{
//		return TRUE;
//	}
//}
#endif
/**************************Copyright BestFu 2014-05-14*************************/
