/***************************Copyright BestFu 2014-05-14*************************
�� ����  UserData.c
˵ ����  �û����ݹ����ļ�
�� �룺  Keil uVision4 V4.54.0.0
�� ����  v1.0
�� д��  Unarty
�� �ڣ�  2014.06.24
�ޡ��ģ� 2014.11.11 Uarty �޸�UserData_Init�еĳ�ֵ�жϣ�������EEPROM�쳣�����û����ݶ�ʧ�Ŀ����ԡ�
		 2016.05.29 jay �ع���������
*******************************************************************************/
#include "EEPROM.h"
#include "Updata.h"
#include "SysHard.h"
#include "Upload.h"
#include "linkWRDC.h"
#include "FlashAttr.h"
#include "DevPartTab.h"
#include "TimeHandle.h"
#include "Scene&Group.h"

//Private macro define 
#define MACRO_SCENE_ERR									((u16)(0xffff))
#define CLR_DAT8_BIT(arr,pos)							((arr[(pos)/8])&=~(1<<((pos)%8)))
#define SET_DAT8_BIT(arr,pos)							((arr[(pos)/8])|= (1<<((pos)%8)))
#if (SCENCETIME_EN > 0u)
#define SET_SCENE_INDEX_VAL(unit,area,num,flag,timepos)	do{pSceneManage->SceneIndexes[(flag)].SceneInfo.scene.unit = (unit);\
															pSceneManage->SceneIndexes[(flag)].SceneInfo.scene.area = (area);\
															pSceneManage->SceneIndexes[(flag)].SceneInfo.scene.sceneNum = (num);\
															pSceneManage->SceneIndexes[(flag)].sceneTimeSavePos = (timepos);\
														}while(0)
#else
#define SET_SCENE_INDEX_VAL(unit,area,num,flag,timepos)	do{pSceneManage->SceneIndexes[(flag)].SceneInfo.scene.unit = (unit);\
															pSceneManage->SceneIndexes[(flag)].SceneInfo.scene.area = (area);\
															pSceneManage->SceneIndexes[(flag)].SceneInfo.scene.sceneNum = (num);\
														}while(0)														
#endif

//public global variable
SysData_t gSysData;

//Private variable define
static sceneManage_t sceneManage;
static sceneManage_t* pSceneManage = &sceneManage;

//Private function declaration
static void SceneEEPROMData_Save(u32 addr, u8 *data);
static u32 SceneEEPROMSave_Seat(const u8 len);
static u32 SceneEEPROMSave_Addr(u8 seat);
static u16 FindScene(const u8 unitNum, const u8 areaLevel, const u8 sceneNum);
static u8 DeleteScene(const u16 label);
static u16 SceneEEPROMData_Seat(const u8 len);
static void  SceneUseFlag_Set(const u8 label);
static u8 GetCurSceneSpace(const u8 label);

#if (SCENCETIME_EN > 0u)
static void SceneParamSeparate(u8* timedata, u8* orgdata);
static void  SceneTimeUseFlag_Clr(const u8 label);													
static void  SceneTimeUseFlag_Set(const u8 label);
static void SceneParamCombine(u8* timedata, u8* orgdata);
static u32 SceneTimeSave_Seat(const u8 len);
static u8 SceneEEPROMTimeData_Seat(const u8 len);
static u32 SceneTimeEEPROMSave_Addr(u8 seat);
static u8 GetCurSceneTimeSpace(const u8 label);
static void ClearSceneTimePara(const u8 label);

#endif

/*******************************************************************************
�� �� ����  UserData_Init()
����˵����  �û����ݳ�ʼ��
��   ����   ��
�� �� ֵ��  ��
��   �ģ�   Uarty 2014.11.11 ����Cnt��������EEPROM�쳣����������ɾ����
*******************************************************************************/
void UserData_Init(void)
{
    static u8 cnt = 0;  // Unarty Add 2014.11.11

    ReadDataFromEEPROM(SYSDATA_START_ADD, sizeof(gSysData), (u8 *)&gSysData);   //��ȡ����ʹ�ñ�ʶ
    if (HardID_Check(gSysData.deviceID) != 0)   								//�豸ID����ͬ
    {
        Thread_Login(ONCEDELAY, 0, 50, UserData_Init);      					//������֤����ֹEEPROM�ϵ�ʱ�쳣// Unarty Add 2014.11.11
        if (++cnt > 3)   // Unarty Add 2014.11.11
        {
            Thread_Logout(UserData_Init);       								//ȥ��������֤�ӿ�// Unarty Add 2014.11.11
            FaultData_Init();
            UserEEPROMData_Init(FristLevel);        							//��ʼ���û�EEPROM����
        }
    }
    else if (cnt > 0)
    {
        cnt = 0;            													//Unarty 2014.12.02 �޸ģ���ֹ������
        Unit_Init();
    }
}

/*******************************************************************************
�� �� ����  UserData_Init()
����˵����  �û����ݳ�ʼ��
��   ����   FristLevel ���ָ�433�ŵ�
            SecondLevel �ָ�433�ŵ�
�� �� ֵ��  ��
*******************************************************************************/
void UserEEPROMData_Init(DataInitLevel_t level)
{
    u8 tmp[] = {0, 0xF8, 1, 0x53};
	
    SysEEPROMData_Init();
    UnitPublicEEPROMData_Init();
    UnitPrivateEEPROMData_Init();
    GroupEEPROM_Init();
    SceneEEPROM_Init();
    DelAllLinkFromTable(SIMPLESTLINK);

    if (2 == level)
    {
        Set_433Channel_Attr((UnitPara_t *)tmp);//�ָ���������ʱ���ָ���83�ŵ�
    }

    Upload_Fault(USER_DATA_RESET);  			//�û���������
}

/*******************************************************************************
�� �� ����  FaultData_Init
����˵����  �쳣�¼��洢�ռ��ʼ��
��   ����   data:   �쳣���ش洢��ַ
�� �� ֵ��  ��
*******************************************************************************/
void FaultData_Init(void)
{
    WriteDataToEEPROMEx(FAULT_START_ADD, FAULT_SPACE, 0);
}

/*******************************************************************************
�� �� ����  FaultData_Save
����˵����  �쳣�¼��洢
��   ����   event�� �쳣�¼���
�� �� ֵ��  ��
*******************************************************************************/
void FaultData_Save(u8 event)
{
    u32 name;

    ReadDataFromEEPROM(FAULT_START_ADD + sizeof(name)*event,
                       sizeof(name), (u8 *)&name);     //��ȡ��ʶ
    name++;
    WriteDataToEEPROM(FAULT_START_ADD + sizeof(name)*event,
                      sizeof(name), (u8 *)&name);     //��ȡ��ʶ
}

/*******************************************************************************
�� �� ����  FaultData_Get
����˵����  �쳣�¼��洢
��   ����   data:   �쳣���ش洢��ַ
�� �� ֵ��  ��
*******************************************************************************/
void FaultData_Get(u8 *len, u8 *data)
{
    *len = FAULT_SPACE;
    ReadDataFromEEPROM(FAULT_START_ADD, *len, data);    //��ȡ��ʶ
}

/*******************************************************************************
�� �� ����  SysEEPROMData_Init()
����˵����  ϵͳEEPROM���ݳ�ʼ��
��   ����   ��
�� �� ֵ��  ��
*******************************************************************************/
void SysEEPROMData_Init(void)
{
    gSysData.userID     = DEFAULT_USERID;   //�û���
    gSysData.deviceID   = HardID_Check(0);      //
    gSysData.sendCnt    = 2; //Ĭ�����η����źţ������޸ģ�2015.7.2
    gSysData.serialNum  = 0;
    memset((u8 *)gSysData.BandAddr, 0, sizeof(gSysData.BandAddr)); //��󶨵�ַת�����ܣ���������
    gSysData.BandAreaEn = 0; //������㲥ת�����ܣ���������
    WriteDataToEEPROM(SYSDATA_START_ADD, sizeof(gSysData), (u8 *)&gSysData);
}

/*******************************************************************************
�� �� ����  SysEEPROMData_Init()
����˵����  ϵͳEEPROM���ݳ�ʼ��
��   ����   size:   �������ݴ�С
            data:   ������������
�� �� ֵ��  ��
*******************************************************************************/
u8 SysEEPROMData_Set(u8 size , u8 *data)
{
    return WriteDataToEEPROM(SYSDATA_START_ADD + ((u32)data - (u32)&gSysData), size, data);
}

/*******************************************************************************
�� �� ����  UnitPrivateEEPROMData_Init
����˵����  ��Ԫ˽��EEPROM���ݳ�ʼ��
��   ����   ��
�� �� ֵ��  ��
*******************************************************************************/
void UnitPrivateEEPROMData_Init(void)
{
    u8 unitCnt;

    for (unitCnt = 0; unitCnt < UnitCnt_Get(); unitCnt++)
    {
        UnitPrivateEEPROMUnitData_Init(unitCnt);
    }
}

/*******************************************************************************
�� �� ����  UnitPrivateEEPROMUnitData_Init
����˵����  ��Ԫ˽��EEPROM��Ԫ���ݳ�ʼ��
��   ����   unit�� Ŀ���ʽ����Ԫ��
�� �� ֵ��  ��
*******************************************************************************/
void UnitPrivateEEPROMUnitData_Init(u8 unit)
{
    if (UnitTab[unit].DefEE != NULL)
    {
        UnitTab[unit].DefEE(unit);
    }
}

/*******************************************************************************
�� �� ����  UnitPublicEEPROMData_Init
����˵����  ��Ԫ����EEPROM���ݳ�ʼ��
��   ����   ��
�� �� ֵ��  ��
*******************************************************************************/
void UnitPublicEEPROMData_Init(void)
{
    u8 unitCnt;

    for (unitCnt = 0; unitCnt < UnitCnt_Get(); unitCnt++)
    {
        UnitPublicEEPROMUnitData_Init(unitCnt);
    }
}

/*******************************************************************************
�� �� ����  UnitPublicEEPROMUnitData_Init
����˵����  ��Ԫ����EEPROM��Ԫ���ݳ�ʼ��
��   ����   unit�� Ŀ���ʽ����Ԫ��
�� �� ֵ��  ��
*******************************************************************************/
void UnitPublicEEPROMUnitData_Init(u8 unit)
{
    *(u32*)gUnitData[unit].area = DEFAULT_AREA; //Ĭ�������
    gUnitData[unit].type = 0;					//Ĭ���豸Ӧ������
    gUnitData[unit].able = 1;					//Ĭ��ʹ�ܱ�ʶ
    gUnitData[unit].common = 0;				//Ĭ�ϵ�ԪӦ�ô���
    gUnitData[unit].UploadAble = 1;   //Ĭ���ϱ�ʹ�ܱ��   yanhuan adding 2016/01/04
    gUnitData[unit].UploadSetTime = 3;//Ĭ���ϱ��ϱ�ʹ��������ʱ��
    WriteDataToEEPROM(UNITDATA_START_ADD + ONCEUNITDATA_SPACE*unit,
                      sizeof(UnitData_t), (u8*)&gUnitData[unit]);
}

/*******************************************************************************
�� �� ����  UnitPublicEEPROMData_Get
����˵����  ��ȡ��Ԫ����EEPROM����
��   ����   unitCnt:    ��ȡ��Ԫ����
            unitData:   ��Ԫ��������
�� �� ֵ��  ��
*******************************************************************************/
void UnitPublicEEPROMData_Get(u8 unitCnt, UnitData_t *unitData)
{
    while (unitCnt--)
    {
        ReadDataFromEEPROM(UNITDATA_START_ADD + (ONCEUNITDATA_SPACE * (unitCnt)), sizeof(UnitData_t), (u8 *)&unitData[unitCnt]);
    }
}

/*******************************************************************************
�� �� ����  UnitPublicEEPROMData_Set
����˵����  ���õ�Ԫ����EEPROM���ݳ�ʼ��
��   ����   size:   �������ݴ�С
            data:   ������������
�� �� ֵ��  ��
*******************************************************************************/
u8 UnitPublicEEPROMData_Set(u8 size , u8 *data)
{
    u32 seat = (u32)data - (u32)&gUnitData;

    return WriteDataToEEPROM(UNITDATA_START_ADD + (ONCEUNITDATA_SPACE * (seat / sizeof(UnitData_t))) + seat % sizeof(UnitData_t), size, data);
}

/*******************************************************************************
�� �� ����  u8 SetDeviceChannelFlag(void)
����˵����  �����豸������Ϣ
��   ����   ������Ϣ
�� �� ֵ��  ��
*******************************************************************************/
u8 SetDeviceChannelFlag(const u32 deviceUseNum, const u8 operateLevel)
{
	u8 i = 0;
	const u8 validNum = (offsetof(Hard_t,data) - offsetof(Hard_t,channelFlag[0]))/sizeof(u32);
	u32 data_tmp[validNum];
	DevPivotalSet_t* pDevPivotalSet = GetDevPivotalSet();
	Hard_t* pHard = (Hard_t*)(pDevPivotalSet->DevHardInfoAddr);

	data_tmp[0] = deviceUseNum;
	if(!operateLevel)	
	{
		for(; (i < validNum)&&(pHard->channelFlag[i] != Flash_DefaultValue()); i++);
		if(validNum > i)
		{
			return Program_Write((u32)pHard + offsetof(Hard_t,channelFlag[i]), 4, (u8*)&data_tmp);
		}
		return (u8)FLASH_SPACE_FULL_ERR;
	}
	else
	{
		memset(&data_tmp[1], Flash_DefaultValue(), sizeof(data_tmp) - 4);
		return Program_WriteEx((u32)pHard + offsetof(Hard_t,channelFlag), sizeof(data_tmp), (u8*)data_tmp);
	}
}

/*******************************************************************************
�� �� ����  u32 GetDeviceChannelFlag(void)
����˵����  ��ȡ�豸������Ϣ
��   ����   ��
�� �� ֵ��  ������Ϣ
*******************************************************************************/
u32 GetDeviceChannelFlag(void)
{
	u8 i = 0;
	DevPivotalSet_t* pDevPivotalSet = GetDevPivotalSet();
	Hard_t* pHard = (Hard_t*)(pDevPivotalSet->DevHardInfoAddr);
	const u8 validNum = (offsetof(Hard_t,data) - offsetof(Hard_t,channelFlag[0]))/sizeof(u32);
	
	for(; (i < validNum)&&(pHard->channelFlag[i] != Flash_DefaultValue()); i++);
	if((i)&&(pHard->channelFlag[i-1])&&(0xFFFFFFFF != pHard->channelFlag[i-1]))
	{
		return pHard->channelFlag[i-1];
	}
	return 1;		//Ĭ���ǻۿռ�
}

/*******************************************************************************
�� �� ����  GroupEEPROMData_Init
����˵����  ��EEPROM���ݳ�ʼ��
��   ����   ��
�� �� ֵ��  ��
*******************************************************************************/
void GroupEEPROM_Init(void)
{
    u8  unitCnt;
    for (unitCnt = 1; unitCnt < UnitCnt_Get(); unitCnt++)
    {
        GroupEEPROMUnit_Init(unitCnt);
    }
}

/*******************************************************************************
�� �� ����  GroupEEPROMUnitData_Init
����˵����  ��EEPROM��Ԫ���ݳ�ʼ��
��   ����   unit�� Ŀ���ʽ����Ԫ��
�� �� ֵ��  ��
*******************************************************************************/
void GroupEEPROMUnit_Init(const u8 unit)
{
    WriteDataToEEPROMEx(GROUP_START_ADD + (UNIT_GROUP_SPACE * unit), UNIT_GROUP_SPACE, 0);
}

/*******************************************************************************
�� �� ����  GroupEEPROM_Clr
����˵����  �����ű�ʶλ
��   ����   unit:  ��Ԫλ��
            area�� ����λ��(0~4)
            num:   ���
�� �� ֵ��  ��
*******************************************************************************/
void GroupEEPROM_Clr(const u8 unit, const  u8 area, const  u8 num)
{
    u8 groupAddr = 0;
	
    ReadDataFromEEPROM(GROUP_START_ADD + (UNIT_GROUP_SPACE * unit) + (AREA_GROUP_SPACE * area) + (num/8), 1, &groupAddr);
    if ((num < AREA_GROUP_NUM) && ((groupAddr & (1 << (num%8)))))   // �����ȷ, ���ұ�����
    {
        groupAddr &=  ~(1 << (num%8));
        WriteDataToEEPROM(GROUP_START_ADD + (UNIT_GROUP_SPACE * unit) + (AREA_GROUP_SPACE * area) + (num/8), 1, &groupAddr);
    }
    else if (CLEAR == num)     //�����������
    {
		WriteDataToEEPROMEx(GROUP_START_ADD + (UNIT_GROUP_SPACE * unit) + (AREA_GROUP_SPACE * area), AREA_GROUP_SPACE, 0);
    }
}

/*******************************************************************************
�� �� ����  GroupEEPROMData_Set
����˵����  ������ű�ʶ�Ƿ���Ч
��   ����   unit:  ��Ԫλ��
            area�� ����λ��(0~4)
            num:   ���
�� �� ֵ��  ��
*******************************************************************************/
void GroupEEPROM_Set(const u8 unit,const u8 area,const u8 num)
{
    u8 groupAddr = 0;

    ReadDataFromEEPROM(GROUP_START_ADD + (UNIT_GROUP_SPACE * unit) + (AREA_GROUP_SPACE * area) + (num/8), 1, &groupAddr);
    if ((num < AREA_GROUP_NUM)&&(!(groupAddr & (1 << (num%8)))))   // �����ȷ, ���һ�δ����
    {
        groupAddr |=  1 << (num%8);
        WriteDataToEEPROM(GROUP_START_ADD + (UNIT_GROUP_SPACE * unit) + (AREA_GROUP_SPACE * area) + (num/8), 1, &groupAddr);
    }
}

/*******************************************************************************
�� �� ����  GroupEEPROMData_Get
����˵����  ��ȡ��ű�ʶ�Ƿ���Ч
��   ����   unit:  ��Ԫλ��
            area�� ����λ��(0~4)
            num:   ���
�� �� ֵ��  ��ʶ��� TRUE(��Ч)/FALSE(��Ч)
*******************************************************************************/
u8 GroupEEPROM_Get(const u8 unit,const u8 area,const u8 num)
{
    u8 groupAddr = 0;

    ReadDataFromEEPROM(GROUP_START_ADD + (UNIT_GROUP_SPACE * unit) + (AREA_GROUP_SPACE * area) + (num/8), 1, &groupAddr);
    if (groupAddr & (1 << (num%8)))
    {
        return TRUE;
    }
    return FALSE;
}

/*******************************************************************************
�� �� ����  SceneData_Init
����˵����  �������ݳ�ʼ��
��   ����   ��
�� �� ֵ��  ��
*******************************************************************************/
void SceneData_Init(void)
{	
	ReadDataFromEEPROM(SCENE_INDEX_START_ADD, sizeof(SceneIndexes_t)*SCENE_MAX_NUM, (u8*)&pSceneManage->SceneIndexes);
	ReadDataFromEEPROM(SCENE_USE_FLAG_ADD, SCENE_USE_SPACE + SCENE_TIME_USE_SPACE, (u8*)&pSceneManage->sceneUseFlag);
}

/*******************************************************************************
�� �� ����  SceneEEPROM_Init
����˵����  ����EEPROM���ݳ�ʼ��
��   ����   ��
�� �� ֵ��  ��
*******************************************************************************/
void SceneEEPROM_Init(void)
{
	memset((u8*)pSceneManage, 0, sizeof(sceneManage_t));
	WriteDataToEEPROMEx(SCENE_INDEX_START_ADD, sizeof(SceneIndexes_t)*SCENE_MAX_NUM, SCENE_DEFAULT_VALUE);
	WriteDataToEEPROMEx(SCENE_USE_FLAG_ADD, SCENE_USE_SPACE + SCENE_TIME_USE_SPACE, SCENE_DEFAULT_VALUE);
}

/*******************************************************************************
�� �� ����  SceneEEPROMUnitData_Init
����˵����  ����EEPROM��Ԫ���ݳ�ʼ��
��   ����   unit�� Ŀ���ʽ����Ԫ��
�� �� ֵ��  ��
*******************************************************************************/
void SceneEEPROMUnit_Init(u8 unit)
{
	u16 i = 0;
#if (SCENCETIME_EN > 0u)
	u8 timeLabel;
#endif
	
	if((!unit)&&(unit > UnitCnt_Get()))
	{
		return ;
	}
	for(; i < SCENE_MAX_NUM; i++)
	{
		if(unit == pSceneManage->SceneIndexes[i].SceneInfo.scene.unit)
		{		
#if (SCENCETIME_EN > 0u)
			timeLabel = pSceneManage->SceneIndexes[i].sceneTimeSavePos;
			if((SCENE_DEFAULT_VALUE != timeLabel)&&(timeLabel <= SCENE_TIME_TOTAL_NUM))
			{
				CLR_DAT8_BIT(pSceneManage->sceneTimeUseFlag, timeLabel - 1);
			}
#endif			
			CLR_DAT8_BIT(pSceneManage->sceneUseFlag, i);
			memset(&pSceneManage->SceneIndexes[i], 0, sizeof(SceneIndexes_t));
			WriteDataToEEPROMEx(SCENE_INDEX_START_ADD + sizeof(SceneIndexes_t)*i, sizeof(SceneIndexes_t), 0);	
		}
	}
	WriteDataToEEPROM(SCENE_USE_FLAG_ADD, SCENE_USE_SPACE + SCENE_TIME_USE_SPACE, (u8*)&pSceneManage->sceneUseFlag);
}


/*******************************************************************************
�� �� ��:  	FindScene
����˵��:  	����һ������
��    ��:  	unit ��Ԫ��
			area �����
			sceneNum ������						
�� �� ֵ:  	sceneAddr �����Ĵ洢��ַ
			MACRO_SCENE_ERR������ʧ��
*******************************************************************************/
static u16 FindScene(const u8 unitNum, const u8 areaLevel, const u8 sceneNum)
{
	u16 i;
	for(i = 0; i < SCENE_MAX_NUM; i++)
	{
		if((unitNum == pSceneManage->SceneIndexes[i].SceneInfo.scene.unit)&&\
			(areaLevel == pSceneManage->SceneIndexes[i].SceneInfo.scene.area)&&\
			(sceneNum == pSceneManage->SceneIndexes[i].SceneInfo.scene.sceneNum))
		{
			return (i);
		}
	}
	return (MACRO_SCENE_ERR);	
}

/*******************************************************************************
�� �� ��:  	DeleteScene
����˵��:  	ɾ��һ������
��    ��:  	label:Ҫɾȥ�����ı��
�� �� ֵ:   Ҫɾ�������Ĵ洢λ��
*******************************************************************************/
static u8 DeleteScene(const u16 label)
{		
	CLR_DAT8_BIT(pSceneManage->sceneUseFlag, label);
	WriteDataToEEPROM(SCENE_USE_FLAG_ADD + label/8, 1, (u8*)&pSceneManage->sceneUseFlag[label/8]);
	memset(&pSceneManage->SceneIndexes[label], 0, sizeof(SceneIndexes_t));
	return WriteDataToEEPROMEx(SCENE_INDEX_START_ADD + sizeof(SceneIndexes_t)*label, sizeof(SceneIndexes_t), 0);
}

/*******************************************************************************
�� �� ����  static void  SceneUseFlag_Set(const u8 label)
����˵����  ���ó����ڴ�ʹ�ñ��
��   ����   label: �������ݱ��(0x00~0xff)
�� �� ֵ��  ��
*******************************************************************************/
static void  SceneUseFlag_Set(const u8 label)
{
	SET_DAT8_BIT(pSceneManage->sceneUseFlag, label);
	WriteDataToEEPROM(SCENE_USE_FLAG_ADD + (label)/8, 1, (u8*)&pSceneManage->sceneUseFlag[(label)/8]);
}

#if (SCENCETIME_EN > 0u)
/*******************************************************************************
�� �� ����  static void  SceneTimeUseFlag_Clr(const u8 label)
����˵����  �������ʱ���ڴ�ʹ�ñ��
��   ����   label: �������ݱ��(0x01~56)
�� �� ֵ��  ��
*******************************************************************************/
static void  SceneTimeUseFlag_Clr(const u8 label)
{
	CLR_DAT8_BIT(pSceneManage->sceneTimeUseFlag, label - 1);
	WriteDataToEEPROM(SCENE_TIME_USE_FLAG_ADD + (label - 1)/8, 1, (u8*)&pSceneManage->sceneTimeUseFlag[(label - 1)/8]);
}

/*******************************************************************************
�� �� ����  static void  SceneTimeUseFlag_Set(const u8 label)
����˵����  ���ó���ʱ���ڴ�ʹ�ñ��
��   ����   label: �������ݱ��(0x01~56)
�� �� ֵ��  ��
*******************************************************************************/
static void  SceneTimeUseFlag_Set(const u8 label)
{
	SET_DAT8_BIT(pSceneManage->sceneTimeUseFlag, label - 1);
	WriteDataToEEPROM(SCENE_TIME_USE_FLAG_ADD + (label - 1)/8, 1, (u8*)&pSceneManage->sceneTimeUseFlag[(label - 1)/8]);
}

/*******************************************************************************
�� �� ����  inline u32 CheckTime_PositionRange(u8 position)
����˵����  ��鳡������ʼ�洢λ��
��   ����   position:����鳡������ʼ�洢λ�õı���
�� �� ֵ��  ���س�������ʼ�洢λ�õ�ֵ
*******************************************************************************/
__inline u32 CheckTime_PositionRange(u8 position)
{
    if (position <= SCENE_TIME_SPACE1_NUM)
    {
        return 0;
    }
    else if (position <= SCENE_TIME_SPACE1_NUM + SCENE_TIME_SPACE2_NUM)
    {
        return (SCENE_TIME_SPACE1_NUM);
    }
    else if (position <= SCENE_TIME_TOTAL_NUM)
    {
        return (SCENE_TIME_SPACE1_NUM + SCENE_TIME_SPACE2_NUM);
    }
	else
	{
		return SCENE_TIME_TOTAL_NUM;
	}
}

/*******************************************************************************
�� �� ����  SceneTimeSave_Seat
����˵����  ����EEPROMTime�洢λ��
��   ����   seat:   �洢����
�� �� ֵ��  �洢λ��
*******************************************************************************/
static u32 SceneTimeSave_Seat(const u8 len)
{
    if (len <= SCENE_TIME_DATA_SPACE1)    //���ȴ��ڵ�һ�洢�ռ�
    {
        return  0;
    }
    else if (len <= (SCENE_TIME_DATA_SPACE2))     //���ȴ��ڵڶ��洢�ռ�
    {
        return SCENE_TIME_SPACE1_NUM;
    }
    else if (len <= (SCENE_TIME_DATA_SPACE3))     //���ȴ��ڵ����洢�ռ�
    {
        return (SCENE_TIME_SPACE1_NUM + SCENE_TIME_SPACE2_NUM);
    }

    return SCENE_TIME_TOTAL_NUM;   //����Խ��
}

/*******************************************************************************
�� �� ����  SceneEEPROMTimeData_Seat
����˵����  ����һ������ʱ��洢λ��
��   ����   len:    Ŀ��洢���ݳ���
�� �� ֵ��  number: �洢��λ��ֵ,MACRO_SCENE_ERR:��ʾû�����뵽
*******************************************************************************/
static u8 SceneEEPROMTimeData_Seat(const u8 len)
{
    u16 i;
	u16 xReturn = SCENE_DEFAULT_VALUE;

	for(i = SceneTimeSave_Seat(len); i < SCENE_TIME_TOTAL_NUM; i++)
	{
		if (!(pSceneManage->sceneTimeUseFlag[i / 8] & (1 << (i % 8))))   //����洢λ���ǿ��е�
		{
			xReturn = i + 1;
			break;
		}
	}
	return (xReturn);
}

/*******************************************************************************
�� �� ����  SceneTimeEEPROMSave_Addr
����˵����  ����ʱ��EEPROM�洢��ַ
��   ����   seat:   �洢���
�� �� ֵ��  �洢��ַ
*******************************************************************************/
static u32 SceneTimeEEPROMSave_Addr(u8 seat)
{
    u32 addr = SCENE_TIME_DATA_START_ADD;
	
	if(seat <= SCENE_TIME_SPACE1_NUM)
	{
		addr += (seat-1) * SCENE_TIME_DATA_SPACE1;
        goto RETURN;		//λ�ô��ڴ洢�ռ�1
	}

    addr += SCENE_TIME_SPACE1_NUM * SCENE_DATA_SPACE1;
    seat -= SCENE_TIME_SPACE1_NUM;
    if (seat <= SCENE_SPACE2_NUM)
    {
        addr += (seat - 1) * SCENE_TIME_DATA_SPACE2;
        goto RETURN;//λ�ô��ڴ洢�ռ�2
    }

    addr += SCENE_TIME_SPACE2_NUM * SCENE_TIME_DATA_SPACE2;
    seat -= SCENE_TIME_SPACE2_NUM;
    if (seat <= SCENE_TIME_SPACE3_NUM)
    {
        addr += (seat - 1) * SCENE_TIME_DATA_SPACE3;
        goto RETURN;//λ�ô��ڴ洢�ռ�3
    }

RETURN:
    return addr;
}
#endif

/*******************************************************************************
�� �� ����  SceneEEPROM_Clr
����˵����  �������
��   ����   unit:  ��Ԫλ��
            area�� ����λ��(0~4)
            num:   ������
�� �� ֵ��  ��
*******************************************************************************/
void SceneEEPROM_Clr(const u8 unit, const u8 area, const u16 num)
{
    u16 label = 0;							//�����洢��ı��
	u16 i;
#if (SCENCETIME_EN > 0u)
	u8 timeLabel;
#endif

    if ((num < AREA_SCENE_NUM)&&(MACRO_SCENE_ERR != (label = FindScene(unit,area,num))))  //�����洢��ı�źϷ�,ע��FindScene�ķ���ֵ,(MACRO_SCENE_ERR)��ʾ������
    {
#if (SCENCETIME_EN > 0u)
		timeLabel = pSceneManage->SceneIndexes[label].sceneTimeSavePos;
		if((SCENE_DEFAULT_VALUE != timeLabel)&&(timeLabel <= SCENE_TIME_TOTAL_NUM)&&\
			(pSceneManage->sceneTimeUseFlag[(timeLabel - 1)/8]&(1<<((timeLabel - 1)%8))))
		{
			SceneTimeUseFlag_Clr(timeLabel);	
		}
#endif
		DeleteScene(label);
    }
	else if(CLEAR == num)     //�����������
	{
		for(i = 0; i < SCENE_MAX_NUM; i++)
		{
			if((unit == pSceneManage->SceneIndexes[i].SceneInfo.scene.unit)&&\
				(area == pSceneManage->SceneIndexes[i].SceneInfo.scene.area))
			{
#if (SCENCETIME_EN > 0u)
				timeLabel = pSceneManage->SceneIndexes[i].sceneTimeSavePos;
				if((SCENE_DEFAULT_VALUE != timeLabel)&&(timeLabel <= SCENE_TIME_TOTAL_NUM))
				{
					CLR_DAT8_BIT(pSceneManage->sceneTimeUseFlag, timeLabel - 1);
				}
#endif
				CLR_DAT8_BIT(pSceneManage->sceneUseFlag, i);
				memset((u8*)&pSceneManage->SceneIndexes[i], 0, sizeof(SceneIndexes_t));
				WriteDataToEEPROMEx(SCENE_INDEX_START_ADD + sizeof(SceneIndexes_t)*i, sizeof(SceneIndexes_t), 0);
			}
		}
		WriteDataToEEPROM(SCENE_USE_FLAG_ADD, SCENE_USE_SPACE + SCENE_TIME_USE_SPACE, (u8*)&pSceneManage->sceneUseFlag);	
	}
}

/*******************************************************************************
�� �� ����  inline u32 Check_PositionRange(u8 position)
����˵����  ��鳡������ʼ�洢λ��
��   ����   position:����鳡������ʼ�洢λ�õı���
�� �� ֵ��  ���س�������ʼ�洢λ�õ�ֵ
*******************************************************************************/
__inline u32 Check_PositionRange(u8 position)
{
    if (position < SCENE_SPACE1_NUM)
    {
        return 0;
    }
    else if (position < SCENE_SPACE1_NUM + SCENE_SPACE2_NUM)
    {
        return (SCENE_SPACE1_NUM);
    }
    else
    {
        return (SCENE_SPACE1_NUM + SCENE_SPACE2_NUM);
    }
}

/*******************************************************************************
�� �� ����  SceneEEPROMSave_Seat
����˵����  ����EEPROM�洢λ��
��   ����   seat:   �洢����
�� �� ֵ��  �洢λ��
*******************************************************************************/
u32 SceneEEPROMSave_Seat(const u8 len)
{
    if (len < SCENE_DATA_SPACE1)    //���ȴ��ڵ�һ�洢�ռ�
    {
        return  0;
    }
    else if (len < (SCENE_DATA_SPACE2))     //���ȴ��ڵڶ��洢�ռ�
    {
        return SCENE_SPACE1_NUM;
    }
    else if (len < (SCENE_DATA_SPACE3))     //���ȴ��ڵ����洢�ռ�
    {
        return (SCENE_SPACE1_NUM + SCENE_SPACE2_NUM);
    }

    return SCENE_MAX_NUM;   //����Խ��
}

/*******************************************************************************
�� �� ����  SceneEEPROM_Set
����˵����  д��һ������
��   ����   unit:       ��Ԫλ��
            area��      ����λ��(0~4)
            num:        �龰��
            data[0]:    �������ݳ���
            data[1~n]:  ��������
�� �� ֵ��  �洢���
*******************************************************************************/
u32 SceneEEPROM_Set(u8 unit, u8 area, u8 num, u8 *data)
{
	u16 label = FindScene(unit,area,num);

	if(label != MACRO_SCENE_ERR)
	{
#if (SCENCETIME_EN > 0u)
		ClearSceneTimePara(label);
#endif
		if ((!(pSceneManage->sceneUseFlag[label/8]&(1<<(label%8))))||\
			(Check_PositionRange(label) != SceneEEPROMSave_Seat(data[0])))     //��ͬ�����ŵ������,����Ҫ�Ĵ洢�ռ��ڴ�鲻ͬ,��Ҫ��������
		
		{
			DeleteScene(label);				//���֮ǰ�洢��ʶ,��ɾ��ʱ�����
			label = MACRO_SCENE_ERR;
		}
	}
	
    if(MACRO_SCENE_ERR == label)   	//δ�ҵ��ó�����������û������ռ�
    {
		if(MACRO_SCENE_ERR == (label = (SceneEEPROMData_Seat(data[0]))))//����һ���洢�ռ�
        {
            return EEPROM_RAND_ERR;
        }

		SceneUseFlag_Set(label);
        SET_SCENE_INDEX_VAL(unit,area,num,label,SCENE_DEFAULT_VALUE);
        WriteDataToEEPROM(SCENE_INDEX_START_ADD + sizeof(SceneIndexes_t)*label, sizeof(SceneInfo_t), (u8*)&pSceneManage->SceneIndexes[label]);
    }
	
	SceneEEPROMData_Save(SceneEEPROMSave_Addr(label), data);
    return COMPLETE;
}

#if (SCENCETIME_EN > 0u)
/*******************************************************************************
�� �� ����  u32 SceneEEPROM_SetEx(u8 unit, u8 area, u8 num, u8 *data)
����˵����  д��һ����������չ�ӿ�,֧����ʱ˳��ִ��
��   ����   unit:       ��Ԫλ��
            area��      ����λ��(0~4)
            num:        �龰��
            cnt:        ������������
            data[0~n]:  ����ʱ����������
�� �� ֵ��  �洢���
*******************************************************************************/
u32 SceneEEPROM_SetEx(u8 unit, u8 area, u8 num, u8 *data)
{
	u8 	data_tmp[SCENE_TIME_DATA_SPACE3 + 1];
	u16 label = FindScene(unit,area,num);
	u8 timePos = SCENE_DEFAULT_VALUE;
	u8 status = 0;
	
	SceneParamSeparate((u8*)data_tmp, (u8*)data);		//��������
	if(TRUE == CheckAllTimeParaIsOK((DlyTimeData_t*)&data_tmp[1], data_tmp[0]/sizeof(DlyTimeData_t)))
	{
		if(label != MACRO_SCENE_ERR)
		{
			if(SCENE_DEFAULT_VALUE != (timePos = pSceneManage->SceneIndexes[label].sceneTimeSavePos))
			{
				if((timePos > SCENE_TIME_TOTAL_NUM)||\
					(!(pSceneManage->sceneTimeUseFlag[(timePos - 1)/8]&(1<<((timePos - 1)%8)))))
				{
					timePos = SCENE_DEFAULT_VALUE;
				}
				else if(CheckTime_PositionRange(timePos) != SceneTimeSave_Seat(data_tmp[0]))
				{
					SceneTimeUseFlag_Clr(timePos);
					timePos = SCENE_DEFAULT_VALUE;
				}
			}
			if((!(pSceneManage->sceneUseFlag[label/8]&(1<<(label%8))))||\
				(Check_PositionRange(label) != SceneEEPROMSave_Seat(data[0])))
			{
				DeleteScene(label);				//���֮ǰ�洢��ʶ,��ɾ��ʱ�����
				label = MACRO_SCENE_ERR;
			}
		}
			
		if(SCENE_DEFAULT_VALUE == timePos)
		{
			if(SCENE_DEFAULT_VALUE == (timePos = (SceneEEPROMTimeData_Seat(data_tmp[0]))))//����һ���洢�ռ�
			{
				return EEPROM_RAND_ERR;
			}
			status |= (1 << 0);
		}
		
		if(MACRO_SCENE_ERR == label)   	//δ�ҵ��ó�����������û������ռ�
		{
			if(MACRO_SCENE_ERR == (label = (SceneEEPROMData_Seat(data[0]))))//����һ���洢�ռ�
			{
				return EEPROM_RAND_ERR;
			}
			status |= (1 << 1); 
		}
		
		switch(status)
		{
		case 3:
		case 2:
			SceneUseFlag_Set(label);
		case 1:
			if((1 == status)||(3 == status))
			{
				SceneTimeUseFlag_Set(timePos);
			}
			SET_SCENE_INDEX_VAL(unit,area,num,label,timePos);
			WriteDataToEEPROM(SCENE_INDEX_START_ADD + sizeof(SceneIndexes_t)*label, sizeof(SceneIndexes_t), (u8*)&pSceneManage->SceneIndexes[label]);
		break;
		default:
			break;		
		}
		WriteDataToEEPROM(SceneTimeEEPROMSave_Addr(timePos), data_tmp[0], &data_tmp[1]); 
		SceneEEPROMData_Save(SceneEEPROMSave_Addr(label), data);	
		return COMPLETE;
	}
	return TIME_PARA_ERR;	
}

/*******************************************************************************
�� �� ����  SceneEEPROM_GetEx
����˵����  ��ȡ�����ű�ʶ�Ƿ���Ч
��   ����   unit:       ��Ԫλ��
            area��     ����λ��(0~4)
            num:        ���
            data[0]:    �������ݳ��� data[0] = 0, û�д˳���
            data[1~n]:  ��������
�� �� ֵ��  ��ʶ��� TRUE(��Ч)/FALSE(��Ч)
*******************************************************************************/
u8 SceneEEPROM_GetEx(u8 unit, u8 area, u8 num, u8 *data)
{
    u8 	data_tmp[SCENE_TIME_DATA_SPACE3];
	u16 label = FindScene(unit,area,num);
	u8 sceneTimeLabel;
	
	if(MACRO_SCENE_ERR != label)
	{
		ReadDataFromEEPROM(SceneEEPROMSave_Addr(label),GetCurSceneSpace(label), data);  //��������   
		if((SCENE_DEFAULT_VALUE != (sceneTimeLabel = pSceneManage->SceneIndexes[label].sceneTimeSavePos))&&\
			(sceneTimeLabel <= SCENE_TIME_TOTAL_NUM))
		{
			ReadDataFromEEPROM(SceneTimeEEPROMSave_Addr(sceneTimeLabel), GetCurSceneTimeSpace(sceneTimeLabel), data_tmp); //��ȡʱ������
			SceneParamCombine(data_tmp, data);
			return TRUE;
		}
	}
    else
    {
        *data = 0;  //���س�������Ϊ��
    }
    return FALSE;
}

/*******************************************************************************
�� �� ����  static void SceneParamSeparate(u8 *timedata, u8 *orgdata)
����˵����  �������ݷ��뺯��
��   ����   timedata��ʱ�����ݴ洢ָ��,����timedata[0]��Ŷ���䳤��
			orgdata��ʱ�����ݴ洢ָ��,orgdata[0]��ʾ���䳤��
�� �� ֵ��  ��
*******************************************************************************/
static void SceneParamSeparate(u8* timedata, u8* orgdata)
{
	const u8 len = orgdata[0];
	u8  pos = 0,pos1 = 0;
	u8* orgaddr;
	
	while( len > pos + pos1)
	{
		orgaddr = &orgdata[pos + 1] + ((UnitPara_t *)&orgdata[pos + 1])->len + offsetof(UnitPara_t,data);
		memcpy(timedata + pos1 + 1, orgaddr, sizeof(DlyTimeData_t));
		pos += ((UnitPara_t *)&orgdata[pos + 1])->len + offsetof(UnitPara_t,data);
		pos1 += sizeof(DlyTimeData_t);
		memmove(orgaddr, orgaddr + sizeof(DlyTimeData_t), len - pos - pos1);
	}
	timedata[0] = pos1;
	orgdata[0] = pos;
}

/*******************************************************************************
�� �� ����  static void SceneParamCombine(u8 *timedata, u8 *orgdata)
����˵����  �������ݺϲ�����
��   ����   timedata��ʱ�����ݴ洢ָ��
			orgdata��ʱ�����ݴ洢ָ��,orgdata[0]��ʾ����
�� �� ֵ��  ��
*******************************************************************************/
static void SceneParamCombine(u8* timedata, u8* orgdata)
{
	const u8 len = orgdata[0];
	u8  pos = 0,pos1 = 0;
	u8* orgaddr;
	
	while( len > pos)
	{
		orgaddr = &orgdata[pos + pos1 + 1] + ((UnitPara_t *)&orgdata[pos + pos1 + 1])->len + offsetof(UnitPara_t,data);
		pos += ((UnitPara_t *)&orgdata[pos + pos1 + 1])->len + offsetof(UnitPara_t,data);
		memmove(orgaddr + sizeof(DlyTimeData_t), orgaddr, len - pos);
		memcpy(orgaddr, timedata + pos1, sizeof(DlyTimeData_t));	
		pos1 += sizeof(DlyTimeData_t);
	}
	orgdata[0] = pos + pos1;
}

/*******************************************************************************
�� �� ����  GetCurSceneTimeSpace
����˵����  ��ȡ����ʱ���Ŷ�Ӧ�Ĵ洢��С
��   ����   label:����ʱ����
�� �� ֵ��  �ڴ��Ĵ�С
*******************************************************************************/
static u8 GetCurSceneTimeSpace(const u8 label)
{
	if(label <= SCENE_TIME_SPACE1_NUM)
	{
		return SCENE_TIME_DATA_SPACE1;
	}
	else if(label <= SCENE_TIME_SPACE1_NUM + SCENE_TIME_SPACE2_NUM)
	{
		return SCENE_TIME_DATA_SPACE2;
	}
	else
	{
		return SCENE_TIME_DATA_SPACE3;
	}
}


/*******************************************************************************
�� �� ����  ClearSceneTimePara
����˵����  ��������е�ʱ�����
��   ����   label:�������
�� �� ֵ��  ��SceneEEPROM_Set����ʹ��
*******************************************************************************/
static void ClearSceneTimePara(const u8 label)
{
	u8* pTimeLabel = &pSceneManage->SceneIndexes[label].sceneTimeSavePos;
	
	if(SCENE_DEFAULT_VALUE != *pTimeLabel)
	{
		if((*pTimeLabel <= SCENE_TIME_TOTAL_NUM)&&\
			(pSceneManage->sceneTimeUseFlag[(*pTimeLabel - 1)/8]&(1<<((*pTimeLabel - 1)%8))))
		{
			SceneTimeUseFlag_Clr(*pTimeLabel);	
		}
		*pTimeLabel = SCENE_DEFAULT_VALUE;
		WriteDataToEEPROMEx(SCENE_INDEX_START_ADD + sizeof(SceneIndexes_t)*label + offsetof(SceneIndexes_t, sceneTimeSavePos),\
			sizeof(SceneIndexes_t) - sizeof(SceneInfo_t), SCENE_DEFAULT_VALUE);
	}
}

#endif

/*******************************************************************************
�� �� ����  GetCurSceneSpace
����˵����  ��ȡ������Ŷ�Ӧ�Ĵ洢��С
��   ����   label:����ʱ����
�� �� ֵ��  �ڴ��Ĵ�С
*******************************************************************************/
static u8 GetCurSceneSpace(const u8 label)
{
	if(label < SCENE_SPACE1_NUM)
	{
		return SCENE_DATA_SPACE1;
	}
	else if(label < SCENE_SPACE1_NUM + SCENE_SPACE2_NUM)
	{
		return SCENE_DATA_SPACE2;
	}
	else
	{
		return SCENE_DATA_SPACE3;
	}
}

/*******************************************************************************
�� �� ����  SceneEEPROM_Get
����˵����  ��ȡ�����ű�ʶ�Ƿ���Ч
��   ����   unit:       ��Ԫλ��
            area��      ����λ��(0~4)
            num:        ���
            data[0]:    �������ݳ��� data[0] = 0, û�д˳���
            data[1~n]:  ��������
�� �� ֵ��  ��ʶ��� TRUE(��Ч)/FALSE(��Ч)
*******************************************************************************/
void SceneEEPROM_Get(u8 unit, u8 area, u8 num, u8 *data)
{
    u16 flag = FindScene(unit,area,num);
	
    if(MACRO_SCENE_ERR != flag)
    {
        ReadDataFromEEPROM(SceneEEPROMSave_Addr(flag),GetCurSceneSpace(flag), data);  //��������                        
    }
    else
    {
        *data = 0;  //���س�������Ϊ��
    }
}

/*******************************************************************************
�� �� ����  SceneEEPROMSaveData_Seat
����˵����  ����һ�������洢λ��
��   ����   len:    Ŀ��洢���ݳ���
�� �� ֵ��  number: �洢��λ��ֵ,MACRO_SCENE_ERR:��ʾû�����뵽
*******************************************************************************/
static u16 SceneEEPROMData_Seat(const u8 len)
{
    u16 i;
	u16 xReturn = MACRO_SCENE_ERR;

	for(i = SceneEEPROMSave_Seat(len); i < SCENE_MAX_NUM; i++)
	{
		if (!(pSceneManage->sceneUseFlag[i / 8] & (1 << (i % 8))))   //����洢λ���ǿ��е�
		{
			xReturn = i;
			break;
		}
	}
	return (xReturn);
}
/*******************************************************************************
�� �� ����  SceneEEPROMData_Save
����˵����  ����EEPROM�洢����
��   ����   addr:   �洢�յ�ַ
            data:   Ҫ�洢�ĳ�������
�� �� ֵ��  �洢��ţ�0��127��/0xFF(û�д洢�ռ�)
*******************************************************************************/
void SceneEEPROMData_Save(u32 addr, u8 *data)
{
    WriteDataToEEPROM(addr, data[0] + 1, data); //��������
}

/*******************************************************************************
�� �� ����  SceneEEPROMSave_Addr
����˵����  ����EEPROM�洢��ַ
��   ����   seat:   �洢���
�� �� ֵ��  �洢��ַ
*******************************************************************************/
u32 SceneEEPROMSave_Addr(u8 seat)
{
    u32 addr = SCENE_DATA_START_ADD;

    if (seat < SCENE_SPACE1_NUM)
    {
        addr += seat * SCENE_DATA_SPACE1;
        goto RETURN;//λ�ô��ڴ洢�ռ�1
    }

    addr += SCENE_SPACE1_NUM * SCENE_DATA_SPACE1;
    seat -= SCENE_SPACE1_NUM;
    if (seat < SCENE_SPACE2_NUM)
    {
        addr += seat * SCENE_DATA_SPACE2;
        goto RETURN;//λ�ô��ڴ洢�ռ�2
    }

    addr += SCENE_SPACE2_NUM * SCENE_DATA_SPACE2;
    seat -= SCENE_SPACE2_NUM;
    if (seat < SCENE_SPACE3_NUM)
    {
        addr += seat * SCENE_DATA_SPACE3;
        goto RETURN;//λ�ô��ڴ洢�ռ�3
    }

RETURN:
    return addr;
}

/*******************************************************************************
�� �� ����  EEPROM_Write
����˵����  EEPROM_д������
��   ����   ����:  addr-EEPROM�е�ʵ�ʵ�ַ
            len-����
            data-Ҫд�����ͬ����
  ����ֵ:   ��Ϣִ�н��
*******************************************************************************/
MsgResult_t EEPROM_Write(u32 addr, u32 len, u8 *data)
{
    if ((addr + len) > A24C256          //��ַԽ��
            || addr < USERDATA_END_ADD
       )
    {
        return EEPROM_RAND_ERR;
    }

    if (WriteDataToEEPROM(addr, len, data) == FALSE)
    {
        return EEPROM_ERR;
    }

    return COMPLETE;
}
/*******************************************************************************
�� �� ����  EEPROM_Read
����˵����  EEPROM��ȡ����
��   ����   ����:  addr-EEPROM�е�ʵ�ʵ�ַ
            len-����
            data-Ҫд�����ͬ����
  ����ֵ:   ��Ϣִ�н��
*******************************************************************************/
MsgResult_t EEPROM_Read(u32 addr, u32 len, u8 *data)
{
    if ((addr + len) > A24C256          //��ַԽ��
            || addr < USERDATA_END_ADD
       )
    {
        return EEPROM_RAND_ERR;
    }

    if (ReadDataFromEEPROM(addr, len, data) == FALSE)
    {
        return EEPROM_ERR;
    }

    return COMPLETE;
}

/**************************Copyright BestFu 2014-05-14*************************/
