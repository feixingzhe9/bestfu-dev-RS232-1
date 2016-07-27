/***************************Copyright BestFu 2014-05-14*************************
文 件：  UserData.c
说 明：  用户数据管理文件
编 译：  Keil uVision4 V4.54.0.0
版 本：  v1.0
编 写：  Unarty
日 期：  2014.06.24
修　改： 2014.11.11 Uarty 修改UserData_Init中的初值判断，减少因EEPROM异常出现用户数据丢失的可能性。
		 2016.05.29 jay 重构场景功能
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
函 数 名：  UserData_Init()
功能说明：  用户数据初始化
参   数：   无
返 回 值：  无
修   改：   Uarty 2014.11.11 增加Cnt，减少因EEPROM异常而导致数据删除。
*******************************************************************************/
void UserData_Init(void)
{
    static u8 cnt = 0;  // Unarty Add 2014.11.11

    ReadDataFromEEPROM(SYSDATA_START_ADD, sizeof(gSysData), (u8 *)&gSysData);   //获取初次使用标识
    if (HardID_Check(gSysData.deviceID) != 0)   								//设备ID不相同
    {
        Thread_Login(ONCEDELAY, 0, 50, UserData_Init);      					//反复验证，防止EEPROM上电时异常// Unarty Add 2014.11.11
        if (++cnt > 3)   // Unarty Add 2014.11.11
        {
            Thread_Logout(UserData_Init);       								//去除返复验证接口// Unarty Add 2014.11.11
            FaultData_Init();
            UserEEPROMData_Init(FristLevel);        							//初始化用户EEPROM数据
        }
    }
    else if (cnt > 0)
    {
        cnt = 0;            													//Unarty 2014.12.02 修改，防止死锁。
        Unit_Init();
    }
}

/*******************************************************************************
函 数 名：  UserData_Init()
功能说明：  用户数据初始化
参   数：   FristLevel 不恢复433信道
            SecondLevel 恢复433信道
返 回 值：  无
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
        Set_433Channel_Attr((UnitPara_t *)tmp);//恢复出厂设置时，恢复到83信道
    }

    Upload_Fault(USER_DATA_RESET);  			//用户数据重置
}

/*******************************************************************************
函 数 名：  FaultData_Init
功能说明：  异常事件存储空间初始化
参   数：   data:   异常返回存储地址
返 回 值：  无
*******************************************************************************/
void FaultData_Init(void)
{
    WriteDataToEEPROMEx(FAULT_START_ADD, FAULT_SPACE, 0);
}

/*******************************************************************************
函 数 名：  FaultData_Save
功能说明：  异常事件存储
参   数：   event： 异常事件名
返 回 值：  无
*******************************************************************************/
void FaultData_Save(u8 event)
{
    u32 name;

    ReadDataFromEEPROM(FAULT_START_ADD + sizeof(name)*event,
                       sizeof(name), (u8 *)&name);     //获取标识
    name++;
    WriteDataToEEPROM(FAULT_START_ADD + sizeof(name)*event,
                      sizeof(name), (u8 *)&name);     //获取标识
}

/*******************************************************************************
函 数 名：  FaultData_Get
功能说明：  异常事件存储
参   数：   data:   异常返回存储地址
返 回 值：  无
*******************************************************************************/
void FaultData_Get(u8 *len, u8 *data)
{
    *len = FAULT_SPACE;
    ReadDataFromEEPROM(FAULT_START_ADD, *len, data);    //获取标识
}

/*******************************************************************************
函 数 名：  SysEEPROMData_Init()
功能说明：  系统EEPROM数据初始化
参   数：   无
返 回 值：  无
*******************************************************************************/
void SysEEPROMData_Init(void)
{
    gSysData.userID     = DEFAULT_USERID;   //用户号
    gSysData.deviceID   = HardID_Check(0);      //
    gSysData.sendCnt    = 2; //默认两次发送信号，周旺修改，2015.7.2
    gSysData.serialNum  = 0;
    memset((u8 *)gSysData.BandAddr, 0, sizeof(gSysData.BandAddr)); //清绑定地址转发功能，周旺增加
    gSysData.BandAreaEn = 0; //清区域广播转发功能，周旺增加
    WriteDataToEEPROM(SYSDATA_START_ADD, sizeof(gSysData), (u8 *)&gSysData);
}

/*******************************************************************************
函 数 名：  SysEEPROMData_Init()
功能说明：  系统EEPROM数据初始化
参   数：   size:   设置数据大小
            data:   设置数据内容
返 回 值：  无
*******************************************************************************/
u8 SysEEPROMData_Set(u8 size , u8 *data)
{
    return WriteDataToEEPROM(SYSDATA_START_ADD + ((u32)data - (u32)&gSysData), size, data);
}

/*******************************************************************************
函 数 名：  UnitPrivateEEPROMData_Init
功能说明：  单元私有EEPROM数据初始化
参   数：   无
返 回 值：  无
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
函 数 名：  UnitPrivateEEPROMUnitData_Init
功能说明：  单元私有EEPROM单元数据初始化
参   数：   unit： 目标格式化单元号
返 回 值：  无
*******************************************************************************/
void UnitPrivateEEPROMUnitData_Init(u8 unit)
{
    if (UnitTab[unit].DefEE != NULL)
    {
        UnitTab[unit].DefEE(unit);
    }
}

/*******************************************************************************
函 数 名：  UnitPublicEEPROMData_Init
功能说明：  单元共用EEPROM数据初始化
参   数：   无
返 回 值：  无
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
函 数 名：  UnitPublicEEPROMUnitData_Init
功能说明：  单元共用EEPROM单元数据初始化
参   数：   unit： 目标格式化单元号
返 回 值：  无
*******************************************************************************/
void UnitPublicEEPROMUnitData_Init(u8 unit)
{
    *(u32*)gUnitData[unit].area = DEFAULT_AREA; //默认区域号
    gUnitData[unit].type = 0;					//默认设备应用类型
    gUnitData[unit].able = 1;					//默认使能标识
    gUnitData[unit].common = 0;				//默认单元应用大类
    gUnitData[unit].UploadAble = 1;   //默认上报使能标记   yanhuan adding 2016/01/04
    gUnitData[unit].UploadSetTime = 3;//默认上报上报使能最大随机时间
    WriteDataToEEPROM(UNITDATA_START_ADD + ONCEUNITDATA_SPACE*unit,
                      sizeof(UnitData_t), (u8*)&gUnitData[unit]);
}

/*******************************************************************************
函 数 名：  UnitPublicEEPROMData_Get
功能说明：  获取单元共用EEPROM数据
参   数：   unitCnt:    获取单元总数
            unitData:   单元数据内容
返 回 值：  无
*******************************************************************************/
void UnitPublicEEPROMData_Get(u8 unitCnt, UnitData_t *unitData)
{
    while (unitCnt--)
    {
        ReadDataFromEEPROM(UNITDATA_START_ADD + (ONCEUNITDATA_SPACE * (unitCnt)), sizeof(UnitData_t), (u8 *)&unitData[unitCnt]);
    }
}

/*******************************************************************************
函 数 名：  UnitPublicEEPROMData_Set
功能说明：  设置单元共用EEPROM数据初始化
参   数：   size:   设置数据大小
            data:   设置数据内容
返 回 值：  无
*******************************************************************************/
u8 UnitPublicEEPROMData_Set(u8 size , u8 *data)
{
    u32 seat = (u32)data - (u32)&gUnitData;

    return WriteDataToEEPROM(UNITDATA_START_ADD + (ONCEUNITDATA_SPACE * (seat / sizeof(UnitData_t))) + seat % sizeof(UnitData_t), size, data);
}

/*******************************************************************************
函 数 名：  u8 SetDeviceChannelFlag(void)
功能说明：  设置设备供货信息
参   数：   供货信息
返 回 值：  无
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
函 数 名：  u32 GetDeviceChannelFlag(void)
功能说明：  获取设备渠道信息
参   数：   无
返 回 值：  供货信息
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
	return 1;		//默认智慧空间
}

/*******************************************************************************
函 数 名：  GroupEEPROMData_Init
功能说明：  组EEPROM数据初始化
参   数：   无
返 回 值：  无
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
函 数 名：  GroupEEPROMUnitData_Init
功能说明：  组EEPROM单元数据初始化
参   数：   unit： 目标格式化单元号
返 回 值：  无
*******************************************************************************/
void GroupEEPROMUnit_Init(const u8 unit)
{
    WriteDataToEEPROMEx(GROUP_START_ADD + (UNIT_GROUP_SPACE * unit), UNIT_GROUP_SPACE, 0);
}

/*******************************************************************************
函 数 名：  GroupEEPROM_Clr
功能说明：  清除组号标识位
参   数：   unit:  单元位置
            area： 区域位置(0~4)
            num:   组号
返 回 值：  无
*******************************************************************************/
void GroupEEPROM_Clr(const u8 unit, const  u8 area, const  u8 num)
{
    u8 groupAddr = 0;
	
    ReadDataFromEEPROM(GROUP_START_ADD + (UNIT_GROUP_SPACE * unit) + (AREA_GROUP_SPACE * area) + (num/8), 1, &groupAddr);
    if ((num < AREA_GROUP_NUM) && ((groupAddr & (1 << (num%8)))))   // 组号正确, 并且被设置
    {
        groupAddr &=  ~(1 << (num%8));
        WriteDataToEEPROM(GROUP_START_ADD + (UNIT_GROUP_SPACE * unit) + (AREA_GROUP_SPACE * area) + (num/8), 1, &groupAddr);
    }
    else if (CLEAR == num)     //清除整个区域
    {
		WriteDataToEEPROMEx(GROUP_START_ADD + (UNIT_GROUP_SPACE * unit) + (AREA_GROUP_SPACE * area), AREA_GROUP_SPACE, 0);
    }
}

/*******************************************************************************
函 数 名：  GroupEEPROMData_Set
功能说明：  设置组号标识是否有效
参   数：   unit:  单元位置
            area： 区域位置(0~4)
            num:   组号
返 回 值：  无
*******************************************************************************/
void GroupEEPROM_Set(const u8 unit,const u8 area,const u8 num)
{
    u8 groupAddr = 0;

    ReadDataFromEEPROM(GROUP_START_ADD + (UNIT_GROUP_SPACE * unit) + (AREA_GROUP_SPACE * area) + (num/8), 1, &groupAddr);
    if ((num < AREA_GROUP_NUM)&&(!(groupAddr & (1 << (num%8)))))   // 组号正确, 并且还未设置
    {
        groupAddr |=  1 << (num%8);
        WriteDataToEEPROM(GROUP_START_ADD + (UNIT_GROUP_SPACE * unit) + (AREA_GROUP_SPACE * area) + (num/8), 1, &groupAddr);
    }
}

/*******************************************************************************
函 数 名：  GroupEEPROMData_Get
功能说明：  获取组号标识是否有效
参   数：   unit:  单元位置
            area： 区域位置(0~4)
            num:   组号
返 回 值：  标识结果 TRUE(有效)/FALSE(无效)
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
函 数 名：  SceneData_Init
功能说明：  场景数据初始化
参   数：   无
返 回 值：  无
*******************************************************************************/
void SceneData_Init(void)
{	
	ReadDataFromEEPROM(SCENE_INDEX_START_ADD, sizeof(SceneIndexes_t)*SCENE_MAX_NUM, (u8*)&pSceneManage->SceneIndexes);
	ReadDataFromEEPROM(SCENE_USE_FLAG_ADD, SCENE_USE_SPACE + SCENE_TIME_USE_SPACE, (u8*)&pSceneManage->sceneUseFlag);
}

/*******************************************************************************
函 数 名：  SceneEEPROM_Init
功能说明：  场景EEPROM数据初始化
参   数：   无
返 回 值：  无
*******************************************************************************/
void SceneEEPROM_Init(void)
{
	memset((u8*)pSceneManage, 0, sizeof(sceneManage_t));
	WriteDataToEEPROMEx(SCENE_INDEX_START_ADD, sizeof(SceneIndexes_t)*SCENE_MAX_NUM, SCENE_DEFAULT_VALUE);
	WriteDataToEEPROMEx(SCENE_USE_FLAG_ADD, SCENE_USE_SPACE + SCENE_TIME_USE_SPACE, SCENE_DEFAULT_VALUE);
}

/*******************************************************************************
函 数 名：  SceneEEPROMUnitData_Init
功能说明：  场景EEPROM单元数据初始化
参   数：   unit： 目标格式化单元号
返 回 值：  无
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
函 数 名:  	FindScene
功能说明:  	查找一个场景
参    数:  	unit 单元号
			area 区域号
			sceneNum 场景号						
返 回 值:  	sceneAddr 场景的存储地址
			MACRO_SCENE_ERR：查找失败
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
函 数 名:  	DeleteScene
功能说明:  	删除一个场景
参    数:  	label:要删去场景的标号
返 回 值:   要删除场景的存储位置
*******************************************************************************/
static u8 DeleteScene(const u16 label)
{		
	CLR_DAT8_BIT(pSceneManage->sceneUseFlag, label);
	WriteDataToEEPROM(SCENE_USE_FLAG_ADD + label/8, 1, (u8*)&pSceneManage->sceneUseFlag[label/8]);
	memset(&pSceneManage->SceneIndexes[label], 0, sizeof(SceneIndexes_t));
	return WriteDataToEEPROMEx(SCENE_INDEX_START_ADD + sizeof(SceneIndexes_t)*label, sizeof(SceneIndexes_t), 0);
}

/*******************************************************************************
函 数 名：  static void  SceneUseFlag_Set(const u8 label)
功能说明：  设置场景内存使用标记
参   数：   label: 场景数据编号(0x00~0xff)
返 回 值：  无
*******************************************************************************/
static void  SceneUseFlag_Set(const u8 label)
{
	SET_DAT8_BIT(pSceneManage->sceneUseFlag, label);
	WriteDataToEEPROM(SCENE_USE_FLAG_ADD + (label)/8, 1, (u8*)&pSceneManage->sceneUseFlag[(label)/8]);
}

#if (SCENCETIME_EN > 0u)
/*******************************************************************************
函 数 名：  static void  SceneTimeUseFlag_Clr(const u8 label)
功能说明：  清除场景时间内存使用标记
参   数：   label: 场景数据编号(0x01~56)
返 回 值：  无
*******************************************************************************/
static void  SceneTimeUseFlag_Clr(const u8 label)
{
	CLR_DAT8_BIT(pSceneManage->sceneTimeUseFlag, label - 1);
	WriteDataToEEPROM(SCENE_TIME_USE_FLAG_ADD + (label - 1)/8, 1, (u8*)&pSceneManage->sceneTimeUseFlag[(label - 1)/8]);
}

/*******************************************************************************
函 数 名：  static void  SceneTimeUseFlag_Set(const u8 label)
功能说明：  设置场景时间内存使用标记
参   数：   label: 场景数据编号(0x01~56)
返 回 值：  无
*******************************************************************************/
static void  SceneTimeUseFlag_Set(const u8 label)
{
	SET_DAT8_BIT(pSceneManage->sceneTimeUseFlag, label - 1);
	WriteDataToEEPROM(SCENE_TIME_USE_FLAG_ADD + (label - 1)/8, 1, (u8*)&pSceneManage->sceneTimeUseFlag[(label - 1)/8]);
}

/*******************************************************************************
函 数 名：  inline u32 CheckTime_PositionRange(u8 position)
功能说明：  检查场景的起始存储位置
参   数：   position:待检查场景的起始存储位置的变量
返 回 值：  返回场景的起始存储位置的值
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
函 数 名：  SceneTimeSave_Seat
功能说明：  场景EEPROMTime存储位置
参   数：   seat:   存储长度
返 回 值：  存储位置
*******************************************************************************/
static u32 SceneTimeSave_Seat(const u8 len)
{
    if (len <= SCENE_TIME_DATA_SPACE1)    //长度处于第一存储空间
    {
        return  0;
    }
    else if (len <= (SCENE_TIME_DATA_SPACE2))     //长度处于第二存储空间
    {
        return SCENE_TIME_SPACE1_NUM;
    }
    else if (len <= (SCENE_TIME_DATA_SPACE3))     //长度处于第三存储空间
    {
        return (SCENE_TIME_SPACE1_NUM + SCENE_TIME_SPACE2_NUM);
    }

    return SCENE_TIME_TOTAL_NUM;   //长度越界
}

/*******************************************************************************
函 数 名：  SceneEEPROMTimeData_Seat
功能说明：  申请一个场景时间存储位置
参   数：   len:    目标存储数据长度
返 回 值：  number: 存储的位置值,MACRO_SCENE_ERR:表示没有申请到
*******************************************************************************/
static u8 SceneEEPROMTimeData_Seat(const u8 len)
{
    u16 i;
	u16 xReturn = SCENE_DEFAULT_VALUE;

	for(i = SceneTimeSave_Seat(len); i < SCENE_TIME_TOTAL_NUM; i++)
	{
		if (!(pSceneManage->sceneTimeUseFlag[i / 8] & (1 << (i % 8))))   //如果存储位置是空闲的
		{
			xReturn = i + 1;
			break;
		}
	}
	return (xReturn);
}

/*******************************************************************************
函 数 名：  SceneTimeEEPROMSave_Addr
功能说明：  场景时间EEPROM存储地址
参   数：   seat:   存储编号
返 回 值：  存储地址
*******************************************************************************/
static u32 SceneTimeEEPROMSave_Addr(u8 seat)
{
    u32 addr = SCENE_TIME_DATA_START_ADD;
	
	if(seat <= SCENE_TIME_SPACE1_NUM)
	{
		addr += (seat-1) * SCENE_TIME_DATA_SPACE1;
        goto RETURN;		//位置处于存储空间1
	}

    addr += SCENE_TIME_SPACE1_NUM * SCENE_DATA_SPACE1;
    seat -= SCENE_TIME_SPACE1_NUM;
    if (seat <= SCENE_SPACE2_NUM)
    {
        addr += (seat - 1) * SCENE_TIME_DATA_SPACE2;
        goto RETURN;//位置处于存储空间2
    }

    addr += SCENE_TIME_SPACE2_NUM * SCENE_TIME_DATA_SPACE2;
    seat -= SCENE_TIME_SPACE2_NUM;
    if (seat <= SCENE_TIME_SPACE3_NUM)
    {
        addr += (seat - 1) * SCENE_TIME_DATA_SPACE3;
        goto RETURN;//位置处于存储空间3
    }

RETURN:
    return addr;
}
#endif

/*******************************************************************************
函 数 名：  SceneEEPROM_Clr
功能说明：  清除场景
参   数：   unit:  单元位置
            area： 区域位置(0~4)
            num:   场景号
返 回 值：  无
*******************************************************************************/
void SceneEEPROM_Clr(const u8 unit, const u8 area, const u16 num)
{
    u16 label = 0;							//场景存储块的标号
	u16 i;
#if (SCENCETIME_EN > 0u)
	u8 timeLabel;
#endif

    if ((num < AREA_SCENE_NUM)&&(MACRO_SCENE_ERR != (label = FindScene(unit,area,num))))  //场景存储块的标号合法,注意FindScene的返回值,(MACRO_SCENE_ERR)表示不存在
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
	else if(CLEAR == num)     //清除整个区域
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
函 数 名：  inline u32 Check_PositionRange(u8 position)
功能说明：  检查场景的起始存储位置
参   数：   position:待检查场景的起始存储位置的变量
返 回 值：  返回场景的起始存储位置的值
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
函 数 名：  SceneEEPROMSave_Seat
功能说明：  场景EEPROM存储位置
参   数：   seat:   存储长度
返 回 值：  存储位置
*******************************************************************************/
u32 SceneEEPROMSave_Seat(const u8 len)
{
    if (len < SCENE_DATA_SPACE1)    //长度处于第一存储空间
    {
        return  0;
    }
    else if (len < (SCENE_DATA_SPACE2))     //长度处于第二存储空间
    {
        return SCENE_SPACE1_NUM;
    }
    else if (len < (SCENE_DATA_SPACE3))     //长度处于第三存储空间
    {
        return (SCENE_SPACE1_NUM + SCENE_SPACE2_NUM);
    }

    return SCENE_MAX_NUM;   //长度越界
}

/*******************************************************************************
函 数 名：  SceneEEPROM_Set
功能说明：  写入一个场景
参   数：   unit:       单元位置
            area：      区域位置(0~4)
            num:        情景号
            data[0]:    场景内容长度
            data[1~n]:  场景内容
返 回 值：  存储结果
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
			(Check_PositionRange(label) != SceneEEPROMSave_Seat(data[0])))     //相同场景号的情况下,所需要的存储空间内存块不同,需要重新申请
		
		{
			DeleteScene(label);				//清除之前存储标识,不删除时间参数
			label = MACRO_SCENE_ERR;
		}
	}
	
    if(MACRO_SCENE_ERR == label)   	//未找到该场景，场景还没有申请空间
    {
		if(MACRO_SCENE_ERR == (label = (SceneEEPROMData_Seat(data[0]))))//申请一个存储空间
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
函 数 名：  u32 SceneEEPROM_SetEx(u8 unit, u8 area, u8 num, u8 *data)
功能说明：  写入一个场景的扩展接口,支持延时顺序执行
参   数：   unit:       单元位置
            area：      区域位置(0~4)
            num:        情景号
            cnt:        属性命令数量
            data[0~n]:  场景时间数据内容
返 回 值：  存储结果
*******************************************************************************/
u32 SceneEEPROM_SetEx(u8 unit, u8 area, u8 num, u8 *data)
{
	u8 	data_tmp[SCENE_TIME_DATA_SPACE3 + 1];
	u16 label = FindScene(unit,area,num);
	u8 timePos = SCENE_DEFAULT_VALUE;
	u8 status = 0;
	
	SceneParamSeparate((u8*)data_tmp, (u8*)data);		//分离数据
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
				DeleteScene(label);				//清除之前存储标识,不删除时间参数
				label = MACRO_SCENE_ERR;
			}
		}
			
		if(SCENE_DEFAULT_VALUE == timePos)
		{
			if(SCENE_DEFAULT_VALUE == (timePos = (SceneEEPROMTimeData_Seat(data_tmp[0]))))//申请一个存储空间
			{
				return EEPROM_RAND_ERR;
			}
			status |= (1 << 0);
		}
		
		if(MACRO_SCENE_ERR == label)   	//未找到该场景，场景还没有申请空间
		{
			if(MACRO_SCENE_ERR == (label = (SceneEEPROMData_Seat(data[0]))))//申请一个存储空间
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
函 数 名：  SceneEEPROM_GetEx
功能说明：  获取场景号标识是否有效
参   数：   unit:       单元位置
            area：     区域位置(0~4)
            num:        组号
            data[0]:    场景内容长度 data[0] = 0, 没有此场景
            data[1~n]:  场景内容
返 回 值：  标识结果 TRUE(有效)/FALSE(无效)
*******************************************************************************/
u8 SceneEEPROM_GetEx(u8 unit, u8 area, u8 num, u8 *data)
{
    u8 	data_tmp[SCENE_TIME_DATA_SPACE3];
	u16 label = FindScene(unit,area,num);
	u8 sceneTimeLabel;
	
	if(MACRO_SCENE_ERR != label)
	{
		ReadDataFromEEPROM(SceneEEPROMSave_Addr(label),GetCurSceneSpace(label), data);  //保存数据   
		if((SCENE_DEFAULT_VALUE != (sceneTimeLabel = pSceneManage->SceneIndexes[label].sceneTimeSavePos))&&\
			(sceneTimeLabel <= SCENE_TIME_TOTAL_NUM))
		{
			ReadDataFromEEPROM(SceneTimeEEPROMSave_Addr(sceneTimeLabel), GetCurSceneTimeSpace(sceneTimeLabel), data_tmp); //获取时间数据
			SceneParamCombine(data_tmp, data);
			return TRUE;
		}
	}
    else
    {
        *data = 0;  //返回场景长度为零
    }
    return FALSE;
}

/*******************************************************************************
函 数 名：  static void SceneParamSeparate(u8 *timedata, u8 *orgdata)
功能说明：  场景数据分离函数
参   数：   timedata：时间数据存储指针,其中timedata[0]表哦是其长度
			orgdata：时间数据存储指针,orgdata[0]表示是其长度
返 回 值：  无
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
函 数 名：  static void SceneParamCombine(u8 *timedata, u8 *orgdata)
功能说明：  场景数据合并函数
参   数：   timedata：时间数据存储指针
			orgdata：时间数据存储指针,orgdata[0]表示长度
返 回 值：  无
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
函 数 名：  GetCurSceneTimeSpace
功能说明：  获取场景时间标号对应的存储大小
参   数：   label:场景时间标号
返 回 值：  内存块的大小
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
函 数 名：  ClearSceneTimePara
功能说明：  清除场景中的时间参数
参   数：   label:场景标号
返 回 值：  被SceneEEPROM_Set函数使用
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
函 数 名：  GetCurSceneSpace
功能说明：  获取场景标号对应的存储大小
参   数：   label:场景时间标号
返 回 值：  内存块的大小
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
函 数 名：  SceneEEPROM_Get
功能说明：  获取场景号标识是否有效
参   数：   unit:       单元位置
            area：      区域位置(0~4)
            num:        组号
            data[0]:    场景内容长度 data[0] = 0, 没有此场景
            data[1~n]:  场景内容
返 回 值：  标识结果 TRUE(有效)/FALSE(无效)
*******************************************************************************/
void SceneEEPROM_Get(u8 unit, u8 area, u8 num, u8 *data)
{
    u16 flag = FindScene(unit,area,num);
	
    if(MACRO_SCENE_ERR != flag)
    {
        ReadDataFromEEPROM(SceneEEPROMSave_Addr(flag),GetCurSceneSpace(flag), data);  //保存数据                        
    }
    else
    {
        *data = 0;  //返回场景长度为零
    }
}

/*******************************************************************************
函 数 名：  SceneEEPROMSaveData_Seat
功能说明：  申请一个场景存储位置
参   数：   len:    目标存储数据长度
返 回 值：  number: 存储的位置值,MACRO_SCENE_ERR:表示没有申请到
*******************************************************************************/
static u16 SceneEEPROMData_Seat(const u8 len)
{
    u16 i;
	u16 xReturn = MACRO_SCENE_ERR;

	for(i = SceneEEPROMSave_Seat(len); i < SCENE_MAX_NUM; i++)
	{
		if (!(pSceneManage->sceneUseFlag[i / 8] & (1 << (i % 8))))   //如果存储位置是空闲的
		{
			xReturn = i;
			break;
		}
	}
	return (xReturn);
}
/*******************************************************************************
函 数 名：  SceneEEPROMData_Save
功能说明：  场景EEPROM存储数据
参   数：   addr:   存储空地址
            data:   要存储的场景数据
返 回 值：  存储编号（0～127）/0xFF(没有存储空间)
*******************************************************************************/
void SceneEEPROMData_Save(u32 addr, u8 *data)
{
    WriteDataToEEPROM(addr, data[0] + 1, data); //保存数据
}

/*******************************************************************************
函 数 名：  SceneEEPROMSave_Addr
功能说明：  场景EEPROM存储地址
参   数：   seat:   存储编号
返 回 值：  存储地址
*******************************************************************************/
u32 SceneEEPROMSave_Addr(u8 seat)
{
    u32 addr = SCENE_DATA_START_ADD;

    if (seat < SCENE_SPACE1_NUM)
    {
        addr += seat * SCENE_DATA_SPACE1;
        goto RETURN;//位置处于存储空间1
    }

    addr += SCENE_SPACE1_NUM * SCENE_DATA_SPACE1;
    seat -= SCENE_SPACE1_NUM;
    if (seat < SCENE_SPACE2_NUM)
    {
        addr += seat * SCENE_DATA_SPACE2;
        goto RETURN;//位置处于存储空间2
    }

    addr += SCENE_SPACE2_NUM * SCENE_DATA_SPACE2;
    seat -= SCENE_SPACE2_NUM;
    if (seat < SCENE_SPACE3_NUM)
    {
        addr += seat * SCENE_DATA_SPACE3;
        goto RETURN;//位置处于存储空间3
    }

RETURN:
    return addr;
}

/*******************************************************************************
函 数 名：  EEPROM_Write
功能说明：  EEPROM_写入数据
参   数：   参数:  addr-EEPROM中的实际地址
            len-长度
            data-要写入的相同数据
  返回值:   消息执行结果
*******************************************************************************/
MsgResult_t EEPROM_Write(u32 addr, u32 len, u8 *data)
{
    if ((addr + len) > A24C256          //地址越界
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
函 数 名：  EEPROM_Read
功能说明：  EEPROM读取数据
参   数：   参数:  addr-EEPROM中的实际地址
            len-长度
            data-要写入的相同数据
  返回值:   消息执行结果
*******************************************************************************/
MsgResult_t EEPROM_Read(u32 addr, u32 len, u8 *data)
{
    if ((addr + len) > A24C256          //地址越界
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
