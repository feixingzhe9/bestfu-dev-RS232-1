/***************************Copyright BestFu 2014-05-14*************************
��	����    cmdProcess.c
˵	����    FIFOָ�����
��	�룺    Keil uVision4 V4.54.0.0
��	����    v1.0
��	д��    Unarty
��	�ڣ�    2013-11-01
��  ��:     2014.12.28 Unarty �޸ĵ�Ԫ����ִ�б߽������ж�ֵ����ֹ�൥Ԫִ��ʱ����Խ��
			2014.01.08 Unarty ���ӵ�Ԫ��ֵ�˶� ��ֹ����Խ��
*******************************************************************************/
#include "fifo.h"
#include "command.h"
#include "cmdProcess.h"
#include "Object.h"
#include "UnitCfg.h"
#include "MsgPackage.h"
#include "UnitShare_Attr.h"
#include "SI4432.h"
#include "Thread.h"
#include "Instruct.h"
#include "SysTick.h"
#include "AttrEncrypt.h"
#include "DataEncrypt.h"
#include "Updata.h"

#if STM32L151 
extern  void StandbyCountReset(void);
#endif 

static u8 Msg_Process(u8 *sdata, u8 len, u8 level);
FIFO_t rcv433fifo;
FIFO_t send433fifo;

u8 rcvBuf[FIFOBUFSZE];
static u8 sendBuf[FIFOBUFSZE];

Process_t rcv_433;
Process_t send_433;

#if (COMMUNICATION_SHORT_FRAME == 1)
FIFO_t rcv433_shortfifo;
FIFO_t send433_shortfifo;  //433��֡��������

u8 rcvshortBuf[ADDFIFOBUFSZ];
static u8 sendshortBuf[ADDFIFOBUFSZ];

Process_t rcv_433short;
Process_t send_433short;

//static u8 ShortMsg_Level(u8 version) ;	
static u8 ShortMsg_Process(u8 *data, u8 len, u8 level); //��֡��Ϣ���� yanhuan adding 2015/12/29
#endif

static u8   Msg_ReturnType(u8 msgType);
static void DataSend_Thread(void);
//static u8   Msg_Level(u32 object);		////����ͨ��ƥ�䣨Unarty 2014.08.07)

/*****************************************************************************
  ������:  void sendFIFOInit(void)
����˵��: 	����FIFO���г�ʼ��
    ����:  	void
  ����ֵ:  	void
*****************************************************************************/
void FIFOInit(void)
{
	fifo_Init(&rcv433fifo, rcvBuf, FIFOBUFSZE);
	fifo_Init(&send433fifo, sendBuf, FIFOBUFSZE);

	PROCESS_INIT(&rcv_433, 0, NORMAL, COMMON, Msg_Process, EncryptCmd_getEx);
	PROCESS_INIT(&send_433, 0, MANY_P, COMMON, Si4432_Send, EncryptCmd_get);

#if (COMMUNICATION_SHORT_FRAME == 1)//��֡������� yanhuan adding 2015/12/28
	fifo_Init(&rcv433_shortfifo, rcvshortBuf, ADDFIFOBUFSZ);
	fifo_Init(&send433_shortfifo, sendshortBuf, ADDFIFOBUFSZ);	
	
	PROCESS_INIT(&rcv_433short, 0, NORMAL, SHORT, ShortMsg_Process, short_get);
	PROCESS_INIT(&send_433short, 0, MANY_P, SHORT, Si4432_Send, short_get);

#endif	
	Thread_Login(FOREVER, 0, 5, &DataSend_Thread);
}

/*****************************************************************************
  ������:  	ProcessFun
����˵��: 	�����ͺ����޸�
	����:  	rcveFun:	���մ�����
			sendFun:	���ʹ�����
  ����ֵ:  	void
*****************************************************************************/
void ProcessSend_Fun(ProcessFunction sendFun)
{
	send_433.fun = sendFun;
}

/*****************************************************************************
  ������:  	DataSend_Thread
����˵��: 	���ݷ����߳�
	����:  	void
  ����ֵ:  	void
*****************************************************************************/
void DataSend_Thread(void)
{
	Process_FIFOData(&send433fifo, &send_433);		//������FIFO
	
#if (COMMUNICATION_SHORT_FRAME == 1)
	Process_FIFOData(&send433_shortfifo, &send_433short);		//�����Ͷ�֡FIFO
#endif
}

/*******************************************************************
�� �� ����	Send_FIFOData
����˵���� 	��433�����е�����ͨ��433���ͳ�ȥ
��	  ���� 	fifo:	����Ŀ��FIFO
			send:	�����͵���������
			fun:	���ͺ���
�� �� ֵ��	��
*******************************************************************/
void Process_FIFOData(FIFO_t *fifo, Process_t *prc)
{
    switch (prc->flag)
    {
    case 0:
        if (0 == (prc->len = prc->Data_Get(prc->data, fifo))) //��FIFO�л�ȡһ��ָ��ʧ��
        {
            break;
        }
//#if (COMMUNICATION_SHORT_FRAME == 1)
//				prc->level = (prc->type == COMMON)? \
//				   Msg_Level(((Communication_t*)&prc->data[HEAD_SIZE])->aim.object.id)\
//			            	:ShortMsg_Level(prc->data[0]);
//#else
//				prc->level = Msg_Level(((Communication_t*)&prc->data[HEAD_SIZE])->aim.object.id);
//#endif
        prc->level = 0;
        prc->mean = (prc->con == MANY_P ? gSysData.sendCnt : 1);//��ȡ���ݴ������
    case 1:
        if (FALSE == (prc->fun(prc->data, prc->len, prc->level)) || --prc->mean) //���ݴ���δ���
        {
            prc->flag = 1;
            break;
        }
    default:
        UpdateRandomTime(); //�������ʱ��   yanhuan adding 2015/12/02
        prc->flag = 0;
        break;
    }
}
#if (COMMUNICATION_SHORT_FRAME == 1)
/*******************************************************************
�� �� ����	ShortMsg_Process
����˵���� 	��֡��Ϣ������
��	  ���� 	pMsg:	��Ϣ����
			*data;	����������
�� �� ֵ��	��
*******************************************************************/
u8 ShortMsg_Process(u8 *data, u8 len, u8 level)
{  
  ShortCommunication_t *pMsg = (ShortCommunication_t*)data;
	if(COMPLETE == ShortObject_Check(pMsg , len)) //��֡�˶Գɹ�
	{
		switch(pMsg->aim.version)
		{
			case SYSTICK_SHORT_VERSION:
				break ;
			default:
				break ;
		}
	}
	return TRUE;
}

///*******************************************************************
//�� �� ����	ShortMsg_Level
//����˵���� 	��֡��Ϣִ�еȼ�����Ҫ�����ݷ���
//��	  ���� 	��ϢĿ��
//�� �� ֵ��	��Ϣ�ȼ�
//*******************************************************************/
//u8 ShortMsg_Level(u8 version)
//{
//    if (SYSTICK_SHORT_VERSION == version)	//�����ϱ�
//    {
//        return 2;
//    }
//    return 0;
//}

#endif
/*******************************************************************
�� �� ����	Msg_Process
����˵���� 	��Ϣ������
��	  ���� 	pMsg:	��Ϣ����
			*data;	����������
�� �� ֵ��	��
*******************************************************************/
u8 Msg_Process(u8 *sdata, u8 len, u8 level)
{
    MsgResult_t result; 
	Communication_t *pMsg = (Communication_t*)&sdata[HEAD_SIZE];  
	
	//�˶�ͨ�Ű汾	pMsg->version;
	if ((result = Object_Check(sdata, len, &pMsg->aim, &pMsg->para, 
		             &sdata[HEAD_SIZE+sizeof(Communication_t)])) == COMPLETE) //Ŀ��˶Գɹ�
	{
		/*ָ��ִ�� pMsg->para ; data*/
		result =  Msg_Execute(&pMsg->para);
	    /*ָ���*/
		if (result != NO_ACK)
		{
#if STM32L151 		//ר���ڵ͹��Ĳ�Ʒ�� ����ʱ������
			StandbyCountReset();
#endif 
			Instruct();		//�ź�ָʾ
			if (WRITEWITHACK == pMsg->para.msgType							//дӦ��
				|| READWITHACK == pMsg->para.msgType						//��Ӧ��
				|| ((WRITESUC == pMsg->para.msgType 
					|| READSUC == pMsg->para.msgType) && result == COMPLETE) 	//д/���ɹ�Ӧ���Ҳ����ɹ�
				|| ((WRITEFAIL == pMsg->para.msgType 
					|| READFAIL == pMsg->para.msgType) && result != COMPLETE) 	//д/��ʧ��Ӧ���Ҳ���ʧ��
				)	//��Ϣ��ҪӦ��
			{	
				pMsg->para.msgType = Msg_ReturnType(pMsg->para.msgType);	//��Ϣִ�н��
				Msg_Feedback(pMsg);
			}
		}
	}
	return TRUE;
}

/*******************************************************************
�� �� ����	Msg_Execute
����˵���� 	��Ϣִ��
��	  ���� 	data:	Ҫִ�е���������
			len;	���ݳ���
�� �� ֵ��	TRUE(�ظ�)/FALSE(���ظ�)
*******************************************************************/
MsgResult_t Msg_Execute(CMDPara_t *para)
{
    MsgResult_t result = NO_CMD;
    const AttrExe_st *pAttrExe = UnitTab[para->unit].attrExe;
    u8 ExeAttrNum = 0u;
    u8 ackLen = 0;

    if (para->unit < UnitCnt_Get())	//ָ�Ԫû��Խ��	2014.01.08 Unarty add ��ֹ����Խ��
    {
        do  //���������б�
        {
            if (pAttrExe->cmd == para->cmd)	//���Ժ�ƥ��OK
            {
                /*д����*/
                if (WRITEWITHACK == para->msgType
                        || WRITENACK == para->msgType
                        || WRITESUC  == para->msgType
                        || WRITEFAIL == para->msgType
                   )
                {
                    if (pAttrExe->pSet_Attr != NULL)
                    {
                        if (!gUnitData[para->unit].able)	//��Ԫ������
                        {
                            result = UNIT_DISABLE;
                            Upload(para->unit);
                        }
                        else
                        {
#if (DALAY_EXECUTE_EN > 0u)
                            //��Ϣ����
                            DlyMsgFilterProcess((UnitPara_t*)&para->unit,RELATE_MSG_DEL);		//
#endif
                            result =  pAttrExe->pSet_Attr((UnitPara_t*)&para->unit);
                            if (result == OWN_RETURN_PARA) //���д���������ز���
                            {
                                if (pAttrExe->cmd > 0xCF)
                                {
                                    result = COMPLETE;
                                }
                                ackLen = para->len;
                                memmove((&para->len) + 2, (&para->len) + 1, ackLen);  //V1.8 add
                            }
                        }
                    }
                    else
                    {
                        result = NO_WRITE;	//û��дȨ��
                    }
                }
                /*������*/
                else if (READWITHACK == para->msgType
                         || READSUC == para->msgType
                         || READFAIL == para->msgType
                        )
                {
                    if (pAttrExe->pGet_Attr != NULL)
                    {
                        result = pAttrExe->pGet_Attr((UnitPara_t*)&para->unit, &ackLen, ((&para->len) + 2));//((&para->len) + 1));V1.8 Chang
                    }
                    else
                    {
                        result = NO_READ;	//û�ж�Ȩ��
                    }
                }
                else
                {
                    result = MSG_TYPE_ERR;	//��Ϣ���ʹ���
                }
                break;
            }
        }
        while (((++ExeAttrNum) < UnitTab[para->unit].ExeAttrNum())   //2016.01.29 jay modify
                &&((++pAttrExe)->cmd < 0x0100));
        /*while((pAttrExe->cmd < (++pAttrExe)->cmd)		//2014.12.28 Unarty Add
        		&& (pAttrExe->cmd < 0x0100));*/
    }

    para->len = ackLen + 1;
    *((&para->len)+1) = result;		//V1.8 ADD

    return result;
}

/*******************************************************************
�� �� ����	Msg_ReturnType
����˵���� 	��Ϣ��������
��	  ���� 	ִ����Ϣ����
�� �� ֵ��	������ϢӦ������
*******************************************************************/
u8 Msg_ReturnType(u8 msgType)
{
    switch (msgType)
    {
    case WRITEWITHACK:
    case WRITESUC:
    case WRITEFAIL:
        return ACKWRITE;
    case READWITHACK:
    case READSUC:
    case READFAIL:
        return READACK;
    default :
        break;
    }
    return 0;
}


///*******************************************************************
//�� �� ����	Msg_Level
//����˵���� 	��Ϣִ�еȼ�����Ҫ�����ݷ���
//��	  ���� 	��ϢĿ��
//�� �� ֵ��	��Ϣ�ȼ�
//*******************************************************************/
//u8 Msg_Level(u32 object)
//{
//    if (MSG_UPLOAD_ID == object)	//�����ϱ�
//    {
//        return 2;
//    }
//    else if (DelayID == object)		//Ŀ�����������ʱӦ��
//    {
//        return 3;
//    }
//    return 0;
//}

/**************************Copyright BestFu 2014-05-14*************************/
