/***************************Copyright BestFu 2014-05-14*************************
��	����	Linksimplest.c
˵	����	�������Դ�ļ�
��	�룺	Keil uVision4 V4.54.0.0
��	����	v1.0
��	д��	Joey
��	�ڣ�	2013.7.22 
�ޡ��ģ�	Unarty 2014.11.11 ALinkInit�н�EEPROM��һ��ʹ���ж�ȥ��
*******************************************************************************/

#include "Linkall.h"

#if (ALINK_SRC_VALUE_EN > 0)
ALinkVTable  ALinkV;    //������ִ��ֵ�������ж���ͬ�����Ƿ�ִ�й�?
//SrcTypeTable SrcType;   //������Դ��һ�εĲ���ֵ
#endif

#if (ALINK_EN > 0)
ALinkTable   ALink;   //���������RAMӳ�����

/*******************************************************************************
�� �� ����	ALinkInit
����˵���� 	���������ĳ�ʼ��
��	  ���� 	void
�� �� ֵ��	void
*******************************************************************************/
void ALinkInit(void)
{
	if (FALSE == ReadDataFromEEPROM(GetALinkStartAddr(), sizeof(ALinkTable), 
							(u8 *)&ALink))
	{
		return;
	}
		
#if (ALINKTIME_EN > 0)
	ALinkTimeInit();
#endif
	if(!ALink.Cnt)
	{
#if (ALINKTIME_EN > 0)
		AllALinkTimeClr();
#endif
	}
	
	if(ALink.Cnt > MAX_ALINKITEM)
	{
		ALinkClr();
#if (ALINKTIME_EN > 0)
		AllALinkTimeClr();
#endif
	}
	
#if (ALINK_SRC_VALUE_EN > 0)
	ALinkVInit();
#endif
}

/*******************************************************************************
�� �� ����	ALinkClr
����˵���� 	������������
��	  ���� 	void
�� �� ֵ��	void
*******************************************************************************/
void ALinkClr(void)
{
	u8 data;
	data = EEPROMINITED;
	WriteDataToEEPROM(GetALinkInited(), 1, &data);
	if(ALink.Cnt != 0)
	{
		memset((u8 *)&ALink, 0, sizeof(ALinkTable));
		WriteDataToEEPROM(GetALinkStartAddr(), sizeof(ALinkTable), (u8 *)&ALink);
	}
}

/*******************************************************************************
�� �� ����	ChkALinkIsExist
����˵���� 	ƥ���������������������Ƿ���ڣ�������ڰ������Ŵ���linknumָ��
��	  ���� 	 *linknum-�����������ҵ��������Ŵ���ڴ�
             *data-��������
�� �� ֵ��	1��ʾ�ɹ���0��ʾʧ��
*******************************************************************************/
u8 ChkALinkIsExist(u8 *linknum, u8 *data)
{
	u8 i,cnt;
	
	for(i=0;i<ALink.Cnt;i++)
	{
		if(ALink.linkitem[i].UseFlag)
		{// ƥ���Ƿ�������ͬĿ��?ǰ2�ֽںͺ���13����������Ҫƥ��(������Ҳƥ�䣬ʶ��һģһ��������)
			cnt = sizeof(ALinkHead)- 2 + ALink.linkitem[i].DestLen;
			if(!memcmp(data, &ALink.linkitem[i].SUnitID, cnt))
			{
				*linknum = ALink.linkitem[i].LinkNum;
				return TRUE;
			}
		}
	}
	return FALSE;
}

/*******************************************************************************
�� �� ����	GetALinkNum
����˵���� 	��ȡ�����ţ���С����˳������С��ֵΪ����������
��	  ���� 	*linknum-�ҵ�����С�����Ŵ���ڴ�
�� �� ֵ��	1��ʾ�ɹ���0��ʾʧ��
*******************************************************************************/
u8 GetALinkNum(u8 *linknum)
{
	u8 i,j;
	for(i=1;i<254;i++)//�����ŷ�Χ1-254
	{
		for(j=0;j<ALink.Cnt;j++)
		{
			if(ALink.linkitem[j].UseFlag)
			{
				if(i == ALink.linkitem[j].LinkNum)
				{
					break;
				}
			}
		}
		if(j == ALink.Cnt)
		{
			*linknum = i;
			return TRUE;
		}
	}
	return FALSE;
}

/*******************************************************************************
�� �� ����	QueryALink
����˵���� 	��ѯdata������������������е�λ��
��	  ���� 	num-Ҫ��ѯ��������
�� �� ֵ��	EXEERROR��ʾû�ҵ���0-254��ʾ�ҵ���ʵ��λ��
*******************************************************************************/
u8 QueryALink(u8 num)
{
	u8 i;
	for(i=0;i<ALink.Cnt;i++)
	{
		if(ALink.linkitem[i].UseFlag)
		{
			if(num == ALink.linkitem[i].LinkNum)
			{
				return i;
			}
		}
	}
	return EXEERROR;
}

/*******************************************************************************
�� �� ����	CheckALinkFull
����˵���� 	�������������Ƿ���
��	  ���� 	void
�� �� ֵ��	1��ʾ������0��ʾ����
*******************************************************************************/
u8 CheckALinkFull(void)
{
	if(ALink.Cnt >= MAX_ALINKITEM)
		return TRUE;
	else
		return FALSE;
}

/*******************************************************************************
�� �� ����	ReadALink
����˵���� 	������������ж�ȡ����
��	  ���� 	num-Ҫ��ѯ������λ�ã���ȡ�����������ݴ����linkָ��
            *len-��ȡ�������ݳ���
            *data-��ȡ��������
�� �� ֵ��	1��ʾ��ȡ�ɹ���0��ʾ��ȡʧ��
*******************************************************************************/
u8 ReadALink(u8 num , u8 *len, u8 *data)
{
	u8 cnt;
	cnt = sizeof(ALinkHead) - 2 + ALink.linkitem[num].DestLen;
	memcpy(data, (u8 *)&ALink.linkitem[num].SUnitID, cnt);
	*len = cnt;
	return TRUE;
}

/*******************************************************************************
�� �� ����	WriteALink
����˵���� 	������д�������������
��	  ���� 	num-Ҫд�������λ�ã���dataָ������д��numλ��
�� �� ֵ��	1��ʾд��ɹ���0��ʾд��ʧ��
*******************************************************************************/
u8 WriteALink(u8 num, u8 linknum, u8 addflag, u8 *data)
{
	ALinkItem *pLink;
	u8 cnt;
	pLink = (ALinkItem *)(data-2);
	if(num <= MAX_ALINKITEM)		//2014.10.08 Unarty Change ������������ֻ����49������50��������
	{
		cnt = sizeof(ALinkHead)-2+pLink->DestLen;
		memcpy(&ALink.linkitem[num].SUnitID, data, cnt);
		ALink.linkitem[num].UseFlag = 1;
		ALink.linkitem[num].LinkNum = linknum;
		WriteDataToEEPROM(GetALinkStartAddr() + sizeof(ALinkItem)*num, 
				sizeof(ALinkItem), (u8 *)&ALink.linkitem[num]);

		if(addflag)
		{
			ALink.Cnt++;
			WriteDataToEEPROM(GetALinkStartAddr() + ((u32)&ALink.Cnt - (u32)&ALink), 
							sizeof(u8), &ALink.Cnt);
		}
#if (ALINK_SRC_VALUE_EN > 0)
		AddALinkValue(ALink.linkitem[num].LinkNum);
#endif
		
		return TRUE;
	}
	return FALSE;
}

/*******************************************************************************
�� �� ����	DelALink
����˵���� 	�������������������ɾ��
��	  ���� 	num-Ҫɾ��������λ��
�� �� ֵ��	1��ʾɾ���ɹ���0��ʾɾ��ʧ��
*******************************************************************************/
u8 DelALink(u8 num)
{
	if(num != (ALink.Cnt-1))
	{
		memcpy((u8 *)&ALink.linkitem[num],
			(u8 *)&ALink.linkitem[ALink.Cnt-1],sizeof(ALinkItem));
		WriteDataToEEPROM(GetALinkStartAddr() + sizeof(ALinkItem)*num, 
				sizeof(ALinkItem), (u8 *)&ALink.linkitem[num]);
		memset((u8 *)&ALink.linkitem[ALink.Cnt-1], 0, sizeof(ALinkItem));
		WriteDataToEEPROM(GetALinkStartAddr() + sizeof(ALinkItem)*(ALink.Cnt-1), 
				sizeof(ALinkItem), (u8 *)&ALink.linkitem[num]);
	}
	else
	{
		memset((u8 *)&ALink.linkitem[num], 0, sizeof(ALinkItem));
		WriteDataToEEPROM(GetALinkStartAddr() + sizeof(ALinkItem)*num, 
				sizeof(ALinkItem), (u8 *)&ALink.linkitem[num]);
	}
	
#if (ALINK_SRC_VALUE_EN > 0)
	DelALinkValue(ALinkV.LinkV[num].LinkNum);
#endif
	
	if (ALink.Cnt > 0)
	{
		ALink.Cnt--;
		WriteDataToEEPROM(GetALinkStartAddr() + ((u32)&ALink.Cnt - (u32)&ALink), 
				sizeof(u8), (u8 *)&ALink.Cnt);
		return TRUE;
	}
	
	return FALSE;
}

#if (ALINK_SRC_VALUE_EN > 0)
/*****************************************************************************
  ������:  void ALinkVInit(void)
����˵��:  ����ִ��ֵ�ĳ�ʼ�������ж���ͬ�������Ƿ�ִ�й�?
    ����:  void
  ����ֵ:  void
*****************************************************************************/
void ALinkVInit(void)
{
   u8 i;
   for(i=0;i<ALink.Cnt;i++)
   {
	   ALinkV.LinkV[i].LinkNum = ALink.linkitem[i].LinkNum;
	   ALinkV.LinkV[i].Value   = 0;
	   ALinkV.LinkV[i].FirstFlag = 0 ; //���α������ֵΪ1 2015/09/10 yanhuan adding
	   ALinkV.LinkV[i].OldValue = 0;
   }
   ALinkV.cnt = ALink.Cnt;
}

/*****************************************************************************
  ������:  u8 GetALinkValue(u8 linknum)
����˵��:  ��ȡ�����ŵĴ���ֵ
    ����:  linknum(linkage number)��д
           ��ʾÿ�������ĺ��룬����ֵΨһ
  ����ֵ:  ������������ֵ0,1,2Ŀǰ�������������û�ҵ�������ʱ���ش���0xFF
*****************************************************************************/
u8 GetALinkValue(u8 linknum)
{
	u8 i;
	for(i=0;i<ALinkV.cnt;i++)
	{
		if(linknum == ALinkV.LinkV[i].LinkNum)
		{
			return ALinkV.LinkV[i].Value;
		}
	}
	return LINKERROR;
}

/*****************************************************************************
  ������:  u8 SetALinkValue(u8 linknum, u8 value)
����˵��:  ������������ֵ
    ����:  linknum(linkage number)��д
           ��ʾÿ�������ĺ��룬����ֵΨһ
           value,��ʾ��������ֵ
  ����ֵ:  1��ʾ�ɹ���0��ʾʧ��
*****************************************************************************/
u8 SetALinkValue(u8 linknum, u8 value)
{
	u8 i;
	for(i=0;i<ALinkV.cnt;i++)
	{
		if(linknum == ALinkV.LinkV[i].LinkNum)
		{
			ALinkV.LinkV[i].Value = value;
			return TRUE;
		}
	}
	return FALSE;
}

/*****************************************************************************
  ������:  u8 AddALinkValue(u8 linknum)
����˵��:  ������������ֵ��������е�һ��
    ����:  linknum(linkage number)��д
           ��ʾÿ�������ĺ��룬����ֵΨһ
  ����ֵ:  1��ʾ�ɹ���0��ʾʧ��
*****************************************************************************/
u8 AddALinkValue(u8 linknum)
{
	u8 num;
	num = FindALinkValueNum(linknum);
	
	if(num == EXEERROR)
	{
		if(ALinkV.cnt >= MAX_ALINKITEM)
		{
			return FALSE;
		}
		ALinkV.LinkV[ALinkV.cnt].LinkNum = linknum;
		ALinkV.LinkV[ALinkV.cnt].Value   = 0;
		ALinkV.LinkV[ALinkV.cnt].OldValue= 0;
		ALinkV.LinkV[ALinkV.cnt].FirstFlag = 0 ; //����ֵ�����󣬵�һ��ֻҪ�����������ɴ��� 2015/09/10 yanhuan adding
		ALinkV.cnt++;
	}
	else
	{
		ALinkV.LinkV[num].Value   = 0;
	}

	return TRUE;
}

/*****************************************************************************
  ������:  u8 FindALinkValueNum(u8 linknum)
����˵��:  ������������ֵ����������е�λ��
    ����:  linknum(linkage number)��д
           ��ʾÿ�������ĺ��룬����ֵΨһ
  ����ֵ:  0-254��ʾ����λ�ã�255��ʾû�ҵ�
*****************************************************************************/
u8 FindALinkValueNum(u8 linknum)
{
	u8 i;
	for(i=0;i<ALinkV.cnt;i++)
	{
		if(linknum == ALinkV.LinkV[i].LinkNum)
		{
			return i;
		}
	}
	return EXEERROR;
}

/*****************************************************************************
  ������:  u8 DelALinkValue(u8 linknum)
����˵��:  ɾ����������ֵ��������е�һ��
    ����:  linknum(linkage number)��д
           ��ʾÿ�������ĺ��룬����ֵΨһ
  ����ֵ:  1��ʾ�ɹ���0��ʾʧ��
*****************************************************************************/
u8 DelALinkValue(u8 linknum)
{
	u8 num;
	if((num = FindALinkValueNum(linknum)) == EXEERROR)
	{
		return FALSE;
	}
	if(num == ALinkV.cnt-1)
	{
		memset((u8 *)&ALinkV.LinkV[num], 0, sizeof(ALinkVItem));
	}
	else
	{
		memcpy((u8 *)&ALinkV.LinkV[num], 
			(u8 *)&ALinkV.LinkV[ALinkV.cnt-1], sizeof(ALinkVItem));
		memset((u8 *)&ALinkV.LinkV[ALinkV.cnt-1], 0, sizeof(ALinkVItem));
	}
	if(ALinkV.cnt > 0) //�޸�������Ϊ1��ʱɾ����������û�����ALinkV.cnt������ 2015/09/17 yanhuan adding
	{
		ALinkV.cnt--;
	}
	return TRUE;
}

/*****************************************************************************
  ������:  void DelAllALinkValue(void)
����˵��:  ɾ����������ֵ���е�Ԫ
    ����:  void
  ����ֵ:  void
*****************************************************************************/
void DelAllALinkValue(void)
{
	memset((u8 *)&ALinkV, 0, sizeof(ALinkVTable));
}
/*****************************************************************************
  ������:  void ALinkValueExe
����˵��:  ����ִ��
    ����:  temp--������
		   pSampleValue--����ֵ����ָ��
link--�������жϵı��  0:�����ϱ�������������ʹ�ã� 1:���ϱ�Ҳ����
  ����ֵ:  void
*****************************************************************************/
void ALinkValueExe(ALinkItem *temp,u8* pSampleValue,u8 link)
{
	int vtmp = 0 ;
	SetALinkValue(temp->LinkNum, link);
	//ִ������
	//ExeLinkPackage(data, LINKSTD);
	vtmp = DWCHAR( temp->DDevID[3],
					temp->DDevID[2],
					temp->DDevID[1],
					temp->DDevID[0]);
	if(!vtmp)//����λ����Լ������Ŀ��Ϊȫ0ʱ����ʾ�����ϱ���ǰ���������
	{
		Upload_Immediately(temp->SUnitID,temp->SPropID,pSampleValue);
		return ;
	}
	if(link)
		Msg_Send((ObjectType_t)temp->DType, 
			vtmp, 
			WCHAR(temp->DAddrT[1],temp->DAddrT[0]), 
			temp->UnitID, 
			temp->DestCmd, 
			temp->DestLen, 
			temp->data);
}
/*******************************************************************************
�� �� ����	CheckALinkValueAndExe
����˵���� 	�ж�ALINK������ֵ���Ƿ�������������������ִ��
��	  ����  *data-ʵ��������
            linknum-������
			newsamp-�µĲ���ֵ
�� �� ֵ��	1��ʾִ�гɹ���0��ʾִ��ʧ��
*******************************************************************************/
u8 CheckALinkValueAndExe(u8 *data, u8 linknum, int newsamp)
{
	int sv1,sv2,sv3,sv4;
	u8 screv,num;
	ALinkItem *atmp;

	atmp  = (ALinkItem *)data;
	screv = GetConditionReverse(atmp->SCondition);
	
	memcpy((u8 *)&sv1, (u8*)atmp->SValue, sizeof(int));
	sv3 = sv1;
	sv2 = 0;
	sv4 = 0;
	
	GetConditionOffset(atmp->SCondition, &sv1, &sv2);

	if((num = FindALinkValueNum(linknum)) == 0xFF)
	{
		return FALSE;
	}
	
	//�ж���һ���Ƿ񣬲���������
	if(ALinkV.LinkV[num].FirstFlag)
	{
		if(CmpSampAndLinkValue((CONDITION)(atmp->SCondition), sv1, sv2, newsamp))
		{
			if(CmpSampAndLinkValue((CONDITION)screv, sv1, sv2, ALinkV.LinkV[num].OldValue))
			{
				//�Ѵ���������?��ֹ����
				if(CLINKEXED != GetALinkValue(atmp->LinkNum))
				{
//					SetALinkValue(atmp->LinkNum, CLINKEXED);
					ALinkValueExe(atmp,(u8*)&newsamp,1);
				}
			}
//			ALinkV.LinkV[num].OldValue = newsamp;
//			return TRUE;
		}
		
		GetConditionOffset(screv, &sv3, &sv4);
		
		if(CmpSampAndLinkValue((CONDITION)screv, sv3, sv4, newsamp))
		{		
			//�ж���һ���Ƿ񣬲���������
			if(CmpSampAndLinkValue((CONDITION)(atmp->SCondition), sv3, sv4, ALinkV.LinkV[num].OldValue))
			{
	//			//���Կ���ִ�з�������
				if(0 != GetALinkValue(atmp->LinkNum))
				{			
//					SetALinkValue(atmp->LinkNum, 0);//��ִ�б�־
					ALinkValueExe(atmp,(u8*)&newsamp,0);
				}
			}
		}
	}
	else //δ��������������δִ�й�
	{
		ALinkV.LinkV[num].FirstFlag = 1;
		if(CmpSampAndLinkValue((CONDITION)(atmp->SCondition), *(u32*)atmp->SValue, 0, newsamp))
		{
//			SetALinkValue(atmp->LinkNum, CLINKEXED);//��ִ�б��
			ALinkValueExe(atmp,(u8*)&newsamp,1);
		}
		else
		{
//			SetALinkValue(atmp->LinkNum, 0);//��ִ�б�־			
			ALinkValueExe(atmp,(u8*)&newsamp,0);
		}		
	}
	
	ALinkV.LinkV[num].OldValue = newsamp;//������һ��ֵ
	return TRUE;
}
#endif
#endif
/**************************Copyright BestFu 2014-05-14*************************/
