/***************************Copyright BestFu 2014-05-14*************************
��	����	Eeprom.c
˵	����	��Eeprom����,����ȫ�ֲ�����EEPROMӳ���ϵ��������дEEPROM���Դ����
��	�룺	Keil uVision4 V4.54.0.0
��	����	v1.0
��	д��	Joey
��	�ڣ�	2013.7.23   
��  ��:     Unart(2014.04.12���ӻ�ҳд��ʱ��æ���)
*******************************************************************************/
#include "EEPROM.h"
#include "I2C.h"
#include "FaultManage.h"

/*******************************************************************************
�� �� ����	ReadDataFromEEPROM
����˵����	�Ѷ�ӦEEPROM�����ݶ�ȡ��RAM��
��	  ����	addr-EEPROM�е�ʵ�ʵ�ַ
            len-����
            data-��ȡ�������ݴ���ڸ�ָ��
�� �� ֵ��	1��ʾ�ɹ���0��ʾʧ��
*******************************************************************************/ 
u8 ReadDataFromEEPROM(u32 addr, u32 len, u8 *data)
{
	u32 count;
	while (len)
	{
		if(addr >= EEPROMENDADDR || len > EEPROMSIZE)
		{
			return FALSE;
		}
			
		count = (len > (PAGESIZE-(addr%PAGESIZE))) ?
					(PAGESIZE - (addr%PAGESIZE)) : len;	
		if (!IIC_Read(EEPROMSLAVEADDR, addr, data, count))
		{
			break;
		}
		
		addr += count;
		len  -= count;
		data += count;
		if (!IIC_GetState(EEPROMSLAVEADDR, 0xfffffFFF)) //??EEPROM???? 
		{
			break;
		}
	}

	if (0 == len)
	{
		return TRUE;
	}
	else
	{
		Fault_Upload(FAULT_0, EEPROM_W_ERR, NULL);
		return FALSE;
	}	
}

/*******************************************************************************
�� �� ����	WriteDataToEEPROM
����˵����	��RAM�е�����д�뵽��Ӧ��EEPROM��
��	  ����	addr-EEPROM�е�ʵ�ʵ�ַ
            len-����
            data-��ȡ�������ݴ���ڸ�ָ��
�� �� ֵ��	1��ʾ�ɹ���0��ʾʧ��
*******************************************************************************/ 
u8 WriteDataToEEPROM(u32 addr, u32 len, u8 *data)
{
	u32 count;

	while (len)
	{
		if(addr >= EEPROMENDADDR || len > EEPROMSIZE)
		{
			return FALSE;
		}
		
		count = (len > (PAGESIZE-(addr%PAGESIZE))) ?
					(PAGESIZE - (addr%PAGESIZE)) : len;		
		if(!IIC_Write(EEPROMSLAVEADDR, addr, data, count))
		{
			break;
		}

		addr += count;
		len  -= count;
		data += count;
		if (!IIC_GetState(EEPROMSLAVEADDR, 0xfffffFF)) //??EEPROM???? 
		{
			break;
		}
	}

	if (0 == len)
	{
		return TRUE;
	}
	else
	{
		Fault_Upload(FAULT_0, EEPROM_W_ERR, NULL);
		return FALSE;
	}		
}

/*******************************************************************************
�� �� ����	WriteDataToEEPROMEx
����˵����	����ͬ����д�뵽��Ӧ��EEPROM��
��	  ����	addr-EEPROM�е�ʵ�ʵ�ַ
            len-����
            data-Ҫд�����ͬ����
�� �� ֵ��	1��ʾ�ɹ���0��ʾʧ��
*******************************************************************************/ 
u8 WriteDataToEEPROMEx(u32 addr, u32 len, u8 data)
{
	u32 count;

	while (len)
	{
		if(addr >= EEPROMENDADDR || len > EEPROMSIZE)
		{
			return FALSE;
		}
		
		count = (len > (PAGESIZE-(addr%PAGESIZE))) ?
					(PAGESIZE - (addr%PAGESIZE)) : len;		
		if(!IIC_WriteSame(EEPROMSLAVEADDR, addr, data, count))
		{
			break;
		}

		addr += count;
		len  -= count;
		if (!IIC_GetState(EEPROMSLAVEADDR, 0xfffffFF)) //??EEPROM???? 
		{
			break;
		}
	}
	if (0 == len)
	{
		return TRUE;
	}
	else
	{
		Fault_Upload(FAULT_0, EEPROM_W_ERR, NULL);
		return FALSE;
	}	
}

/**************************Copyright BestFu 2014-05-14*************************/
