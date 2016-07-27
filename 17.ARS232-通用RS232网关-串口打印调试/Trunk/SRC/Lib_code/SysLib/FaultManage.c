/***************************Copyright BestFu 2014-05-14*************************
��	����	FaultManage.c
˵	����	�쳣�������
��	�룺	Keil uVision4 V4.54.0.0
��	����	v1.0
��	д��	Unarty
��	�ڣ�	2014.06.30  
��  ��:     ����
*******************************************************************************/
#include "FaultManage.h"
#include "Thread.h"
#include "Instruct.h"
#include "UserData.h"
#include "Wdg.h"
#include "Upload.h"


/*******************************************************************
�� �� ����	Fault_Upload
����˵���� 	�쳣�ϱ�����
��	  ���� 	grade:	�쳣�ȼ�
			event:	�쳣�¼�
			fun:	������
�� �� ֵ��	��
*******************************************************************/
void Fault_Upload(FaultGrade_t grade, Fault_t event, void *fun)
{
	switch (grade)
	{
		case FAULT_0: 	//��߼������
						Upload_Fault(EEPROM_FAIL);
						Thread_Logout(&Instruct_Run);	//ע������ָʾ��
						break;	//��������
		case FAULT_1:	//ͨ�����ÿ��Իָ�
						WDG_FeedOFF();		//����ι�����豸�Զ���λ
						break;
		case FAULT_2:	//ͨ��ע�ắ�����ûָ�
						if (fun != NULL)
						{	
							Thread_Login(ONCEDELAY, 0, 5, fun);
						}
						break;
		default : 		break;
	}	
	
	if (event != EEPROM_W_ERR && event != EEPROM_R_ERR)
	{
		FaultData_Save(event);
	}
}

/**************************Copyright BestFu 2014-05-14*************************/
