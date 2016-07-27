/***************************Copyright BestFu 2014-05-14*************************
��	����	UnitShare_Attr.c
˵	����	����Ԫ��������ʵ�ֺ���
��	�룺	Keil uVision4 V4.54.0.0
��	����	v1.0
��	д��	Unarty
��	�ڣ�	2014-04-08
�ޡ��ģ�	Unarty 2014.10.28,�޸����е�Ԫ��֤�ӿ��쳣Bug��
			2014.12.28 Unarty �޸ĵ�Ԫ����ִ�б߽������ж�ֵ����ֹ�൥Ԫִ��ʱ����Խ��
*******************************************************************************/
#include "UnitShare_Attr.h"
#include "cmdProcess.h"
#include "Instruct.h"
#include "Thread.h"

/*******************************************************************************
�� �� ��:  	Set_ManyUnit_Attr
����˵��:  	���ö����Ԫ����
��    ��:  	data[0]: Ŀ�굥Ԫ
			data[1]: ��������
�� �� ֵ:  	��Ϣִ�н��
*******************************************************************************/
MsgResult_t Set_ManyUnit_Attr(UnitPara_t *pData)
{
	if (pData->len > 0)	//�˶Բ�������
	{
		MsgResult_t result; 
		u8 buf[128];
		u8 paraLen = pData->len;	//��������
		u8 *pPara = (u8*)&pData->data;	//������ָ��
		
		while (pPara < &pData->data[paraLen]) //������û��ʹ����
		{
			if (pPara[2] > 128)	//������Բ������ȴ���128
			{
				return PARA_LEN_ERR;
			}
			buf[0] = WRITEWITHACK;
			memcpy(&buf[1], pPara, (3 + pPara[2]));
			result = Msg_Execute((CMDPara_t*)buf);
			
			if (result != COMPLETE)	//ָ��ִ�в���ȷ
			{
				*(u16*)&pData->data[0] = *(u16*)&buf[1];  //��ֵ��Ԫ�������Ժ�
				*(u8*)&pData->data[2] = (u8)result;
				pData->len = 3;
				return OWN_RETURN_PARA;
			}
			pPara += (3 + pPara[2]);
		}
		return COMPLETE;		
	}
	
	return PARA_LEN_ERR;
}

#if (DALAY_EXECUTE_EN > 0u)
/*******************************************************************************
�� �� ��:  	Set_ManyUnitEx_Attr
����˵��:  	���ö����Ԫ����,�ô���ʱ�����ԵĶ�����ִ��
��    ��:  	data[0]: Ŀ�굥Ԫ
			data[1]: ��������
�� �� ֵ:  	��Ϣִ�н��
*******************************************************************************/
MsgResult_t Set_ManyUnitEx_Attr(UnitPara_t *pData)
{
//	MsgResult_t result; 
	if (pData->len > 0)							//�˶Բ�������
	{ 
		u8 buf[32];
		u8 paraLen = pData->len;				//��������
		u8 *pPara = (u8*)&pData->data;			//������ָ��
		
		while (pPara < &pData->data[paraLen])   //������û��ʹ����
		{
			if (pPara[2] > 13)					//������Բ������ȴ���13
			{
				return PARA_LEN_ERR;
			}
			memcpy(&buf[0], pPara, (offsetof(UnitPara_t,data) + pPara[2] + sizeof(DlyTimeData_t)));
			if(FALSE == CreateBufferNode((u8*)buf))
			{
				*(u16*)&pData->data[0] = *(u16*)&buf[1];  //��ֵ��Ԫ�������Ժ�
				*(u8*)&pData->data[2] = (u8)DLY_BUFFER_FULL;
				pData->len = 3;
				return OWN_RETURN_PARA;
			}
			pPara += (offsetof(UnitPara_t,data) + pPara[2] + sizeof(DlyTimeData_t));
		}
		return COMPLETE;		
	}
	return PARA_LEN_ERR;
}
#endif

/*******************************************************************************
�� �� ��:  	Get_ManyUnit_Attr
����˵��:  	��ȡ�����Ԫ����ֵ
��    ��:  	pData->unit:	������Ԫ
			pData->cmd:		����ָ��
			pData->len��	��������
			pData->data:	��������
			rLen:	���ز�������
			rpara:	���ز�������
�� �� ֵ:  	��Ϣִ�н��
*******************************************************************************/
MsgResult_t Get_ManyUnit_Attr(UnitPara_t *pData, u8 *rlen, u8 *rpara)
{
	*rlen = 0;	//�������ݳ�������
	
	if (pData->len > 0)	//��ȡĳЩ��Ԫ������
	{
		u8 para[255]; //������
		u8 paraLen = pData->len;
		u8 *pPara = &para[paraLen];	//������ָ��
		u8 *aimPara = para;			//�´���Ϣ����
		
		memcpy(aimPara, pData->data, paraLen);
		
		while (aimPara < &para[paraLen])
		{
			((CMDPara_t*)pPara)->msgType = READWITHACK;
			((CMDPara_t*)pPara)->unit = *aimPara++;
			((CMDPara_t*)pPara)->cmd = *aimPara++;
			((CMDPara_t*)pPara)->len = 0;						//�����´��� 
			if (COMPLETE == Msg_Execute((CMDPara_t*)pPara))		//�����Ϣ���
			{
				rpara[(*rlen)++] = ((CMDPara_t*)pPara)->unit;	//��Ԫ��
				rpara[(*rlen)++] = ((CMDPara_t*)pPara)->cmd;	//���Ժ�				
				rpara[(*rlen)++] = --((CMDPara_t*)pPara)->len; 	//�������ȣ�����Ϊÿ�λ�ȡ����ǰ���һ������Ϊ������ִ�н������������жϣ������ڴ��Ƴ���
				memcpy(&rpara[*rlen], &pPara[sizeof(CMDPara_t)+1], ((CMDPara_t*)pPara)->len); //����ȡ�ĵ�Ԫ�������ݷ��õ����ز�������
				*rlen += ((CMDPara_t*)pPara)->len;
				if (*rlen > MAX_ATTR_NUM)	//�����ȡ�ķ��ز������ݱȽϴ�
				{
					break;
				}
			}
		}
	}
	else // ��ȡĳһ����Ԫ����ֵ
	{
			u8 data[4],ExeAttrNum = 0u; 		//������
			const AttrExe_st *pAttrExe = UnitTab[pData->unit].attrExe;	
			do {
				if (LEVEL_1 == pAttrExe->level)	//������Եȼ��ϸ�
				{
					data[0] = pData->unit;		//��Ԫ	
					data[1] = pAttrExe->cmd;	//����
					data[2] = 0;				//��������
					if ((pAttrExe->pGet_Attr != NULL)
						&& (COMPLETE == pAttrExe->pGet_Attr((UnitPara_t*)data, &data[3], &rpara[(*rlen)+3]))	//�����ȡ�����������
						)
					{
						rpara[(*rlen)++] = pData->unit;		//��Ԫ
						rpara[(*rlen)++] = pAttrExe->cmd;	//����
						rpara[(*rlen)++] = data[3];			//��������
						*rlen += data[3];
						
						if (*rlen > MAX_ATTR_NUM)			//�����ȡ�ķ��ز������ݱȽϴ�
						{
							break;
						}
					}
				}
			}while(((++ExeAttrNum) < UnitTab[pData->unit].ExeAttrNum())  //2016.01.29 jay modify
				&&((++pAttrExe)->cmd < 0x0100));
			/*while ((pAttrExe->cmd < (++pAttrExe)->cmd)		//2014.12.28 Unarty Add
					&& (pAttrExe->cmd < 0x0100));*/
	}	
	return COMPLETE;	
}

/*******************************************************************************
�� �� ��:  	Set_UnitChecking_Attr
����˵��:  	���õ�Ԫ��֤
��    ��:  	data[0]: Ŀ�굥Ԫ
			data[1]: ��������
�� �� ֵ:  	��Ϣִ�н��
*******************************************************************************/
MsgResult_t Set_UnitChecking_Attr(UnitPara_t *pData)
{
	u32 i;
	
	i = UnitCnt_Get();	//��ȡ��Ч��Ԫ��
	
	if (1 != pData->len)
	{
		return PARA_LEN_ERR;	//�������ȴ���
	}
	else if (pData->data[0] >= i)
	{
		return PARA_MEANING_ERR;//�����������	
	}
	
	if (0 == pData->data[0])	//��֤���е�Ԫ
	{
		for (i = UnitCnt_Get(); i > 0;)
		{
			i--;
			if (UnitTab[i].Instruct != NULL)
			{
				UnitTab[i].Instruct(); //ע�ᵥԪ��֤
			}
		}
	}
	else
	{
		if (UnitTab[pData->data[0]].Instruct != NULL)
		{
			 UnitTab[pData->data[0]].Instruct(); //ע�ᵥԪ��֤
		}
	}
	
	Thread_Login(MANY, 200, 50, &Instruct_Run);	//ע���豸��֤
	
	return COMPLETE;	
}

/*******************************************************************************
�� �� ��:  	u8 Get_SingleUnit_AttrLen(u8 unit,u8 cmd)
����˵��:  	��ȡ������Ԫ����ֵ�ĳ���
��    ��:  	unit:	������Ԫ
			cmd:	����ָ��
�� �� ֵ:  	����Get���Խӿڷ���ֵ�ĳ���(0��ʾ����ʧ��)
*******************************************************************************/
u8 Get_SingleUnit_AttrLen(u8 unit,u8 cmd)
{
	u8 data[200],ExeAttrNum = 0u; 		//������	
	const AttrExe_st *pAttrExe = UnitTab[unit].attrExe;	
	
	data[0] = unit;	
	data[1] = cmd;
	data[2] = 0;
	
	do {
			if(pAttrExe->cmd == cmd)
			{
				if ((pAttrExe->pGet_Attr != NULL)
					&& (COMPLETE == pAttrExe->pGet_Attr((UnitPara_t*)data, &data[3], &data[4]))	//�����ȡ�����������
					)
				{
					return (data[3]);
				}
			}
	}while(((++ExeAttrNum) < UnitTab[unit].ExeAttrNum())  
		&&((++pAttrExe)->cmd < 0x0100));
	return 0;
}
/**************************Copyright BestFu 2014-05-14*************************/
