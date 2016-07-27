/***************************Copyright BestFu 2014-05-14*************************
��	����	Linksrc.c
˵	����	Դ��ִ����ں����������ĳ�ʼ������Դ�ļ�
��	�룺	Keil uVision4 V4.54.0.0
��	����	v1.0
��	д��	Joey
��	�ڣ�	2013.7.22 
�ޡ��ģ�	����
*******************************************************************************/

#include "linkall.h"

//#if (SLINK_SRC_VALUE_EN > 0)
//SrcOldValTable SrcOld;
//#endif

/*******************************************************************************
�� �� ����	LinkInit
����˵���� 	���������ĳ�ʼ��
��	  ���� 	void
�� �� ֵ��	void
*******************************************************************************/
void LinkInit(void)
{
#if (ALINK_EN > 0)
	ALinkInit();
#endif
#if (SLINK_EN > 0)
	SLinkInit();
#endif
#if (CLINK_EN > 0)
	CLinkInit();
#endif
}

/*******************************************************************************
�� �� ����	LinkClr
����˵���� 	�����������
��	  ���� 	void
�� �� ֵ��	void
*******************************************************************************/
void LinkClr(void)
{
#if (ALINK_EN > 0)
	ALinkClr();
#endif
#if (SLINK_EN > 0)
	SLinkClr();
#endif
#if (CLINK_EN > 0)
	CLinkClr();
#endif
}

/*******************************************************************************
�� �� ����	CmpSampAndLinkValue
����˵���� 	�Ƚϲ���ֵ��������ֵ����ͬ�����£��Ƿ����㷶Χ
��	  ���� 	CndtΪ0x01-0x08����ʾ��ͬ��ֵ����
            Lvtmp1Ϊ��������ֵ1,��CndtΪ1-6ʱ����Ϊ�жϵ�Ŀ����ֵ��
                               ��CndtΪ7-8ʱ����Ϊ�жϵ���ֵ��
            Lvtmp2Ϊ��������ֵ2,��CndtΪ1-6ʱ��������Ч��
                               ��CndtΪ7-8ʱ����Ϊ�жϵ���ֵ��
            SvtmpΪ����ֵ����ǰ�豸�Ĳ���ֵ��
�� �� ֵ��	1��ʾ����������0��ʾ����������
*******************************************************************************/
u8 CmpSampAndLinkValue(u8 Cndt, int Lvtmp1, int Lvtmp2, int Svtmp)
{
	switch(Cndt)
	{
		case CLARGE:        if(Lvtmp1 <  Svtmp)return TRUE; else return FALSE;
		case CLARGEOREQUAL: if(Lvtmp1 <= Svtmp)return TRUE; else return FALSE;
		case CSMALL:        if(Lvtmp1 >  Svtmp)return TRUE; else return FALSE;
		case CSMALLOREQUAL: if(Lvtmp1 >= Svtmp)return TRUE; else return FALSE;
		case CEQUAL:        if(Lvtmp1 == Svtmp)return TRUE; else return FALSE;
		case CNOTEQUAL:     if(Lvtmp1 != Svtmp)return TRUE; else return FALSE;
		case CINRANGE:      if((Lvtmp1 <= Svtmp) && (Lvtmp2 >= Svtmp))
								return TRUE; 
							else 
								return FALSE;
		case COUTRANGE:		if((Lvtmp1 > Svtmp) || (Lvtmp2 < Svtmp))
								return TRUE; 
							else 
								return FALSE;
		default:            return FALSE;
	}
}

/*******************************************************************************
�� �� ����	GetConditionReverse
����˵���� 	��һ�������ţ���ȡ�䷴��������
��	  ���� 	SrcCndt(source condition)��д
            ��Ӧ�ķ����߼�����ѧ�򵥵��ж�
�� �� ֵ��	��ʾ�����������
*******************************************************************************/
u8 GetConditionReverse(u8 SrcCndt)
{
	switch(SrcCndt)
	{
		case CEQUAL:         return CNOTEQUAL;
		case CNOTEQUAL:      return CEQUAL;
		case CLARGE:         return CSMALLOREQUAL;
		case CSMALLOREQUAL:  return CLARGE;
		case CSMALL:         return CLARGEOREQUAL;
		case CLARGEOREQUAL:  return CSMALL;	
		case CINRANGE:       return COUTRANGE;
		case COUTRANGE:      return CINRANGE;
		default:             return FALSE;
	}
}

/*******************************************************************************
�� �� ����	GetConditionOffset
����˵���� 	��ȡƫ�������ڲ�ͬ�������£�ƫ�����ķ���ͬ���������仯�Ĳ���ֵ����
            ��ƫ�������м䣬���Ա�֤�����Ĵ����й��ɣ����Ҵ���
��	  ���� 	SrcCndt(source condition)��д
            ��ʾ��Ӧ�ķ����߼�����ѧ�򵥵��ж�
            sv1Ϊ��������ֵ1����CndtΪ1-6ʱ����Ϊ�жϵ�Ŀ����ֵ��
                             ��CndtΪ7-8ʱ����Ϊ�жϵ���ֵ��
            sv2Ϊ��������ֵ1, ��CndtΪ1-6ʱ��������Ч��
                             ��CndtΪ7-8ʱ����Ϊ�жϵ���ֵ��

            �˴���OFFSET����̫�󣬴���û���壬С��Ч�������ԣ�ȡֵ��Ҫ����������
            �ڷ�Χ�ںͷ�Χ���ʱ�򣬲�����ȡֵ��ƫ����OFFSET����������ì�ܡ�
�� �� ֵ��	1��ʾ�ɹ���0��ʾʧ��
*******************************************************************************/
u8 GetConditionOffset(u8 SrcCndt, int *sv1, int *sv2)
{
	switch(SrcCndt)
	{
		case CLARGE:         
		case CLARGEOREQUAL:  *sv1 += OFFSET;    break;
		case CSMALL:         
		case CSMALLOREQUAL:  *sv1 -= OFFSET;    break;
		case CINRANGE:       *sv1 += OFFSET;    *sv2 -= OFFSET;  break;
		case COUTRANGE:      *sv1 += OFFSET;    *sv2 -= OFFSET;  break;
		default:             return FALSE;
	}
	return TRUE;
}

#if (SLINK_SRC_VALUE_EN > 0)
///*******************************************************************************
//�� �� ����	FindOldValueTableFull
//����˵���� 	��һ�β���ֵ�б��Ƿ��Ѿ����ˣ���Ψһ��Ԫ�������Ժ�����
//��	  ���� 	unitID-��ԪID�ţ�propID-����ID��
//�� �� ֵ��	1��ʾδ����0��ʾ����
//*******************************************************************************/
//u8 FindOldValueTableFull(u8 unitID, u8 propID)
//{
//	if(SrcOld.Cnt >= MAX_SRC_OLD_CNT)
//	{
//		return FALSE;
//	}
//	return TRUE;
//}

///*******************************************************************************
//�� �� ����	FindOldValueTableItem
//����˵���� 	Ѱ��unitID,propID��Ӧ���ϴβ���ֵ�Ƿ���ڣ����ڷ��ض�Ӧλ�ú�
//            �����ڵĻ�������OxFF��
//��	  ���� 	unitID-��ԪID�ţ�propID-����ID��
//�� �� ֵ��	0-254���ض�Ӧ��λ�úţ�255��ʾ������
//*******************************************************************************/
//u8 FindOldValueTableItem(u8 unitID, u8 propID)
//{
//	u8 i;
//	for(i=0; i<SrcOld.Cnt; i++)
//	{
//		if( SrcOld.src[i].UnitID == unitID &&
//			SrcOld.src[i].PropID == propID )
//		{
//			return i;
//		}
//	}
//	return 0xFF;
//}

///*******************************************************************************
//�� �� ����	AddOldValueTableItem
//����˵���� 	����ֵ���ڶ�����
//��	  ���� 	unitID-��ԪID�ţ�propID-����ID��
//�� �� ֵ��	0-254���ض�Ӧ��λ�úţ�255��ʾ������
//*******************************************************************************/
//u8 AddOldValueTableItem(u8 unitID, u8 propID)
//{
//	if(FindOldValueTableFull(unitID, propID))
//	{
//		return FALSE;
//	}
//	else
//	{
//		SrcOld.src[SrcOld.Cnt].UnitID   = unitID;
//		SrcOld.src[SrcOld.Cnt].PropID   = propID;
//		SrcOld.src[SrcOld.Cnt].Value    = 0;
//		SrcOld.Cnt++;
//	}
//	return TRUE;
//}
#endif

/*******************************************************************************
�� �� ����	LinkSrcExe
����˵���� 	Դ���¼���������ֵ�仯����������������������ִ������
��	  ���� 	unitID����ԪID��
            type,   ���ͣ����¼��ͻ�����ֵ��
            propID, ���Ժ�
            num��   �¼���ʱ��ʾ�����ţ���ֵ��ʱ��ʾԴ���ͱ��
�� �� ֵ��	1��ʾ�ɹ���0��ʾʧ��
*******************************************************************************/
u8 LinkSrcExe(u8 unitID, u8 propID, u8 type, int num)
{
#if(ALINK_EN > 0 || SLINK_EN > 0 || CLINK_EN > 0)
	u8 i;
#if (ALINKTIME_EN > 0)	
	u8 number, data[40];
	ALinkItem *pLink;
#endif
#endif

#if(ALINK_EN > 0)
	for(i=0;i<ALink.Cnt;i++)
	{
		if(ALink.linkitem[i].UseFlag == 0x01)
		{
			if(type == SRCEVENT)
			{
	#if(SLINK_SRC_EVENT_EN > 0)
				u32 value=0;
				value = DWCHAR( ALink.linkitem[i].SValue[3],
								ALink.linkitem[i].SValue[2],
								ALink.linkitem[i].SValue[1],
								ALink.linkitem[i].SValue[0]);
				
				if( ALink.linkitem[i].SUnitID == unitID && 
					ALink.linkitem[i].SPropID == propID)
				{
					if(CmpSampAndLinkValue(ALink.linkitem[i].SCondition, value, 0, num))
					{
						value = DWCHAR( ALink.linkitem[i].DDevID[3],
										ALink.linkitem[i].DDevID[2],
										ALink.linkitem[i].DDevID[1],
										ALink.linkitem[i].DDevID[0]);
						if(!value)//����λ����Լ������Ŀ��Ϊȫ0ʱ����ʾ�����ϱ���ǰ���������
						{
								Upload_Immediately(ALink.linkitem[i].SUnitID,ALink.linkitem[i].SPropID,ALink.linkitem[i].SValue);
								continue;
						}
#if (ALINKTIME_EN > 0)		
						if( (number = QueryALinkTime(ALink.linkitem[i].LinkNum)) != EXEERROR)
						{
							memcpy(data, (u8 *)&ALink.linkitem[i].UseFlag, sizeof(ALinkItem));
							AddALinkTimeCmdX1(number, data);
							
							pLink = (ALinkItem *)data;
							Msg_Send((ObjectType_t)pLink->DType, 
									value, 
									WCHAR(pLink->DAddrT[1], pLink->DAddrT[0]), 
									pLink->UnitID, 
									pLink->DestCmd, 
									pLink->DestLen + sizeof(DlyTimeData_t), 
									pLink->data);
						}
						else
#endif
						{
							Msg_Send((ObjectType_t)ALink.linkitem[i].DType, 
									value, 
									WCHAR(ALink.linkitem[i].DAddrT[1],ALink.linkitem[i].DAddrT[0]), 
									ALink.linkitem[i].UnitID, 
									ALink.linkitem[i].DestCmd, 
									ALink.linkitem[i].DestLen, 
									ALink.linkitem[i].data);
						}
					}
				}
	#endif
			}
			else if(type == SRCVALUE)
			{
	#if(ALINK_SRC_VALUE_EN > 0)
				if( ALink.linkitem[i].SUnitID == unitID && 
					ALink.linkitem[i].SPropID == propID )
				{
					CheckALinkValueAndExe((u8 *)&ALink.linkitem[i], ALink.linkitem[i].LinkNum, num);
				}
	#endif
			}
		}
	}
#endif
	return TRUE;
}
/**************************Copyright BestFu 2014-05-14*************************/
