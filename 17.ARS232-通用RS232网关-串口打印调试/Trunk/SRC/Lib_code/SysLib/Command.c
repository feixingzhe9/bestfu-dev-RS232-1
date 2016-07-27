/***************************Copyright BestFu 2014-05-14*************************
��	����    command.c
˵	��:	    �Ӷ����л�ȡһ��ָ��
��	�룺    Keil uVision4 V4.54.0.0
��	����    v1.0
��	д��    Unarty
��	�ڣ�    2013-10-14
��  ��:     ����
*******************************************************************************/
#include "command.h"
#include "fifo.h"
#include "crc.h"
#include "DataEncrypt.h"
#include "MsgPackage.h"
#include "Updata.h"
#include "stdio.h"


const u8 HeadCMD[HEAD_SIZE] = {0xF1, 0xF1, 0xF1, 0xF2, 0xF1, 0xF1};
const u8 TailCMD[TAIL_SIZE] = {0xF2, 0xF2, 0xF2, 0xF1, 0xF2, 0xF2};

/*******************************************************************************
�� �� ����  head_put
����˵����  �����з���һ�����ݱ�ʶͷ
��	  ����  fifo:	Ŀ�����
�� �� ֵ��  ���ý����TRUR(���óɹ�)/FASLE
*******************************************************************************/
u8 head_put(FIFO_t *fifo)
{
	return fifo_puts(fifo, (u8*)HeadCMD, sizeof(HeadCMD));
}

/*******************************************************************************
�� �� ����  tail_put
����˵����  �����з���һ�����ݱ�ʶβ
��	  ����  fifo:	Ŀ�����
�� �� ֵ��  ���ý����TRUR(���óɹ�)/FASLE
*******************************************************************************/
u8 tail_put(FIFO_t *fifo)
{
	return fifo_puts(fifo, (u8*)TailCMD, sizeof(TailCMD));
}

#if 0
/*******************************************************************************
�� �� ����  cmd_get
����˵����  �Ӷ����л�ȡһ��ָ��
��	  ����  fifo:	��ȡԴ
            cmdBuf: ָ��洢������	
�� �� ֵ��  ָ��ȡ�Ϊ�㣬û�пɶ�ָ��
*******************************************************************************/
u32 cmd_get(u8 *cmdBuf, FIFO_t *fifo)
{
	u32 i;
     
	ERRR(fifo == NULL, goto ERR1);
	
	ERRR(fifo_empty(fifo) == TRUE, goto ERR1);        //FIFOΪ��
	i = fifo_find(fifo, HeadCMD, sizeof(HeadCMD));
	fifo->front = i;
	ERRR(fifo_empty(fifo) == TRUE, goto ERR1);        //FIFOΪ��

	i = sizeof(Communication_t) + sizeof(HeadCMD);
	ERRR(i > fifo_validSize(fifo), goto ERR1); //���ݿռ䲻��

	i += fifo->data[(fifo->front + i - ARGE_SEAT)%fifo->size] + CRC_SIZE + sizeof(TailCMD);
	ERRR(i > CMD_SIZE, goto ERR2);	//����ָ��ȴ���
	ERRR(i > fifo_validSize(fifo), goto ERR1); //���ݿռ䲻��
	
	ERRR(fifo_cmp(fifo, (fifo->front + i - sizeof(TailCMD))%fifo->size
					, TailCMD, sizeof(TailCMD)) == FALSE, goto ERR2);	//�˶�����β
					
	ERRR(fifo_gets(fifo, cmdBuf, i) == FALSE, goto ERR1);	//�������ݳ���
	ERRR(ChkCrcValue(&cmdBuf[sizeof(HeadCMD)], (i - sizeof(HeadCMD) - sizeof(TailCMD))), goto ERR1);	//CRC����
	
	return i;

ERR2:	fifo->front = (fifo->front+sizeof(HeadCMD))%fifo->size; //���ݴ���ͷ�Ƴ�
ERR1:	return 0;	
}
/*******************************************************************************
�� �� ����  cmd_put
����˵����  ����һ��ָ�FIFO��
��	  ����  fifo:	Ŀ���ַ
            cmdBuf: ָ��洢������	
			cmdLen:	�������ݳ���
�� �� ֵ��  TRUE-�ɹ���FALSE-ʧ��
*******************************************************************************/
u32 cmd_put(FIFO_t *fifo, u8 *cmdBuf, u32 cmdLen)
{
	u16 crc;
	
	ERRR(fifo == NULL, goto ERR1);
	ERRR(fifo_puts(fifo, (u8*)HeadCMD, sizeof(HeadCMD)) == FALSE, goto ERR1);	//����ͨ�ű�־ͷ
	crc = ChkCrcValue(cmdBuf, cmdLen);	//��ȡCRC
	ERRR(fifo_puts(fifo, cmdBuf, cmdLen) == FALSE, goto ERR1);	//���ò�����
	ERRR(fifo_putc(fifo, (u8)(crc>>8)) == FALSE, goto ERR1);	//����CRC��λ	
	ERRR(fifo_putc(fifo, (u8)(crc)) == FALSE, goto ERR1);		//����CRC��λ	
	ERRR(fifo_puts(fifo, (u8*)TailCMD, sizeof(TailCMD)) == FALSE, goto ERR1);	//����ͨ�ű�β
	
	return TRUE;
ERR1:	return FALSE;
}
#endif

#if (COMMUNICATION_SHORT_FRAME == 1)
/*******************************************************************************
�� �� ����  short_get
����˵����  �Ӷ����л�ȡһ����ָ֡��
��	  ����  fifo:	��ȡԴ
            cmdBuf: ָ��洢������	
�� �� ֵ��  ָ��ȡ�Ϊ�㣬û�пɶ�ָ��
*******************************************************************************/
u32 short_get(u8 *cmdBuf, FIFO_t *fifo)
{
	u32 i;
     
	ERRR(fifo == NULL, goto ERR1);
	
	ERRR(fifo_empty(fifo) == TRUE, goto ERR1);        //FIFOΪ��
	i = fifo_find(fifo, HeadCMD, sizeof(HeadCMD));
	fifo->front = i;
	ERRR(fifo_empty(fifo) == TRUE, goto ERR1);        //FIFOΪ��
	
	i = sizeof(HeadCMD) + 1; //֡ͷ�Ͷ�֡�����ֽ�
	i += fifo->data[(fifo->front + i - ARGE_SEAT)%fifo->size] + CRC_SIZE + sizeof(TailCMD);
	ERRR(i > CMD_SIZE,goto ERR2);	//����ָ��ȴ���
	ERRR(i > fifo_validSize(fifo), goto ERR1); //���ݿռ䲻��
	
	ERRR(fifo_cmp(fifo, (fifo->front + i - sizeof(TailCMD))%fifo->size
					, TailCMD, sizeof(TailCMD)) == FALSE, goto ERR2);	//�˶�����β
				
	ERRR(fifo_gets(fifo, cmdBuf, i) == FALSE, goto ERR1);	//�������ݳ���
	i = cmdBuf[sizeof(HeadCMD)]; //ȡ��Ч�ֽڳ���
	ERRR(ChkCrcValue(cmdBuf+sizeof(HeadCMD)+1, i+2), goto ERR1);	//CRC����
	
	memmove(cmdBuf,cmdBuf+sizeof(HeadCMD)+1,i); //�Ƴ�����ͷ
	return i;

ERR2:	fifo->front = (fifo->front+sizeof(HeadCMD))%fifo->size; //���ݴ���ͷ�Ƴ�
ERR1:	return 0;
}
/*******************************************************************************
�� �� ����  short_put
����˵����  ����һ����ָ֡�FIFO��
��	  ����  fifo:	Ŀ���ַ
            cmdBuf: ָ��洢������	
			cmdLen:	�������ݳ���
�� �� ֵ��  TRUE-�ɹ���FALSE-ʧ��
*******************************************************************************/
u32 short_put(FIFO_t *fifo, u8 *cmdBuf, u32 cmdLen)
{
	u16 crc;
	
	ERRR(fifo == NULL, goto ERR1);
	ERRR(fifo_puts(fifo, (u8*)HeadCMD, sizeof(HeadCMD)) == FALSE, goto ERR1);	//����ͨ�ű�־ͷ
	crc = ChkCrcValue(cmdBuf, cmdLen);	//��ȡCRC
	ERRR(fifo_putc(fifo, cmdLen) == FALSE, goto ERR1);	//��֡����byte
	ERRR(fifo_puts(fifo, cmdBuf, cmdLen) == FALSE, goto ERR1);	//���ò�����
	ERRR(fifo_putc(fifo, (u8)(crc>>8)) == FALSE, goto ERR1);	//����CRC��λ	
	ERRR(fifo_putc(fifo, (u8)(crc)) == FALSE, goto ERR1);		//����CRC��λ	
	ERRR(fifo_puts(fifo, (u8*)TailCMD, sizeof(TailCMD)) == FALSE, goto ERR1);	//����ͨ�ű�β
	
	return TRUE;
ERR1:	return FALSE;
}
#endif
/*******************************************************************************
�� �� ����  EncryptCmd_get
����˵����  �Ӷ����л�ȡһ������ָ��
��	  ����  fifo:	��ȡԴ
            cmdBuf: ָ��洢������	
�� �� ֵ��  ָ��ȡ�Ϊ�㣬û�пɶ�ָ��
*******************************************************************************/
u32 EncryptCmd_get(u8 *cmdBuf, FIFO_t *fifo)
{
	u32 i;
     
	ERRR(fifo == NULL, goto ERR1);
	
	ERRR(fifo_validSize(fifo) <= sizeof(HeadCMD), goto ERR1);        //FIFO��Ч����С��ͷ,�����
	i = fifo_find(fifo, HeadCMD, sizeof(HeadCMD));
	fifo->front = i;
	
	ERRR(fifo_empty(fifo) == TRUE, goto ERR1);        //FIFOΪ��
	i = fifo->data[(fifo->front + sizeof(HeadCMD))%fifo->size]; //��ȡ��Чָ���
	ERRR(i == COMMUNICATION_VERSION, goto ERR2);	 //��ͨ��Э��ֱ֡���Ƴ� yanhuan adding 2016/01/13 
	i += sizeof(HeadCMD) + sizeof(TailCMD);		//��ȡ�������ܳ���
	ERRR(i > CMD_SIZE, {printf("error:data too long \n");goto ERR2;});	//����ָ��ȴ���
	ERRR(i > fifo_validSize(fifo), {printf("error:space error \n");goto ERR1;}); //���ݿռ䲻��

	ERRR(fifo_cmp(fifo, (fifo->front + i - sizeof(TailCMD))%fifo->size
        , TailCMD, sizeof(TailCMD)) == FALSE, {printf("error:tail error \n");goto ERR2;});	//�˶�����β
	ERRR(fifo_gets(fifo, cmdBuf, i) == FALSE, {printf("error:get error \n");goto ERR1;});	//�������ݳ���

	return i;

ERR2:	fifo->front = (fifo->front+sizeof(HeadCMD))%fifo->size; //���ݴ���ͷ�Ƴ�
ERR1:	return 0;	
}

/*******************************************************************************
����  ��:  BF_INT32U EncryptCmd_getEx(BF_INT08U *cmdBuf, FIFO_t *fifo)
����˵��:  �Ӽ��ܶ����л�ȡһ֡����,����������
��    ��:  fifo��������е��׵�ַ
		   cmdBuf���洢�������ݵ��׵�ַ
����  ֵ:  ��
*******************************************************************************/
u32 EncryptCmd_getEx(u8 *cmdBuf, FIFO_t *fifo)
{
    u32 pos = EncryptCmd_get(cmdBuf, fifo);

	if(pos && (!DecryptData((u8*)(&cmdBuf[HEAD_SIZE]))))	//�����ܳɹ�
	{
		pos = HEAD_SIZE + sizeof(Communication_t) + ((Communication_t*)&cmdBuf[HEAD_SIZE])->para.len + CRC_SIZE;
		memcpy(&cmdBuf[pos], (u8*)TailCMD, TAIL_SIZE); 
		pos += TAIL_SIZE;
	}
//    else
//    {
//        pos = 0;
//        printf("error:DecryptData error \n");   
//    }
    return (pos);
}

/*******************************************************************************
�� �� ����  EncryptCmd_put
����˵����  ����һ��ָ�����õ�FIFO��
��	  ����  fifo:	Ŀ���ַ
            cmdBuf: ָ��洢������	
			cmdLen:	�������ݳ���
�� �� ֵ��  ָ��ȡ�Ϊ�㣬û�пɶ�ָ��
*******************************************************************************/
u32 EncryptCmd_put(FIFO_t *fifo, u8 *cmdBuf, u32 cmdLen)
{
	u16 crc;
	u8 cmdBuf_temp[256];

	ERRR(fifo == NULL, goto ERR1);
	ERRR(cmdLen > CMD_SIZE, goto ERR1);

	memcpy(cmdBuf_temp, cmdBuf, cmdLen);
	crc = ChkCrcValue(cmdBuf_temp, cmdLen);	//��ȡCRC
	ERRR(fifo_puts(fifo, (u8*)HeadCMD, sizeof(HeadCMD)) == FALSE, goto ERR1);	//����ͨ�ű�־ͷ
	cmdBuf_temp[cmdLen++] = (u8)(crc >> 8);
	cmdBuf_temp[cmdLen++] = (u8)(crc);		//�������CRC
	
	ERRR(!EncryptData(cmdBuf_temp, cmdLen), goto ERR1);
	cmdLen = cmdBuf_temp[0];
	ERRR(cmdLen > CMD_SIZE, goto ERR1);
	ERRR(fifo_puts(fifo, cmdBuf_temp, cmdLen) == FALSE, goto ERR1);	//���ò�����
	ERRR(fifo_puts(fifo, (u8*)TailCMD, sizeof(TailCMD)) == FALSE, goto ERR1);	//����ͨ�ű�β
	
	return TRUE;
ERR1:	return FALSE;
}
/**************************Copyright BestFu 2014-05-14*************************/
