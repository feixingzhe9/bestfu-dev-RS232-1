/***************************Copyright BestFu 2014-05-14*************************
��	����	UserDataDownload.c
˵	����	�û���������ʵ��ԭ��
��	�룺	Keil uVision4 V4.54.0.0
��	����	v1.0
��	д��	Unarty
��	�ڣ�	2014.08.28 
�ޡ��ģ�	2014.12.25 Unarty �޸�Flash�û����ݴ洢����ַ��Χ
*******************************************************************************/
#include "UserDataDownload.h"
#include "DevPartTab.h"
#include "Program.h"

UserDataDownload_t UserDownload;

/*******************************************************************************
�� �� ����	UserDataDownload_Init
����˵���� 	�û��������ر�ʶ��ʼ��
��	  ���� 	��
�� �� ֵ:  	��
*******************************************************************************/
void UserDataDownload_Init(void)
{
	memset(&UserDownload, 0, sizeof(UserDownload));
	UserDownload.save = NULL;	
}

/*******************************************************************************
�� �� ����	UserDataDownload_Ready
����˵���� 	����׼������
��	  ���� 	pData: ��Ԫ/����/����/����
�� �� ֵ:  	��Ϣִ�н��
*******************************************************************************/
MsgResult_t UserDataDownload_Ready(UnitPara_t *pData)
{
	u32 dataSize;
	
	if (pData->len < 10)	
	{
		return PARA_LEN_ERR;
	}
	
	dataSize = *(u32*)&pData->data[0];		//���ݴ�С
	UserDownload.frameSize = pData->data[4];	//ÿ֡���ݴ�С
	UserDownload.addr = *(u32*)&pData->data[6];	//�û��������ش洢��ַ
	UserDownload.frameCount = NUM_ROUND(dataSize,UserDownload.frameSize);//��ȡ�����ܰ���
	memset(UserDownload.frameFlag, 0, sizeof(UserDownload.frameFlag));	//���֡��ʶ��
	
	if ((sizeof(UserDownload.frameFlag)*8) < UserDownload.frameCount)	//�ܰ�������
	{
		return PARA_MEANING_ERR;
	}
	else if (USER_FLASH == pData->data[5])	//�洢���û�Flash��
	{	
		UserDownload.save = Program_Write;			//����Flashд�뺯��	
		if((DevPartTab->user_data_start_addr <= UserDownload.addr)&&	//�洢��ַ�Ϸ�
		   (DevPartTab->flash_end_addr > (dataSize + UserDownload.addr)))//���ݴ洢���ݲ������	
		{	
			return ((MsgResult_t)(Program_Erase(UserDownload.addr, dataSize)));	//�����û�Flash��
		}		
	}
	else if (USER_EEPROM == pData->data[5])	//�洢���û�EEPROM��
	{
		UserDownload.save = (Save_fun)EEPROM_Write;		//����Flashд�뺯��
		if ((USER_EEPROM_START_ADDR <= UserDownload.addr)		//�洢��ַ����
			&& (USER_EEPROM_END_ADDR > (dataSize + UserDownload.addr))			//���ݴ洢�������
			)
		{
			return COMPLETE;
		}
	}
	
	UserDataDownload_Init();
	
	return PARA_MEANING_ERR;
}

/*******************************************************************************
�� �� ����	UserDataFrame_Save
����˵���� 	��������֡�洢
��	  ���� 	pData: ��Ԫ/����/����/����
�� �� ֵ:  	��Ϣִ�н��
*******************************************************************************/
MsgResult_t UserDataFrame_Save(UnitPara_t *pData)
{
	u32 addr;
	u16 id;
	
	id = *(u16*)&pData->data[0];	//����֡ID
	pData->len -= sizeof(id);	//ÿ֡�û��������ݳ���
	
	if ((pData->len > UserDownload.frameSize)	//֡���ݳ��ȴ���
		|| (NULL == UserDownload.save)	//���ݴ洢����Ϊ��
		)
	{
		return NO_CMD;
	}
	else if (id > UserDownload.frameCount)	//��ID����
	{
		return PARA_MEANING_ERR;
	}
	
	if (!(UserDownload.frameFlag[id>>5]&(1<<(id&0x1f))))
	{
	    addr = id*UserDownload.frameSize + UserDownload.addr;
	    if (TRUE != UserDownload.save(addr, pData->len, (u8*)&pData->data[2]))	//�洢ʧ��
	    {
			return CMD_EXE_ERR;
	    }
		UserDownload.frameFlag[id>>5] |= (1<<(id&0x1f)); //���հ�ID��1
	}
	
	return COMPLETE;
}

/*******************************************************************************
�� �� ��:  	UserDataFrame_Check
����˵��:  	�û�֡�˶�
��    ��:  	pData: ��Ԫ/����/����/����
			rLen:	���ز�������
			rpara:	���ز�������
�� �� ֵ:  	��Ϣִ�н��
*******************************************************************************/
MsgResult_t UserDataFrame_Check(UnitPara_t *pData, u8 *rlen, u8 *rpara)
{
	u32 i;
	
	for (i = 0, *rlen = 0; i < UserDownload.frameCount; i++)
	{
		if (!(UserDownload.frameFlag[i>>5]&(1<<(i&0x1f))))//��δ�յ�
        {
			*(u16*)&rpara[*rlen] = i;
			(*rlen) += 2;
			if ((*rlen) > 100)
			{
				break;
			}	
        }
	}	
	return ((0 == *rlen) ? COMPLETE : (MsgResult_t)0);
}
/**************************Copyright BestFu 2014-05-14*************************/
