/***************************Copyright BestFu 2014-05-14*************************
��	����	LinkScanFifo.c
˵	����	���Ա仯�������������
��	�룺	Keil uVision4 V4.54.0.0
��	����	v1.0
��	д��	Joey
��	�ڣ�	2013.7.22 
�ޡ��ģ�	Unarty 2014.11.18 �޸��豸����״̬������ָ�200��220����������
*******************************************************************************/

#include "linkall.h"
#if 1
FIFO_t  propfifo;             // ���Ա仯ʵ��
unsigned char propbuf[PROPBUFSIZE]; // ���Ա仯������

/*******************************************************************************
�� �� ����	PropFifoInit
����˵���� 	���г�ʼ��
��	  ���� 	void
�� �� ֵ��	void
*******************************************************************************/
void PropFifoInit(void)
{
	fifo_Init(&propfifo, propbuf, PROPBUFSIZE);
}
/*******************************************************************************
�� �� ����	PropChangeScanAndExe
����˵���� 	���Ա仯ɨ�貢ִ��
��	  ���� 	void
�� �� ֵ��	void
*******************************************************************************/
unsigned char PropChangeScanAndExe(void)
{
	PropFifoItem proptmp;

	if(fifo_gets(&propfifo, (unsigned char *)&proptmp, sizeof(PropFifoItem)) != 0)
	{
		LinkSrcExe(proptmp.UnitID, proptmp.PropID, proptmp.Type, proptmp.Value);
		return 1;
	}
	PropFifoInit();
	
	return 0;	
}
/*******************************************************************************
�� �� ����	PropEventFifo
����˵���� 	���Ա仯�¼�ѹ�����
��	  ���� 	unitID--��Ԫ��
            propID--���Ժ�
            type--���ͺ�
            value--ֵ
�� �� ֵ��	void
*******************************************************************************/
void PropEventFifo(unsigned char unitID, unsigned char propID, 
					unsigned char type, int value)
{
	if ((gUnitData[unitID].able)	//��Ԫʹ��
		|| ((199 < propID) && (propID < 221))	//��Ԫ�������Բ������� Unarty 2014.11.18 Add
		)
	{
		PropFifoItem proptmp;
		proptmp.UnitID = unitID;
		proptmp.PropID = propID;
		proptmp.Type   = type;
		proptmp.Value  = value;
		fifo_puts(&propfifo, (unsigned char *)&proptmp, sizeof(PropFifoItem));

		Thread_Login( RESULT, 0, 200, &PropChangeScanAndExe );
	}
	else
	{
		Upload(unitID);
	}
}

#endif
/**************************Copyright BestFu 2014-05-14*************************/
