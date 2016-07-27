/***************************Copyright BestFu 2014-05-14*************************
��	����	LinkWRDC.c
˵	����	�����д�롢��ȡ��ɾ���Ͳ�ѯ����Դ�ļ�
��	�룺	Keil uVision4 V4.54.0.0
��	����	v1.0
��	д��	Joey
��	�ڣ�	2013.7.22 
�ޡ��ģ�	����
*******************************************************************************/
#include "linkall.h"
/*******************************************************************************
�� �� ����	CheckLinkFull
����˵���� 	����������Ƿ�����?
��	  ���� 	type����������
�� �� ֵ��	255��ʾ�쳣��1��ʾ�ɹ���0��ʾʧ��
*******************************************************************************/
#if (ALINK_EN > 0 || SLINK_EN > 0 || CLINK_EN > 0)
u8 CheckLinkFull(u8 type)
{
	switch(type)
	{
#if(ALINK_EN > 0)
		case SIMPLESTLINK: return CheckALinkFull(); 
#endif
#if(SLINK_EN > 0)
		case SIMPLELINK:   return CheckSLinkFull();
#endif
#if(CLINK_EN > 0)
		case COMPLEXLINK:  return CheckCLinkFull();
#endif
		default:		   return EXEERROR;
	}
}

/*******************************************************************************
�� �� ����	QuerySLink
����˵���� 	��ѯdata�������ڼ��������е�λ��
��	  ���� 	data-Ҫ��ѯ��������,type����������
�� �� ֵ��	255��ʾ�쳣��1��ʾ�ɹ���0��ʾʧ��
*******************************************************************************/
u8 QueryLink(u8 type, u8 num)
{
	switch(type)
	{
#if(ALINK_EN > 0)
		case SIMPLESTLINK: return QueryALink(num); 
#endif
#if(SLINK_EN > 0)
		case SIMPLELINK:   return QuerySLink(num); 
#endif
#if(CLINK_EN > 0)
		case COMPLEXLINK:  return QueryCLink(num); 
#endif
		default:           return EXEERROR;
	}
}

/*******************************************************************************
�� �� ����	QueryAllLinkFromTable
����˵���� 	��ѯ�������е�����������
��	  ���� 	type���������ͣ���ѯ���������Ŵ����dataָ��
�� �� ֵ��	255��ʾ�쳣��1��ʾ�ɹ���0��ʾʧ��
*******************************************************************************/
u8 QueryAllLinkFromTable(u8 type, u8 *len, u8 *data)
{
	u8 i;
	switch(type)
	{
#if(ALINK_EN > 0)
		case SIMPLESTLINK:
			for(i=0;i<ALink.Cnt;i++)
			{
				if(ALink.linkitem[i].UseFlag)
				{
					data[i] = ALink.linkitem[i].LinkNum;
				}
			}
			*len = ALink.Cnt;
			return TRUE;
#endif
#if(SLINK_EN > 0)
		case SIMPLELINK:
			for(i=0;i<SLink.Cnt;i++)
			{
				if(SLink.linkitem[i].UseFlag)
				{
					data[i] = SLink.linkitem[i].LinkNum;
				}
			}
			*len = SLink.Cnt;
			return TRUE;
#endif
#if(CLINK_EN > 0)
		case COMPLEXLINK:
			for(i=0;i<CLink.Cnt;i++)
			{
				if(CLink.linkitem[i].UseFlag)
				{
					data[i] = CLink.linkitem[i].LinkNum;
				}
			}
			*len = CLink.Cnt;
			return TRUE;
#endif
		default:           
			return EXEERROR;
	}
}

/*******************************************************************************
�� �� ����	ReadLinkFromTable
����˵���� 	�����������������Ŷ�ȡ����
��	  ���� 	type����������
            num��ʾ����λ�ã���ȡ�������������dataָ��
�� �� ֵ��	255��ʾ�쳣��1��ʾ�ɹ���0��ʾʧ��
*******************************************************************************/
u8 ReadLinkFromTable(u8 type, u8 num, u8 *len, u8 *data)
{
	switch(type)
	{
#if(ALINK_EN > 0)
		case SIMPLESTLINK: return ReadALink(num,len,data); 
#endif
#if(SLINK_EN > 0)
		case SIMPLELINK:   return ReadSLink(num,len,data); 
#endif
#if(CLINK_EN > 0)
		case COMPLEXLINK:  return ReadCLink(num,len,data); 
#endif
		default:           return EXEERROR;
	}
}

/*******************************************************************************
�� �� ����	WriteLinkToTable
����˵���� 	������д���������е�numλ��
��	  ���� 	type����������
            num��ʾ����λ�ã�Ҫд������������dataָ��
�� �� ֵ��	255��ʾ�쳣��1��ʾ�ɹ���0��ʾʧ��
*******************************************************************************/
u8 WriteLinkToTable(u8 type, u8 num, u8 linknum, u8 addflag, u8 *data)
{
	switch(type)
	{
#if(ALINK_EN > 0)
		case SIMPLESTLINK: 
			if(WriteALink(((num==EXEERROR)?ALink.Cnt:num), linknum, addflag, data))
			{
				return TRUE;
			}
			else
			{
				return FALSE;
			}
#endif
#if(SLINK_EN > 0)
		case SIMPLELINK:
			if(WriteSLink(((num==EXEERROR)?SLink.Cnt:num), linknum, addflag, data))
			{
				return TRUE;
			}
			else
			{
				return FALSE;
			}
#endif
#if(CLINK_EN > 0)
		case COMPLEXLINK:
			if(WriteCLink(((num==EXEERROR)?CLink.Cnt:num), linknum, addflag, data))
			{
				return TRUE;
			}
			else
			{
				return FALSE;
			}
#endif
		default:           
			return EXEERROR;
	}
}

/*******************************************************************************
�� �� ����	DelAllLinkFromTable
����˵���� 	ɾ��type���͵���������
��	  ���� 	type����������
�� �� ֵ��	255��ʾ�쳣��1��ʾ�ɹ���0��ʾʧ��
*******************************************************************************/
u8 DelAllLinkFromTable(u8 type)
{
	switch(type)
	{
#if(ALINK_EN > 0)
		case SIMPLESTLINK:
			memset((u8 *)&ALink, 0, sizeof(ALinkTable));
	#if (ALINK_SRC_VALUE_EN > 0)
			memset((u8 *)&ALinkV,0,sizeof(ALinkVTable));
	#endif
			WriteDataToEEPROM(GetALinkStartAddr(), 
				(u32)sizeof(ALinkTable), (u8 *)&ALink);
			return TRUE;
#endif
#if(SLINK_EN > 0)
		case SIMPLELINK:
			memset((u8 *)&SLink, 0, sizeof(SLinkTable));
			WriteDataToEEPROM(GetSLinkStartAddr(), 
				(u32)sizeof(SLinkTable), (u8 *)&SLink);
			DelAllSLinkValue();//ˢ�������Ƿ�ִ�й���
			SLinkSrcInit();//ɾ������Դִ�б�
			return TRUE;
#endif
#if(CLINK_EN > 0)
		case COMPLEXLINK:
			memset((u8 *)&CLink, 0, sizeof(CLinkTable));
			WriteDataToEEPROM(GetCLinkStartAddr(), 
				(u32)sizeof(CLinkTable), (u8 *)&CLink);
			DelAllCLinkSrc();//ˢ�¸���������
			return TRUE;
#endif
		default:           
			return EXEERROR;
	}
}

/*******************************************************************************
�� �� ����	DelAllLinkFromTable
����˵���� 	ɾ��type���͵�numλ������
��	  ���� 	type����������
�� �� ֵ��	255��ʾ�쳣��1��ʾ�ɹ���0��ʾʧ��
*******************************************************************************/
u8 DelLinkFromTable(u8 type, u8 num)
{
	switch(type)
	{
#if(ALINK_EN > 0)
		case SIMPLESTLINK: 
			if(num < MAX_ALINKITEM)
			{
				return DelALink(num);
			}
			else
				return FALSE;
#endif
#if(SLINK_EN > 0)
		case SIMPLELINK:
			if(num < MAX_SLINKITEM)
			{
				return DelSLink(num);
			}
			else
				return FALSE;
#endif
#if(CLINK_EN > 0)
		case COMPLEXLINK:
			if(num < MAX_CLINKITEM)
			{
				return DelCLink(num);
			}
			else
				return FALSE;
#endif
		default:           
			return EXEERROR;
	}
}

/*******************************************************************************
�� �� ��:  u8 ChkLinkIsExist(u8 type, u8 *linknum, u8 *data)
����˵��:  ��type�����в����Ƿ������ͬ�����������ڵĻ���linknum��ֵ
��    ��:  *linknum-�ҵ�����С�����Ŵ���ڴ�
           type-����
           *data-��������
�� �� ֵ:  1��ʾ�ɹ���0��ʾʧ��
*******************************************************************************/
u8 ChkLinkIsExist(u8 type, u8 *linknum, u8 *data)
{
	switch(type)
	{
#if(ALINK_EN > 0)
		case SIMPLESTLINK:
			return ChkALinkIsExist(linknum, data);
#endif
#if(SLINK_EN > 0)
		case SIMPLELINK:
			return ChkSLinkIsExist(linknum, data);
#endif
#if(CLINK_EN > 0)
		case COMPLEXLINK:
			return ChkCLinkIsExist(linknum, data);
#endif
		default:           
			return EXEERROR;
	}
}

/*******************************************************************************
�� �� ����	GetLinkNum
����˵���� 	��type�����л�ȡ��Ӧ��������ֵ
��	  ���� 	*linknum-�ҵ�����С�����Ŵ���ڴ�
             type-����
�� �� ֵ��	1��ʾ�ɹ���0��ʾʧ��
*******************************************************************************/
u8 GetLinkNum(u8 type, u8 *linknum)
{
	switch(type)
	{
#if(ALINK_EN > 0)
		case SIMPLESTLINK:
			return GetALinkNum(linknum);
#endif
#if(SLINK_EN > 0)
		case SIMPLELINK:
			return GetSLinkNum(linknum);
#endif
#if(CLINK_EN > 0)
		case COMPLEXLINK:
			return GetCLinkNum(linknum);
#endif
		default:           
			return EXEERROR;
	}
}

/*******************************************************************************
�� �� ����	WriteLinkItem
����˵���� 	����������д��type�����������У�д����»�Ӧ���������ݣ�����Ӧ��
��	  ���� 	*data-�������ݣ�����ͬSLinkItem�ṹ��
�� �� ֵ��	1��ʾ�ɹ���0��ʾʧ��
*******************************************************************************/
u8 WriteLinkItem(u8 type, u8 *data)
{
	u8 num, linknum, addflag=0;
	if(ChkLinkIsExist(type, &linknum, data) == 0)//�Ƿ���ڣ����������linknumֵ
	{//������
		if(CheckLinkFull(type) == TRUE)
		{
			return LINKFULL;
		}
		
		if(GetLinkNum(type, &linknum) == TRUE)
		{
			addflag = 1;//������������������
			num = EXEERROR;
		}
		else
		{
			return LINKNUM_ERR;
		}
		if (WriteLinkToTable(type, num, linknum, addflag, data) != 1)
		{
			return EEPROM_ERR;
		}
	}
	else
	{
#if(ALINKTIME_EN > 0)		//����ʱ��T�����������ͬʱ��Tɾ��������
		u8 number;
		if((number = QueryALinkTime(num)) != EXEERROR)
		{
			return DelALinkTimeFromTable(number);
		}
#endif
	}

	data -= 2;
	data[0] = linknum;
	
	return COMPLETE;
}

/*******************************************************************************
�� �� ����	ModifyLinkItem
����˵���� 	����������������������д�������������������ڸ�����������Ӧ��������
            ֱ�Ӹ��ǣ�����������в����ڣ����ں��������������
��	  ���� 	type-�������ͣ�linknum-�����ţ�*data-�������ݣ�����ͬSLinkItem�ṹ��
�� �� ֵ��	1��ʾ�ɹ���0��ʾʧ��
*******************************************************************************/
u8 ModifyLinkItem(u8 type, u8 linknum, u8 *data)
{
	u8 num,flag=0,addflag=0;
#if (ALINKTIME_EN > 0)
	u8 number;
#endif
	if((num = QueryLink(type, linknum)) == EXEERROR)
	{
		addflag = 1;
	}

	flag = WriteLinkToTable(type, num, linknum, addflag, data);

#if (ALINKTIME_EN > 0)
	if((number = QueryALinkTime(linknum)) != 0xFF)
	{
		return DelALinkTimeFromTable(number);
	}
#endif	
	return flag;
}

/*******************************************************************************
�� �� ����	ReadSLinkItem
����˵���� 	��type�����������ж�����Ӧ���������ݣ������dataָ��
��	  ���� 	num-�����ţ��������������ݷ���dataָ��
            typeΪ���ͺŷ�����������������͸�����������
            *len-��ȡ�������ݳ���
�� �� ֵ��	1��ʾ�ɹ���0��ʾʧ��
*******************************************************************************/
u8 ReadLinkItem(u8 type, u8 num, u8 *len, u8 *data)
{
	u8 number;
#if (ALINKTIME_EN > 0)	
	u8 cnt,time[6]={0};
	ALinkItem *pLink;
#endif
	if((number = QueryLink(type,num)) != EXEERROR)
	{
		ReadLinkFromTable(type, number, len, data);
		
#if (ALINKTIME_EN > 0)	
		pLink = (ALinkItem *)(data-2);
		
		if( (number = QueryALinkTime(num)) != EXEERROR)
		{
			ReadALinkTime(number , &cnt, &time[1]);
			
			*len += sizeof(DlyTimeData_t) + 1;
			time[0] = 1;
			memcpy(&pLink->data[pLink->DestLen], &time[0], sizeof(DlyTimeData_t)+1);
		}
#endif
		
		return TRUE;
	}
	return FALSE;
}

/*******************************************************************************
�� �� ����	QueryLinkItem
����˵���� 	�������Ų�ѯ�����������Ƿ���������
��	  ���� 	*data-�������������ݷ���dataָ��
            type-���ͣ��ֱ�Ϊ��򡢼򵥡�������������
            num-�����ţ�Ψһ
�� �� ֵ��	1��ʾ�ɹ���0��ʾʧ��
*******************************************************************************/
u8 QueryLinkItem(u8 type, u8 num, u8 *len, u8 *data)
{
	if(num == 0xFF)
	{
		QueryAllLinkFromTable(type, len, data);
		return TRUE;
	}
	else if((data[0] = QueryLink(type, num)) != EXEERROR)
	{
		*len = 1;
		return TRUE;
	}
	return FALSE;
}

/*******************************************************************************
�� �� ����	DelLinkItem
����˵���� 	��������ɾ��type�����������ж�Ӧ����
��	  ���� 	type-���ͣ��ֱ�Ϊ��򡢼򵥡�������������
            num-�����ţ�Ψһ
�� �� ֵ��	1��ʾ�ɹ���0��ʾʧ��
*******************************************************************************/
u8 DelLinkItem(u8 type, u8 num)
{
	u8 number;
	
	if(num == 0xFF)
	{
#if (ALINKTIME_EN > 0)
		AllALinkTimeClr();
#endif
		return DelAllLinkFromTable(type);
	}
	
	if((number = QueryLink(type, num)) != 0xFF)
	{
		DelLinkFromTable(type, number);
	}
	
#if (ALINKTIME_EN > 0)
	if((number = QueryALinkTime(num)) != 0xFF)
	{
		return DelALinkTimeFromTable(number);
	}
#endif	
	return TRUE;
}

#if (ALINKTIME_EN > 0)
/*******************************************************************************
�� �� ����	WriteLinkItemEx
����˵���� 	����������д��type�����������У�д����»�Ӧ���������ݣ�����Ӧ��
��	  ���� 	*data-�������ݣ�����ͬSLinkItem�ṹ��
�� �� ֵ��	1��ʾ�ɹ���0��ʾʧ��
*******************************************************************************/
u8 WriteLinkItemEx(u8 type, u8 *data)
{
	u8 num, linknum, addflag=0;
	u8 time[5]={0};

	ALinkItem *pLink;
	pLink = (ALinkItem *)(data-2);
	
	if(pLink->data[pLink->DestLen] == TIMEFLAG)//flag��Чʱ������ʱ�䣬����Ϊ��0
	{
		memcpy(time, (u8 *)&pLink->data[pLink->DestLen+1], sizeof(DlyTimeData_t));
		if(FALSE == CheckAllTimeParaIsOK((DlyTimeData_t*)time,1))
		{
			return TIME_PARA_ERR;
		}
	}
	if(ChkLinkIsExist(type, &linknum, data) == 0)//�Ƿ���ڣ����������linknumֵ
	{//������
		if(CheckLinkFull(type) == TRUE)
		{
			return LINKFULL;
		}
		
		if(GetLinkNum(type, &linknum) == TRUE)
		{
			addflag = 1;//������������������
			num = EXEERROR;
		}
		else
		{
			return LINKNUM_ERR;
		}
		if (WriteLinkToTable(type, num, linknum, addflag, data) != 1)
		{
			return EEPROM_ERR;
		}
	}
	else
	{//����
		if( (num = QueryALinkTime(linknum)) == EXEERROR)
		{
			addflag = 1;//����
		}
	}
	
	WriteALinkTimeToTable(num, linknum, addflag, time);
	
	data -= 2;
	data[0] = linknum;
	
	return COMPLETE;
}
/*******************************************************************************
�� �� ����	ModifyLinkItemEx
����˵���� 	����������������������д�������������������ڸ�����������Ӧ��������
            ֱ�Ӹ��ǣ�����������в����ڣ����ں��������������
��	  ���� 	type-�������ͣ�linknum-�����ţ�*data-�������ݣ�����ͬSLinkItem�ṹ��
�� �� ֵ��	1��ʾ�ɹ���0��ʾʧ��
*******************************************************************************/
u8 ModifyLinkItemEx(u8 type, u8 linknum, u8 *data)
{
	u8 num,flag=0,addflag=0;
	u8 time[5]={0};

	ALinkItem *pLink;
	pLink = (ALinkItem *)(data-2);

	if(pLink->data[pLink->DestLen] == TIMEFLAG)//flag��Чʱ������ʱ�䣬����Ϊ��0
	{
		memcpy(time, (u8 *)&pLink->data[pLink->DestLen+1], sizeof(DlyTimeData_t));
		if(FALSE == CheckAllTimeParaIsOK((DlyTimeData_t*)time,1))
		{
			return TIME_PARA_ERR;
		}
	}
	
	if((num = QueryLink(type, linknum)) == EXEERROR)
	{
		addflag = 1;
	}

	flag = WriteLinkToTable(type, num, linknum, addflag, data);
	
	if( (num = QueryALinkTime(linknum)) == EXEERROR)
	{
		addflag = 1;//����
	}
	WriteALinkTimeToTable(num, linknum, addflag, time);
	
	return flag;
}
///*******************************************************************************
//�� �� ����	ReadSLinkItemEx
//����˵���� 	��type�����������ж�����Ӧ���������ݣ������dataָ��
//��	  ���� 	num-�����ţ��������������ݷ���dataָ��
//            typeΪ���ͺŷ�����������������͸�����������
//            *len-��ȡ�������ݳ���
//�� �� ֵ��	1��ʾ�ɹ���0��ʾʧ��
//*******************************************************************************/
//u8 ReadLinkItemEx(u8 type, u8 num, u8 *len, u8 *data)
//{
//	u8 number;
//	u8 cnt,time[6]={0};
//	ALinkItem *pLink;
//	
//	if((number = QueryLink(type,num)) != EXEERROR)
//	{
//		ReadLinkFromTable(type, number, len, data);

//		pLink = (ALinkItem *)(data-2);
//		ReadALinkTime(num , &cnt, &time[1]);
//		if(!(time[0] == 0 &&
//			 time[1] == 0 &&
//			 time[2] == 0 &&
//			 time[3] == 0 &&
//			 time[4] == 0	))
//		{
//			*len += sizeof(DlyTimeDat_t) + 1;
//			time[0] = 1;
//			memcpy(&pLink->data[pLink->DestLen], &time[1], sizeof(DlyTimeDat_t));
//		}
//		
//		return TRUE;
//	}
//	return FALSE;
//}
///*******************************************************************************
//�� �� ����	DelLinkItemEx
//����˵���� 	��������ɾ��type�����������ж�Ӧ����
//��	  ���� 	type-���ͣ��ֱ�Ϊ��򡢼򵥡�������������
//            num-�����ţ�Ψһ
//�� �� ֵ��	1��ʾ�ɹ���0��ʾʧ��
//*******************************************************************************/
//u8 DelLinkItemEx(u8 type, u8 num)
//{
//	u8 number;
//	
//	if(num == 0xFF)
//	{
//		AllALinkTimeClr();
//		return DelAllLinkFromTable(type);
//	}
//	
//	if((number = QueryLink(type, num)) != 0xFF)
//	{
//		DelLinkFromTable(type, number);
//	}
//	
//	if((number = QueryALinkTime(num)) != 0xFF)
//	{
//		return DelALinkTimeFromTable(number);
//	}
//	
//	return TRUE;
//}
#endif

/*******************************************************************************
�� �� ����	DelLinkForUnit
����˵���� 	����Ԫ��ɾ������
��	  ���� 	unitnum-��Ԫ��
�� �� ֵ��	1��ʾ�ɹ���0��ʾʧ��
*******************************************************************************/
u8 DelLinkForUnit(u8 unitnum)
{
	u8 i=0,updateflag=0;

	if(!ALink.Cnt)
	{
		return 1;
	}

	while(ALink.Cnt > i)
	{
		if(unitnum == ALink.linkitem[i].SUnitID)
		{
			updateflag = 1;
			memcpy((u8 *)&ALink.linkitem[i], (u8 *)&ALink.linkitem[ALink.Cnt-1], sizeof(ALinkItem));
			ALink.Cnt--;
			
#if (ALINKTIME_EN > 0)
			DelLinkItem(0, ALink.linkitem[i].LinkNum);
#endif
		}
		else
		{
			i++;
		}
	}
	
	if(updateflag)
	{
		WriteDataToEEPROM(GetALinkStartAddr(), sizeof(ALinkTable), 
			(u8 *)&ALink);
	}
	return 1;
}
/*******************************************************************************
�� �� ����	DelLinkForPropIDandVal
����˵���� 	�����Ժź�Դֵɾ������
��	  ���� 	propID-���Ժţ�sValue-Դֵ
�� �� ֵ��	1��ʾ�ɹ���0��ʾʧ��
*******************************************************************************/
u8 DelLinkForPropIDandVal(u8 sPropID , int sValue) // yanhuan adding 2015/11/02
{
	u8 i=0 ;
	for( i = 0 ; i < ALink.Cnt ;)
	{
		if(ALink.linkitem[i].SPropID == sPropID && 
			*(u32*)&ALink.linkitem[i].SValue == sValue)
		{
			DelALink(i);
#if (ALINKTIME_EN > 0)
			DelLinkItem(0, ALink.linkitem[i].LinkNum);
#endif
			continue ;
		}
		i++ ;
	}		
	return 1;
}

#endif
/**************************Copyright BestFu 2014-05-14*************************/
