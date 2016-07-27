/***************************Copyright BestFu 2014-05-14*************************
��	����	AttrEncrypt.c
˵	����	���Լ���ԭ�ļ�
��	�룺	Keil uVision4 V4.54.0.0
��	����	v1.0
��	д��	Unarty
��	�ڣ�	2014-01-26
�ޡ��ģ�	��
*******************************************************************************/
#include "AttrEncrypt.h"
#include "MsgPackage.h"
#include "UnitCfg.h"
#include "crc.h"
#include "Thread.h"
#include "SysTick.h"

#define CMD_ADD_FLAG	(0xAA5AA5AA)	//ָ����ܱ��
#define CMD_SUB_FLAG	(0xAAA55AAA)	//ָ����ܱ��
#define ENCRYPT_NUM		(sizeof(Encrypt)/sizeof(Encrypt_t))	//������

static u8 EncryptFlag = 0;		//���ܱ�ʶ

//static Encrypt_t Encrypt[5] = {0};					//ִ����

//static u8 ExeSrc_Check(u32 srcAddr);
//static u8 ExeSrc_Add(Communication_t *cmd);
//static void ExeSrc_TimeCheck(void);
static void Encrypt_FlagClr(void);
static void Encrypt_Data(u8 *data, u16 key);


/*******************************************************************************
�� �� ����  AttrEncrypt_Check
����˵����  ���Լ��ܺ˶�
��	  ����  cmd:	Ŀ��ִ��ָ��			
�� �� ֵ��  TRUE(�˶Գɹ�)/FALSE
*******************************************************************************/
u8 AttrEncrypt_Check(Communication_t *cmd)
{	
	u16 i;
	
//	if (*(u32*)(&cmd->para.len + cmd->para.len - 3) == CMD_SUB_FLAG)//��Կָ���Ӧ
//	{
//		if ((i = ExeSrc_Check(cmd->aim.object.id)) < ENCRYPT_NUM)		//������Դ
//		{
//			cmd->para.len -= 4;		//ȥ��ָ���ʶ
//			Encrypt_Data(&cmd->para.len, Encrypt[i].prd);	//��Կ����
//			Encrypt_Data(&cmd->para.len, Encrypt[i].prv);	//˽Կ����
//			if (Encrypt[i].crc == ChkCrcValue(&cmd->para.len + 1, cmd->para.len)) //���ݽ���ɹ�
//			{
//				cmd->aim.serialNum--;
//				return TRUE;
//			}
//		}
//	}
//	else if (*(u32*)(&cmd->para.len + cmd->para.len - 3) == CMD_ADD_FLAG)//ָ�����
	if (*(u32*)(&cmd->para.len + cmd->para.len - 3) == CMD_ADD_FLAG)//ָ�����
	{
		if (EncryptFlag)	// ����Լ�������ָ��
		{
			Encrypt_FlagClr();
			i = (*(u16*)(&cmd->para.len + cmd->para.len - 5));	//��ȡ��Կ
			cmd->para.len -= 6;		//��ȥָ���ʶ�빫Կ
			Encrypt_Data(&cmd->para.len, i);	//��Կ��������
			*(u32*)(&cmd->para.len + cmd->para.len + 1) = CMD_SUB_FLAG; //���ָ���ʶ
			cmd->para.len += 4;
			return TRUE;
		}
	}
//	else
//	{
//		if (cmd->aim.objectType == SINGLE_ACT		//���һ��ִ�й���
//			&& TRUE == ExeSrc_Add(cmd))	//Ŀ�����Ϊ�������
//		{
//			*(u32*)(&cmd->para.len + cmd->para.len + 1) = CMD_ADD_FLAG;	//����β�����Կָ����
//			cmd->para.len += 4;
//			cmd->aim.serialNum++;
//			Msg_Feedback(cmd);	//����Ϣ���ظ�Դ�豸
//		}
//	}

	return FALSE;
}

/******************************************************************************* 
�� �� ����  Encrypt_FlagSet
����˵����  ��Կ����
��	  ����  ��
�� �� ֵ��  ��
ע	  �⣺	�������޸� ����EncryptFlag ������3s���Զ�����Encrypt_FlagClr�������
*******************************************************************************/	
void Encrypt_FlagSet(void)
{
	EncryptFlag = 1;
	Thread_Login(ONCEDELAY, 0, 3000, Encrypt_FlagClr);
}

/******************************************************************************* 
�� �� ����  Encrypt_FlagClr
����˵����  ��Կ���
��	  ����  ��
�� �� ֵ��  ��
ע	  �⣺	�������޸� ����EncryptFlag
*******************************************************************************/	
static void Encrypt_FlagClr(void)
{
	EncryptFlag = 0;
}

/******************************************************************************* 
�� �� ����  Encrypt_Data
����˵����  ��������
��	  ����  data:	data[0]���ݳ���, data[1~data[0]] ��������
			key:	Կ��
�� �� ֵ��  ��
*******************************************************************************/	
static void Encrypt_Data(u8 *data, u16 key)
{
	u8 i;
	
	for (i = 0; i < data[0]; i++)
	{
		i++;
		*(u16*)&data[i] ^= key;
	}
}
	
///*******************************************************************************
//�� �� ����  ExeSrc_Check
//����˵����  �˶�ָ��Դ��ַ
//��	  ����  Դ��ַ
//�� �� ֵ��  0(û�в��ҵ����ݣ�/���㣺���ݵĴ洢λ��
//*******************************************************************************/
//static u8 ExeSrc_Check(u32 srcAddr)
//{
//    u8 i;
//	
//	for (i = 0; i < ENCRYPT_NUM; i++)
//	{
//		if (Encrypt[i].srcAddr == srcAddr)
//		{
//			break;
//		}
//	}
//	
//	return i;
//}

///******************************************************************************* 
//�� �� ����  ExeSrc_Add
//����˵����  ���ָ��ִ��Դ
//��	  ����  srcAddr:	Դ��ַ
//�� �� ֵ��  FALSE/TRUE
//*******************************************************************************/
//static u8 ExeSrc_Add(Communication_t *cmd)
//{
//	u8 i;
//	
//	if ((i = ExeSrc_Check(cmd->aim.object.id)) >= ENCRYPT_NUM)	//����б�������ڴ�����
//	{
//		for (i = 0; i < ENCRYPT_NUM; i++)
//		{
//			if (0 == Encrypt[i].srcAddr)
//			{
//				Encrypt[i].cnt = 1;
//				break;
//			}
//		}
//	}
//	else
//	{
//		Encrypt[i].cnt++;
//	}
//	
//	if (Encrypt[i].cnt > 5)
//	{
//		//�ϱ��쳣
//	}
//	else
//	{
//		Encrypt[i].time = 5;
//		Encrypt[i].srcAddr = cmd->aim.object.id;
//		Encrypt[i].crc = ChkCrcValue(&cmd->para.len + 1, cmd->para.len);
//		Encrypt[i].prd = cmd->aim.serialNum + cmd->aim.sourceID 	//���������Կ
//						+ cmd->aim.object.area[cmd->aim.serialNum%4] + Time_Get();
//		Encrypt[i].prv = *(u16*)(Encrypt[i].prd%0xF000 + 0x08000000);	//��Flash�������ȡһ��˽Կ
//		
//		Encrypt_Data(&cmd->para.len, Encrypt[i].prv);	//˽Կ��������
//		*(u16*)(&cmd->para.len + cmd->para.len + 1) = Encrypt[i].prd;	//��Կ���������
//		cmd->para.len += sizeof(Encrypt[i].prd);
//		
//		Thread_Login(FOREVER, 0, 1000, ExeSrc_TimeCheck);
//		
//		return TRUE;
//	}
//	return FALSE;
//}

///******************************************************************************* 
//�� �� ����  ExeSrc_TimeCheck
//����˵����  ִ��Դʱ��˶�
//��	  ����  ��
//�� �� ֵ��  ��
//*******************************************************************************/
//static void ExeSrc_TimeCheck(void)
//{
//	u8 i, j;
//	
//	for (i = 0, j = 0; i < ENCRYPT_NUM; i++)
//	{
//		if (Encrypt[i].srcAddr != 0)
//		{
//			if (0 == (--Encrypt[i].time))
//			{
//				Encrypt[i].srcAddr = 0;
//			}
//			else
//			{
//				j++;
//			}
//		}
//	}
//	
//	if (0 == j)
//	{
//		Thread_Logout(ExeSrc_TimeCheck);
//	}
//}	

/**************************Copyright BestFu 2014-05-14*************************/
