/***************************Copyright BestFu 2016-01-28*************************
�� ����    DlyAttrProcess.c
˵ ����    ϵͳ����˳��ִ�д�����
�� �룺    Keil uVision V5.12.0.0
�� ����    v1.0
�� д��    jay
�� �ڣ�    2016-01-28
��  ��:    ��
*******************************************************************************/
#include "Command.h"
#include "BestFuLib.h"
#include "Thread.h"
#include "TimeHandle.h"
#include "WDG.h"

#if (DALAY_EXECUTE_EN > 0u)
//variable define
static DlyExeNode_t DlyExeNode = {0};       //������ʱ����ִ�ж���,����ʼ��
static DlyExeNode_t *pDlyExeNode = &DlyExeNode;

//function declaration
static u8 DlyBufferIsEmpty(void);
static u8 DlyBufferIsFull(void);
static u8 QueryAttrRelateValue(UnitPara_t *data);
static MsgResult_t Msg_ExecuteEx(CMDPara_t *para);
static void DlyMsgProcess(void);
static u8 DeleteRelateNode(UnitPara_t *pData, DlyMsgDelType_t active);
static u8 DeleteBufferNode(u8 pos);
static void DlyRepeatHandle(DlyExeData_t *pDlyExeData, u8 *pos);

/*******************************************************************************
�� �� ����  u8 CreateBufferNode(u8 *data)
����˵����  ����ʱ����ṹ�����һ���ڵ�
��   ����   Ҫ��ӵ�����
�� �� ֵ��  ������ӳɹ�����ʧ��;TRUE(��ӳɹ�),FALSE(��ʾ���ʧ��)
˵    ����  ��
*******************************************************************************/
u8 CreateBufferNode(u8 *data)
{
    static u8 xNodeHasBeenInitialised = 0u;
    u8 pos = 0u;
    if (0 == xNodeHasBeenInitialised)
    {
//      memset(&DlyExeNode,0,sizeof(DlyExeNode_t));
        xNodeHasBeenInitialised = 1u;
        Thread_Login(FOREVER, 0, TRAVERCYCLE, DlyMsgProcess); //������ʱ��������
    }
    if (FALSE == DlyBufferIsFull())                           //��ʱ�������δ��
    {
        pos = ((UnitPara_t *)data)->len + offsetof(UnitPara_t, data);
        DlyMsgFilterProcess((UnitPara_t *)data, RELATE_MSG_DEL);
        memcpy((u8 *)&pDlyExeNode->DlyExeData[DlyExeNode.cnt].unit, (u8 *)data, pos);
//        if (0 == *(u16 *)(&data[pos]))          			 //����ʱ����Ϊ0,������ظ�����
//        {
//            *(u16 *)(&data[pos + offsetof(DlyTimeData_t, repeat)]) = 0;
//        }
        memcpy((u8 *)&pDlyExeNode->DlyExeData[DlyExeNode.cnt].DlyTimeData.delaytime[0], data + pos, sizeof(DlyTimeData_t));
        *(u32 *)&pDlyExeNode->DlyExeData[DlyExeNode.cnt].remaindelaytime[0] = TransformTimePara((u16 *)&data[pos]) * DELAY_PRECISION / TRAVERCYCLE; //��ʱ�ľ���Ϊ0.1s
        *(u32 *)&pDlyExeNode->DlyExeData[DlyExeNode.cnt].remainkeeptime[0] = TransformTimePara((u16 *)&data[pos + offsetof(DlyTimeData_t, keeptime)]) * KEEP_PRECISION / TRAVERCYCLE; //����ʱ��ľ���Ϊ0.1s
        pDlyExeNode->DlyExeData[DlyExeNode.cnt].unitAttrRelate = QueryAttrRelateValue((UnitPara_t *)data);
        pDlyExeNode->cnt++;
		return  TRUE;
    }
    return FALSE;
}

/*******************************************************************************
�� �� ����  static u8 DlyBufferIsEmpty(void)
����˵����     ��ʱ��������Ƿ�Ϊ��
��   ����    ��
�� �� ֵ��  TRUE(����Ϊ��),FALSE(���зǿ�)
˵    ����  ��
*******************************************************************************/
static u8 DlyBufferIsEmpty(void)
{
    return ((0 == pDlyExeNode->cnt) ? (TRUE) : (FALSE));
}

/*******************************************************************************
�� �� ����  static u8 DlyBufferIsFull(void)
����˵����  ��ʱ��������Ƿ�����
��   ����   ��
�� �� ֵ��  TRUE(��������),FALSE(����δ��)
˵    ����  ��
*******************************************************************************/
static u8 DlyBufferIsFull(void)
{
    return ((pDlyExeNode->cnt >= TOTAL_Block_NUM) ? (TRUE) : (FALSE));
}

/*******************************************************************************
�� �� ����  static u8 CreateBufferNode(u8 pos)
����˵����  ����ʱ����ṹ��ɾ��һ���ڵ�
��   ����   pos:Ҫɾ���ڵ��λ��(0~TOTAL_Block_NUM)
�� �� ֵ��  ����ɾ���ɹ�����ʧ��;TRUE(ɾ���ɹ�),FALSE(��ʾɾ��ʧ��)
˵    ����  ��
*******************************************************************************/
static u8 DeleteBufferNode(u8 pos)
{
    if (TRUE == DlyBufferIsEmpty())     //��ʱ��������Ƿ�Ϊ��
    {
        return  FALSE;
    }
    if (pDlyExeNode->cnt > pos)
    {
        if ((pDlyExeNode->cnt - 1) != pos) //���Ҫɾ����λ����ĩβ
        {
            memcpy(&pDlyExeNode->DlyExeData[pos], &pDlyExeNode->DlyExeData[pDlyExeNode->cnt - 1], sizeof(DlyExeData_t));
        }
        memset(&pDlyExeNode->DlyExeData[pDlyExeNode->cnt - 1], 0, sizeof(DlyExeData_t));
        pDlyExeNode->cnt--;
        return  TRUE;
    }
    return  FALSE;
}

/*******************************************************************************
�� �� ����  static u8 DeleteRelateNode(UnitPara_t *data)
����˵����  ɾ����ʱ����ṹ�й����ڵ�
��   ����   pData���´��������
            active:ɾ����Ϣ����
�� �� ֵ��  ����ɾ���ɹ�����ʧ��;TRUE(ɾ���ɹ�),FALSE(��ʾɾ��ʧ��)
˵    ����  ��
*******************************************************************************/
static u8 DeleteRelateNode(UnitPara_t *pData, DlyMsgDelType_t active)
{
    u8 pos = 0u;
    u8 result = FALSE;
    u8 relatevalue = 0u;

    switch (active)
    {
    case SAME_MSG_DEL:
    {
        for (pos = 0; pos < pDlyExeNode->cnt; pos++)
        {
            if ((pDlyExeNode->DlyExeData[pos].unit == ((UnitPara_t *)pData)->unit) &&
                    (pDlyExeNode->DlyExeData[pos].Cmd == ((UnitPara_t *)pData)->cmd))
            {
                result = DeleteBufferNode(pos);
                pos--;
            }
        }
    }
    break;
    case RELATE_MSG_DEL:
    {
        relatevalue = QueryAttrRelateValue(pData);
        if (relatevalue > 0u)                            //��������ֵΪ0ʱ,��ɾ���κ�����
        {
            for (pos = 0; pos < pDlyExeNode->cnt; pos++)
            {
                if (pDlyExeNode->DlyExeData[pos].unit == ((UnitPara_t *)pData)->unit)
                {
					if((UNIT_PROPERTY_RELATE == relatevalue)||\
						(pDlyExeNode->DlyExeData[pos].unitAttrRelate == relatevalue))
					{
						result = DeleteBufferNode(pos);
						pos--;
					}
                }
            }
        }
    }
    break;
    default:
        break;
    }
    return (result);
}

/*******************************************************************************
�� �� ����  static u8 QueryAttrRelateValue(UnitPara_t* data)
����˵����     ��ѯ��ǰ��Ԫ���Թ���ֵ
��   ����    pData���´��������
�� �� ֵ��  ���ص�ǰ��Ԫ���ԵĹ���ֵ
˵    ����  ��
*******************************************************************************/
static u8 QueryAttrRelateValue(UnitPara_t *para)
{
    const AttrExe_st *pAttrExe = NULL;
    u8 result = FALSE;
    u8 ExeAttrNum = 0u;
    if (para->unit >= UnitCnt_Get())
    {
        return FALSE;
    }
    pAttrExe = UnitTab[para->unit].attrExe;
    do                                              //���������б�
    {
        if (pAttrExe->cmd == para->cmd)             //���Ժ�ƥ��OK
        {
            result = pAttrExe->unitAttrRelate;
            break;
        }
    }
    while (((++ExeAttrNum) < UnitTab[para->unit].ExeAttrNum())   //2016.01.29 jay modify
            && ((++pAttrExe)->cmd < 0x0100));
    return (result);
}

/*******************************************************************************
�� �� ����  u8 DlyMsgFilterProcess(UnitPara_t* para)
����˵����  ���˵�Ԫ�Ĺ�����Ϣ
��   ����   pData���´��������
            active:ɾ����Ϣ����
�� �� ֵ��  ���ع��˵Ľ��
˵    ����  ��
*******************************************************************************/
u8 DlyMsgFilterProcess(UnitPara_t *para, DlyMsgDelType_t active)
{
    if (para->unit >= UnitCnt_Get())
    {
        return FALSE;
    }
    if (TRUE == DlyBufferIsEmpty())
    {
        return FALSE;
    }
    if (0x00 != para->unit)                 //ϵͳ��Ԫִ�����Բ�������Ϣ����
    {
        return DeleteRelateNode(para, active);
    }
    return FALSE;
}

/*******************************************************************************
�� �� ����  u8 DeleteDlyAssMsg(u8 unit,u8 Cmd)
����˵����     ɾ���͵�ǰ������ص���ʱ��������
��   ����    unit:��Ԫ��
            Cmd:�����
�� �� ֵ��  ���ع��˵Ľ��
˵    ����  ��
*******************************************************************************/
u8 DeleteDlyAssMsg(u8 unit, u8 Cmd)
{
    u8 data[8];
    data[0] = unit;
    data[1] = Cmd;
    data[2] = 0;
    return DlyMsgFilterProcess((UnitPara_t *)data, RELATE_MSG_DEL);
}

/*******************************************************************
�� �� ����  static MsgResult_t Msg_ExecuteEx(CMDPara_t *para)
����˵����     ��Ϣִ��
��   ����    data:   Ҫִ�е���������
            len;    ���ݳ���
�� �� ֵ��  TRUE(�ظ�)/FALSE(���ظ�)
*******************************************************************/
static MsgResult_t Msg_ExecuteEx(CMDPara_t *para)
{
    MsgResult_t result = NO_CMD;
    u8 ExeAttrNum = 0u;
    const AttrExe_st *pAttrExe = UnitTab[para->unit].attrExe;

    if (para->unit < UnitCnt_Get()) //ָ�Ԫû��Խ��  2014.01.08 Unarty add ��ֹ����Խ��
    {
        do  //���������б�
        {
            if (pAttrExe->cmd == para->cmd) //���Ժ�ƥ��OK
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
                        result =  pAttrExe->pSet_Attr((UnitPara_t *)&para->unit);
                    }
                }
                break;
            }//End Of if (pAttrExe->cmd == para->cmd)
        }
        while (((++ExeAttrNum) < UnitTab[para->unit].ExeAttrNum()) //2016.01.29  jay modify
                && ((++pAttrExe)->cmd < 0x0100));
    }//End of (para->unit < UnitCnt_Get())
    return result;
}

/*******************************************************************
�� �� ����  static void DlyRepeatHandle(DlyExeData_t *pDlyExeData, u8 pos)
����˵����  ��ʱ�ظ�ִ�к���
��   ����   pDlyExeData:    ������ʱִ����������ָ��
            pos ����ǰ��ʱ��������ʱִ�������е�λ��ָ��
�� �� ֵ��  ��
*******************************************************************/
static void DlyRepeatHandle(DlyExeData_t *pDlyExeData, u8 *pos)
{
    if (pDlyExeData->DlyTimeData.repeat > 0u)
    {
        if (0xff != pDlyExeData->DlyTimeData.repeat)
        {
            pDlyExeData->DlyTimeData.repeat -- ;
        }
        *(u32 *)(&pDlyExeData->remaindelaytime[0]) = TransformTimePara((u16 *)(&pDlyExeData->DlyTimeData.delaytime[0])) * DELAY_PRECISION / TRAVERCYCLE;
        *(u32 *)(&pDlyExeData->remainkeeptime[0])  = TransformTimePara((u16 *)(&pDlyExeData->DlyTimeData.keeptime[0])) * KEEP_PRECISION / TRAVERCYCLE;		     
    }
    else
    {
        DeleteBufferNode(*pos);
        (*pos)--;
    }//End of if(pDlyExeData->DlyTimeData.repeat > 0u)
}

/*******************************************************************************
�� �� ����  static void DlyMsgProcess(void)
����˵����  ������Ϣ������
��   ����   ��
�� �� ֵ��  ��
˵    ����  ��
*******************************************************************************/
static void DlyMsgProcess(void)
{
    u8 pos = 0;
    u8 buf[20];
	
	WDG_Feed();
    if (TRUE == DlyBufferIsEmpty())	return ;
    for (pos = 0; pos < pDlyExeNode->cnt; pos ++)
    {
        if (*(u32 *)(&pDlyExeNode->DlyExeData[pos].remaindelaytime[0]) >= 1)
        {
            *(u32 *)(&pDlyExeNode->DlyExeData[pos].remaindelaytime[0]) -= 1;
            pDlyExeNode->DlyExeData[pos].delaytaskexeflag = 0;
        }
        else
        {
            *(u32 *)(&pDlyExeNode->DlyExeData[pos].remaindelaytime[0]) = 0;
            if (0 == pDlyExeNode->DlyExeData[pos].delaytaskexeflag)
            {
                //����ִ�к���
                buf[0] = WRITENACK;
                memcpy(&buf[1], &pDlyExeNode->DlyExeData[pos].unit, 3 + pDlyExeNode->DlyExeData[pos].len);
                Msg_ExecuteEx((CMDPara_t *)buf);
                pDlyExeNode->DlyExeData[pos].delaytaskexeflag = 1;
                pDlyExeNode->DlyExeData[pos].keeptaskexeflag = 0;
            }
            else// if(0 != pDlyExeNode->DlyExeData[pos].delaytaskexeflag)
            {
                if (0 != CheckTimeParaIsZero(*(u16 *)(&pDlyExeNode->DlyExeData[pos].DlyTimeData.keeptime[0])))
                {
                    if (*(u32 *)(&pDlyExeNode->DlyExeData[pos].remainkeeptime[0]) >= 1)
                    {
                        *(u32 *)(&pDlyExeNode->DlyExeData[pos].remainkeeptime[0]) -= 1;
                        pDlyExeNode->DlyExeData[pos].keeptaskexeflag = 0;
                    }
                    else
                    {
                        *(u32 *)(&pDlyExeNode->DlyExeData[pos].remainkeeptime[0]) = 0;
                        if (0 == pDlyExeNode->DlyExeData[pos].keeptaskexeflag)
                        {
                            //����ִ�к���
                            buf[0] = WRITENACK;
                            pDlyExeNode->DlyExeData[pos].data[0] ^= 0x01;                   //ֻʵ�ֿ��ص���ʱ�����ظ�
                            memcpy(&buf[1], &pDlyExeNode->DlyExeData[pos].unit, 3 + pDlyExeNode->DlyExeData[pos].len);
                            pDlyExeNode->DlyExeData[pos].data[0] ^= 0x01;                   //ֻʵ�ֿ��ص���ʱ�����ظ�
                            Msg_ExecuteEx((CMDPara_t *)buf);
                            pDlyExeNode->DlyExeData[pos].keeptaskexeflag = 1;
                            pDlyExeNode->DlyExeData[pos].delaytaskexeflag = 0;
                            DlyRepeatHandle(&pDlyExeNode->DlyExeData[pos], &pos);
                        }//End of if(0 == pDlyExeNode->DlyExeData[pos].keeptaskexeflag)
                    }//End of if(*(u16*)(&pDlyExeNode->DlyExeData[pos].remaindelaytime[0]) > TRAVERCYCLE)
                }
                else
                {
                    DlyRepeatHandle(&pDlyExeNode->DlyExeData[pos], &pos);
                }//End of if(0 != *(u16*)(&pDlyExeNode->DlyExeData[pos].DlyTimeData.keeptime[0]))
            }//End of else if(1 == pDlyExeNode->DlyExeData[pos].delaytaskexeflag)
        }//End of if(*(u16*)(&pDlyExeNode->DlyExeData[pos].remaindelaytime[0]) > TRAVERCYCLE)
    }
}
#endif
/******************************End of File************************************/
