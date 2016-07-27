/***************************Copyright BestFu 2014-05-14*************************
文 件：  RS232Attr.h
说 明：  RS232属性接口属性头文件
编 译：  Keil uVision4 V4.54.0.0
版 本：  v2.0
编 写：  Unarty
日 期：  2014.08.25
修　改：    暂无
*******************************************************************************/
#ifndef __RS232ATTR_H
#define __RS232ATTR_H

#include "BF_type.h"
#include "UnitCfg.h"


extern const AttrExe_st RS232AttrTable[];

void RS232ParaInit(u8 unitID);
void RS232EEPROMInit(u8 unitID);
void UnitInstruct(void);

MsgResult_t Set_RS232Cfg_Attr(UnitPara_t *pData);
MsgResult_t Get_RS232Cfg_Attr(UnitPara_t *pData, u8 *rlen, u8 *rpara);

MsgResult_t Set_RS232Data_Attr(UnitPara_t *pData);
MsgResult_t Get_RS232Data_Attr(UnitPara_t *pData, u8 *rlen, u8 *rpara);

MsgResult_t Set_RS232DataSend_Attr(UnitPara_t *pData);

MsgResult_t Get_Query_Status_Attr(UnitPara_t *pData, u8 *rlen, u8 *rpara);

#endif //RS232Attr.h end
/**************************Copyright BestFu 2014-05-14*************************/
