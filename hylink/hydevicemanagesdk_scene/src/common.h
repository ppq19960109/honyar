#ifndef _COMMON_H_
#define _COMMON_H_
#include <sys/time.h>

/*用于开启仅采用涂鸦模块的设备*/
//#define TY_DEV   (1)

/*用于开启多控开关状态同步的功能*/
//#define MULTI_SWITCH_SYNC   (1)


#define STANDER_ZHA  (1)

#define GW_FIRMWARE_VERSION "1.0.0"
#define GW_SOFTWARE_VERSION "1.4.8"

#define GW_GATEWAY_DEVICE_ID "0000000000000000"
#define GW_GATEWAY_MODEL_ID	"000000"

/**************************静态变量-路径长度定义****************************/
#define DATA_BASE_PATH_MAX_LENGTH		(30)
#define DEV_CONFIGUR_FILE_PATH_MAX_LENGTH	(30)
#define FILE_DOWN_LOAD_PATH_MAX_LENGTH	(30)
#define OTA_CONFIGURE_FILE_PATH_MAX_LENGTH (30)


/******************************设备类型********************************/
#define DEVICE_TYPE_ZHA		"ZHA"
#define DEVICE_TYPE_WIFI	"WIFI"


/*********************rex_ZHA设备命令类型代码****************************/
#define COMMAND_TYPE_GET_ZIGBEE_NETWORK_INFO (0x0100)
#define COMMAND_TYPE_ALLOW_TO_JOIN_NETWORK	(0x0101)
#define COMMAND_TYPE_SET_ZIGBEE_NETWORK		(0x0102)
#define COMMAND_TYPE_GET_DEVICE_STATE       (0x0103)
#define COMMAND_TYPE_SOUND_AND_LIGHT_ALARM  (0x0104)
#define COMMAND_TYPE_SET_POWER_SUPPLY_STATUS (0x0105)
#define COMMAND_TYPE_LIGHT_SWITCH           (0x0106)
#define COMMAND_TYPE_LIGHT_LEVEL			(0x0107)
#define COMMAND_TYPE_LIGHT_TEMPERATURE		(0x0108)
#define COMMAND_TYPE_LIGHT_HSL				(0x0109)
#define COMMAND_TYPE_LIGHT_XY				(0x010A)
#define COMMAND_TYPE_LOCK					(0x010B)
#define COMMAND_TYPE_CURTAIN_SET_LEVEL		(0x010C)
#define COMMAND_TYPE_CURTAIN_OPERATION		(0x010D)
#define COMMAND_TYPE_CANCELLATION			(0x010E)
#define COMMAND_TYPE_GET_SDK_VERSION		(0x010F)
#define COMMAND_TYPE_GET_COO_VERSION		(0x0110)
#define COMMAND_TYPE_GET_COO_SIGNATURE_DATA	(0x0111)
#define COMMAND_TYPE_SET_ZIGBEE_CHANNEL		(0x0112)
#define COMMAND_TYPE_SET_ZIGBEE_NETWORK_IDENTIFICATION (0x0113)
#define COMMAND_TYPE_GET_REPROTING_PERIOD	(0x0114)
#define COMMAND_TYPE_SET_REPROTING_PERIOD	(0x0115)
#define COMMAND_TYPE_GET_DEVICE_INFORMATION (0x0116)
#define COMMAND_TYPE_GET_NEIGHBOR_INFORMATION	(0x0118)
#define COMMAND_TYPE_GET_SUBNODE_INFORMATION	(0x011A)
#define COMMAND_TYPE_DEVICE_OTA				(0x011B)
#define COMMAND_TYPE_SET_TEMPERATURE		(0x011C)
#define COMMAND_TYPE_SET_HUMIDITY			(0x011D) //用于加热制冷设备
#define COMMAND_TYPE_SET_FAN_MODE			(0x011E) //用于新风系统、加热/制冷设备、空调温控设备
#define COMMAND_TYPE_SET_WORK_MODE			(0x011F)//用于加热/制冷设备、空调温控设备

#define COMMAND_TYPE_MANAGE_GROUP			(0x0120)
#define COMMAND_TYPE_MANAGE_SCENE			(0x0121)
#define COMMAND_TYPE_SET_SCENE_PANEL		(0x0122)
#define COMMAND_TYPE_GET_DEV_POWER_SUPPLY_INFO		(0x0123)
#define COMMAND_TYPE_FACTORY_RESET			(0x0124)
#define COMMAND_TYPE_BIND_ZHA			(0x0125)

#define COMMAND_TYPE_SET_HUE 				(0x0127)
#define COMMAND_TYPE_SET_STAURATION			(0x0128)


#define COMMAND_TYPE_SETUP_NETWORK			(0x01F0)
#define COMMAND_TYPE_DEBUG_COMMAND			(0x01F1)
#define COMMAND_TYPE_MANAGEMENT_SERVICE		(0x01F2)
#define COMMAND_TYPE_CALL_DEVICE_RESPONSE   (0x01F8)

#define COMMAND_TYPE_OPERATE_ATTRIBUTE		(0x01EF)
#define COMMAND_TYPE_SET_CURTAIN_MODE       (0x01FC)

#define COMMAND_TYPE_SET_JOIN_MODE			(0x012E)
#define COMMAND_TYPE_OPEN_FAST_JOIN         (0x01E0)
#define COMMAND_TYPE_CLOSE_FAST_JOIN        (0x01E1)

/***********************Rex_ZHA子设备类型代码***********************************/

#define REX_DEV_TYPE_1_WAY_SOCKET			(0x1001)
#define REX_DEV_TYPE_2_WAY_SOCKET			(0x1002)
#define REX_DEV_TYPE_3_WAY_SOCKET			(0x1003)
#define REX_DEV_TYPE_4_WAY_SOCKET			(0x1004)

#define REX_DEV_TYPE_1_WAY_SWITCH			(0x1101)
#define REX_DEV_TYPE_2_WAY_SWITCH			(0x1102)
#define REX_DEV_TYPE_3_WAY_SWITCH			(0x1103)
#define REX_DEV_TYPE_4_WAY_SWITCH			(0x1104)

#define REX_DEV_TYPE_1_WAY_ADJUST_LIGHT		(0x1105)
#define REX_DEV_TYPE_2_WAY_ADJUST_LIGHT		(0x1106)
#define REX_DEV_TYPE_3_WAY_ADJUST_LIGHT		(0x1107)
#define REX_DEV_TYPE_4_WAY_ADJUST_LIGHT		(0x1108)

#define REX_DEV_TYPE_DLT					(0x1109)
#define REX_DEV_TYPE_HSL_LIGHT				(0x1110)

#define REX_DEV_TYPE_1_WAY_AUX_DLT			(0x1116)
#define REX_DEV_TYPE_2_WAY_AUX_DLT			(0x1117)
#define REX_DEV_TYPE_3_WAY_AUX_DLT			(0x1118)
#define REX_DEV_TYPE_4_WAY_AUX_DLT			(0x1119)

#define REX_DEV_TYPE_XYY_LIGHT				(0x1111)

#define REX_DEV_TYPE_SECURITY_ALARM			(0x1201)
#define REX_DEV_TYPE_SECURITY_REMOTE_CONTROL		(0x1202)

#define REX_DEV_TYPE_SMART_LOCK				(0x1203)

#define REX_DEV_TYPE_DOOR_MAGNETISM			(0x1204)
#define REX_DEV_TYPE_THERMAL_RELEASE		(0x1205)
#define REX_DEV_TYPE_SMOKE_GAS_SENSOR		(0X1206)
#define REX_DEV_TYPE_WATER_OVERFLOW_SENSOR	(0X1207)

#define REX_DEV_TYPE_HUMMAN_BODY_INDUCTION	(0X1208)

#define REX_DEV_TYPE_TEMPERATURE_SENSOR		(0x1301)
#define REX_DEV_TYPE_HYMIDITY_SENSOR		(0x1302)
#define REX_DEV_TYPE_ILLUMINACE_SENSOR		(0x1303)

#define REX_DEV_TYPE_ELECTRIC_CURTAIN		(0x1304)
#define REX_DEV_TYPE_FRESH_AIR_SYSTEM		(0x1305)
#define REX_DEV_TYPE_2_WAY_FRESH_AIR_SYSTEM	(0X1350)
#define REX_DEV_TYPE_HEART_COOL_DEVICE		(0x1306)

#define REX_DEV_TYPE_1_WAY_AIR_CONDITIONER	(0x1307)
#define REX_DEV_TYPE_2_WAY_AIR_CONDITIONER	(0x1308)
#define REX_DEV_TYPE_3_WAY_AIR_CONDITIONER	(0x1309)
#define REX_DEV_TYPE_4_WAY_AIR_CONDITIONER	(0x1310)
#define REX_DEV_TYPE_5_WAY_AIR_CONDITIONER	(0x1311)
#define REX_DEV_TYPE_6_WAY_AIR_CONDITIONER	(0x1312)
#define REX_DEV_TYPE_7_WAY_AIR_CONDITIONER	(0x1330)
#define REX_DEV_TYPE_8_WAY_AIR_CONDITIONER	(0x1331)
#define REX_DEV_TYPE_9_WAY_AIR_CONDITIONER	(0x1332)
#define REX_DEV_TYPE_10_WAY_AIR_CONDITIONER	(0x1333)
#define REX_DEV_TYPE_11_WAY_AIR_CONDITIONER	(0x1334)
#define REX_DEV_TYPE_12_WAY_AIR_CONDITIONER	(0x1335)
#define REX_DEV_TYPE_13_WAY_AIR_CONDITIONER	(0x1336)
#define REX_DEV_TYPE_14_WAY_AIR_CONDITIONER	(0x1337)
#define REX_DEV_TYPE_15_WAY_AIR_CONDITIONER	(0x1338)
#define REX_DEV_TYPE_16_WAY_AIR_CONDITIONER	(0x1339)

#define REX_DEV_TYPE_SCENE_PANEL			(0x1313)
#define REX_DEV_TYPE_PM25_SENSOR			(0x1314)
#define REX_DEV_TYPE_CO2_SENSOR			    (0x1315)
#define REX_DEV_TYPE_CH4_SENSOR			    (0x1316)


/*************************TRD表属性名称******************************************/
#define ATTRIBUTE_NAME_SWITHCH_ALL "Switch_All"
#define ATTRIBUTE_NAME_SWITHCH		"Switch"
#define ATTRIBUTE_NAME_SWITHCH_1	"Switch_1"
#define ATTRIBUTE_NAME_SWITHCH_2	"Switch_2"
#define ATTRIBUTE_NAME_SWITHCH_3	"Switch_3"
#define ATTRIBUTE_NAME_SWITHCH_4	"Switch_4"
#define ATTRIBUTE_NAME_SWITHCH_5	"Switch_5"
#define ATTRIBUTE_NAME_SWITHCH_6	"Switch_6"
#define ATTRIBUTE_NAME_SWITHCH_7	"Switch_7"
#define ATTRIBUTE_NAME_SWITHCH_8	"Switch_8"
#define ATTRIBUTE_NAME_SWITHCH_9	"Switch_9"
#define ATTRIBUTE_NAME_SWITHCH_10	"Switch_10"
#define ATTRIBUTE_NAME_SWITHCH_11	"Switch_11"
#define ATTRIBUTE_NAME_SWITHCH_12	"Switch_12"
#define ATTRIBUTE_NAME_SWITHCH_13	"Switch_13"
#define ATTRIBUTE_NAME_SWITHCH_14	"Switch_14"
#define ATTRIBUTE_NAME_SWITHCH_15	"Switch_15"
#define ATTRIBUTE_NAME_SWITHCH_16	"Switch_16"

#define INSTRUCTION_NAME_TUMBLER_SWITCH "TumblerSwitch" /**/
#define INSTRUCTION_NAME_TUMBLER_SWITCH_1 "TumblerSwitch_1" /**/
#define INSTRUCTION_NAME_TUMBLER_SWITCH_2 "TumblerSwitch_2" /**/
#define INSTRUCTION_NAME_TUMBLER_SWITCH_3 "TumblerSwitch_3" /**/
#define INSTRUCTION_NAME_TUMBLER_SWITCH_4 "TumblerSwitch_4" /**/

#define ATTRIBUTE_NAME_DCDELAY	"ZDCDelay"
#define ATTRIBUTE_NAME_LOCAL_CONFIG	"localConfiguration"

#define ATTRIBUTE_NAME_LED_ENABLE  	"LedEnable"
#define ATTRIBUTE_NAME_POWER_OFF_PROTECTION  	"PowerOffProtection"
#define ATTRIBUTE_NAME_VOTAGE_OVER_LOAD_PROTECTION "VoltageOverloadProtection"
#define ATTRIBUTE_NAME_VOTAGE_OVER_LOAD_PROTECTION_1 "VoltageOverloadProtection_1"
#define ATTRIBUTE_NAME_VOTAGE_OVER_LOAD_PROTECTION_2 "VoltageOverloadProtection_2"

#define ATTRIBUTE_NAME_OVERLOAD_PROTECTION	"OverloadProtection"
#define ATTRIBUTE_NAME_OVERLOAD_PROTECTION_1	"OverloadProtection_1"
#define ATTRIBUTE_NAME_OVERLOAD_PROTECTION_2	"OverloadProtection_2"

#define ATTRIBUTE_NAME_OVER_LOAD_PROTECT_ENABLE "OverloadProtectEnable"
#define ATTRIBUTE_NAME_VOLTAGE_OVER_LOAD       "VoltageOverload"
#define ATTRIBUTE_NAME_CURRENT_OVER_LOAD       "CurrentOverload"

#define ATTRIBUTE_NAME_POWER_CONSUMPTIONCLEAR 	"PowerConsumptionClear"
#define ATTRIBUTE_NAME_VOLTAGE_COFFICIENT 		"VoltageCoefficient"
#define ATTRIBUTE_NAME_CURRENT_COFFICIENT 		"CurrentCoefficient"
#define ATTRIBUTE_NAME_POWER_COFFICIENT 		"PowerCoefficient"
#define ATTRIBUTE_NAME_POWER_WASTER_COFFICIENT "PowerWasteCoefficient"


#define ATTRIBUTE_NAME_COLOR_TEMPERATURE	"ColorTemperature"
#define ATTRIBUTE_NAME_LUMINANCE			"Luminance"
#define ATTRIBUTE_NAME_LIGHT_MODE			"LightMode"
#define ATTRIBUTE_NAME_MEASURE_LUMINANCE	"MeasuredIlluminance"

#define ATTRIBUTE_NAME_SMOKE_ALARM			"SmokeAlarm"
#define ATTRIBUTE_NAME_BATTERY_PERCENTAGE	"BatteryPercentage"

#define ATTRIBUTE_NAME_CURTAIN_OPERATION	"CurtainOperation"
#define ATTRIBUTE_NAME_CURTAIN_OPERATION_1	"CurtainOperation_1"
#define ATTRIBUTE_NAME_CURTAIN_OPERATION_2	"CurtainOperation_2"


#define ATTRIBUTE_NAME_CURTAIN_MODE			"CurtainMode"
#define ATTRIBUTE_NAME_RUN_MODE             "RunMode"
#define ATTRIBUTE_NAME_RUN_MODE_1			"RunMode_1"
#define ATTRIBUTE_NAME_RUN_MODE_2           "RunMode_2"
#define ATTRIBUTE_NAME_CURTAIN_POSITION		"CurtainPosition"
#define ATTRIBUTE_NAME_CURTAIN_POSITION_1	"CurtainPosition_1"
#define ATTRIBUTE_NAME_CURTAIN_POSITION_2	"CurtainPosition_2"
#define ATTRIBUTE_NAME_WINRAINSENSOR        "WindRainSensor"
#define ATTRIBUTE_NAME_ATOMIZATION        "Atomization"
#define ATTRIBUTE_NAME_ATOMIZATION_1        "Atomization_1"
#define ATTRIBUTE_NAME_ATOMIZATION_2        "Atomization_2"

#define ATTRIBUTE_NAME_MOTION_ALARM			"MotionAlarm"
#define ATTRIBUTE_NAME_EMERGENCY_ALARM		"EmergencyAlarm"
#define ATTRIBUTE_NAME_CURRENT_LUMINANCE 	"CurrentLuminance"

#define ATTRIBUTE_NAME_CONTACT_ALARM		"ContactAlarm"
#define ATTRIBUTE_NAME_RSSI					"Rssi"
#define ATTRIBUTE_NAME_TAMPER_ALARM			"TamperAlarm"
#define ATTRIBUTE_NAME_LOW_BARRERY_ALARM    "LowBatteryAlarm"

#define ATTRIBUTE_NAME_KEY_FOB_VALUE		"KeyFobValue"

#define ATTRIBUTE_NAME_WATER_ALARM			"WaterAlarm"
//#define ATTRIBUTE_NAME_LUMINANCE			"Luminance"
#define ATTRIBUTE_NAME_MOTION_CURTAIN_ALARM "MotionCurtainAlarm"
#define ATTRIBUTE_NAME_DIRECTION_ALARM       "DirectionAlarm"

#define ATTRIBUTE_NAME_PM25					"PM25"
#define ATTRIBUTE_NAME_TVOC					"TVOC"
#define ATTRIBUTE_NAME_VOC					"VOC"
#define ATTRIBUTE_NAME_CO2					"CO2"
#define ATTRIBUTE_NAME_CO					"COValue"
#define ATTRIBUTE_NAME_ERROR_CODE			"ErrorCode"
#define ATTRIBUTE_NAME_HCHO					"HCHO"
#define ATTRIBUTE_NAME_ILLUMINANCE 			"Illuminance"

#define ATTRIBUTE_NAME_CURRENT_HUMIDITY		"CurrentHumidity"
#define ATTRIBUTE_NAME_CURRENT_TEMPERATURE	"CurrentTemperature"
#define ATTRIBUTE_NAME_CURRENT_PRESSURE		"CurrentPressure"

#define ATTRIBUTE_NAME_CURRENT_TEMP			"CurrentTemp"
#define ATTRIBUTE_NAME_CURRENT_TEMPERATURE_1		"CurrentTemperature_1"
#define ATTRIBUTE_NAME_CURRENT_TEMPERATURE_2		"CurrentTemperature_2"
#define ATTRIBUTE_NAME_CURRENT_TEMPERATURE_3		"CurrentTemperature_3"
#define ATTRIBUTE_NAME_CURRENT_TEMPERATURE_4		"CurrentTemperature_4"
#define ATTRIBUTE_NAME_CURRENT_TEMPERATURE_5		"CurrentTemperature_5"
#define ATTRIBUTE_NAME_CURRENT_TEMPERATURE_6		"CurrentTemperature_6"
#define ATTRIBUTE_NAME_CURRENT_TEMPERATURE_7		"CurrentTemperature_7"
#define ATTRIBUTE_NAME_CURRENT_TEMPERATURE_8		"CurrentTemperature_8"
#define ATTRIBUTE_NAME_CURRENT_TEMPERATURE_9		"CurrentTemperature_9"
#define ATTRIBUTE_NAME_CURRENT_TEMPERATURE_10		"CurrentTemperature_10"
#define ATTRIBUTE_NAME_CURRENT_TEMPERATURE_11		"CurrentTemperature_11"
#define ATTRIBUTE_NAME_CURRENT_TEMPERATURE_12		"CurrentTemperature_12"
#define ATTRIBUTE_NAME_CURRENT_TEMPERATURE_13		"CurrentTemperature_13"
#define ATTRIBUTE_NAME_CURRENT_TEMPERATURE_14		"CurrentTemperature_14"
#define ATTRIBUTE_NAME_CURRENT_TEMPERATURE_15		"CurrentTemperature_15"
#define ATTRIBUTE_NAME_CURRENT_TEMPERATURE_16		"CurrentTemperature_16"

#define ATTRIBUTE_NAME_GAS_ALARM			"GasAlarm"
#define ATTRIBUTE_NAME_CO_ALARM				"CoAlarm"
#define ATTRIBUTE_NAME_SIREN_WARNING_MODE	"SirenWarningMode"

#define ATTRIBUTE_NAME_RMS_CURRENT			"RmsCurrent"
#define ATTRIBUTE_NAME_RMS_VOLTAGE			"RmsVoltage"
#define ATTRIBUTE_NAME_SUM_ELECTRIC			"SumElectric"
#define ATTRIBUTE_NAME_ACTIVE_POWER			"ActivePower"
#define ATTRIBUTE_NAME_SWITCH_CHILD_LOCK	"SwitchChildLock"
#define ATTRIBUTE_NAME_SWITCH_MULTIPLE_1	"SwitchMultiple_1"
#define ATTRIBUTE_NAME_SWITCH_MULTIPLE_2	"SwitchMultiple_2"
#define ATTRIBUTE_NAME_SWITCH_MULTIPLE_ALL	"SwitchMultiple_All"

#define ATTRIBUTE_NAME_WIND_SPEED			"WindSpeed"
#define ATTRIBUTE_NAME_WIND_SPEED_1			"WindSpeed_1"
#define ATTRIBUTE_NAME_WIND_SPEED_2			"WindSpeed_2"
#define ATTRIBUTE_NAME_WIND_SPEED_3			"WindSpeed_3"
#define ATTRIBUTE_NAME_WIND_SPEED_4			"WindSpeed_4"
#define ATTRIBUTE_NAME_WIND_SPEED_5			"WindSpeed_5"
#define ATTRIBUTE_NAME_WIND_SPEED_6			"WindSpeed_6"
#define ATTRIBUTE_NAME_WIND_SPEED_7			"WindSpeed_7"
#define ATTRIBUTE_NAME_WIND_SPEED_8			"WindSpeed_8"
#define ATTRIBUTE_NAME_WIND_SPEED_9			"WindSpeed_9"
#define ATTRIBUTE_NAME_WIND_SPEED_10		"WindSpeed_10"
#define ATTRIBUTE_NAME_WIND_SPEED_11		"WindSpeed_11"
#define ATTRIBUTE_NAME_WIND_SPEED_12		"WindSpeed_12"
#define ATTRIBUTE_NAME_WIND_SPEED_13		"WindSpeed_13"
#define ATTRIBUTE_NAME_WIND_SPEED_14		"WindSpeed_14"
#define ATTRIBUTE_NAME_WIND_SPEED_15		"WindSpeed_15"
#define ATTRIBUTE_NAME_WIND_SPEED_16		"WindSpeed_16"

#define ATTRIBUTE_NAME_TEMPERATUR			"Temperature"
#define ATTRIBUTE_NAME_TARGET_TEMPERATURE   "TargetTemperature"
#define ATTRIBUTE_NAME_TARGET_TEMPERATURE_1   "TargetTemperature_1"
#define ATTRIBUTE_NAME_TARGET_TEMPERATURE_2   "TargetTemperature_2"
#define ATTRIBUTE_NAME_TARGET_TEMPERATURE_3   "TargetTemperature_3"
#define ATTRIBUTE_NAME_TARGET_TEMPERATURE_4   "TargetTemperature_4"
#define ATTRIBUTE_NAME_TARGET_TEMPERATURE_5   "TargetTemperature_5"
#define ATTRIBUTE_NAME_TARGET_TEMPERATURE_6   "TargetTemperature_6"
#define ATTRIBUTE_NAME_TARGET_TEMPERATURE_7   "TargetTemperature_7"
#define ATTRIBUTE_NAME_TARGET_TEMPERATURE_8   "TargetTemperature_8"
#define ATTRIBUTE_NAME_TARGET_TEMPERATURE_9   "TargetTemperature_9"
#define ATTRIBUTE_NAME_TARGET_TEMPERATURE_10   "TargetTemperature_10"
#define ATTRIBUTE_NAME_TARGET_TEMPERATURE_11   "TargetTemperature_11"
#define ATTRIBUTE_NAME_TARGET_TEMPERATURE_12   "TargetTemperature_12"
#define ATTRIBUTE_NAME_TARGET_TEMPERATURE_13   "TargetTemperature_13"
#define ATTRIBUTE_NAME_TARGET_TEMPERATURE_14   "TargetTemperature_14"
#define ATTRIBUTE_NAME_TARGET_TEMPERATURE_15   "TargetTemperature_15"
#define ATTRIBUTE_NAME_TARGET_TEMPERATURE_16   "TargetTemperature_16"

#define ATTRIBUTE_NAME_MODE					"WorkMode"
#define ATTRIBUTE_NAME_MODE_1				"WorkMode_1"
#define ATTRIBUTE_NAME_MODE_2				"WorkMode_2"
#define ATTRIBUTE_NAME_MODE_3				"WorkMode_3"
#define ATTRIBUTE_NAME_MODE_4				"WorkMode_4"
#define ATTRIBUTE_NAME_MODE_5				"WorkMode_5"
#define ATTRIBUTE_NAME_MODE_6				"WorkMode_6"
#define ATTRIBUTE_NAME_MODE_7				"WorkMode_7"
#define ATTRIBUTE_NAME_MODE_8				"WorkMode_8"
#define ATTRIBUTE_NAME_MODE_9				"WorkMode_9"
#define ATTRIBUTE_NAME_MODE_10				"WorkMode_10"
#define ATTRIBUTE_NAME_MODE_11				"WorkMode_11"
#define ATTRIBUTE_NAME_MODE_12				"WorkMode_12"
#define ATTRIBUTE_NAME_MODE_13				"WorkMode_13"
#define ATTRIBUTE_NAME_MODE_14				"WorkMode_14"
#define ATTRIBUTE_NAME_MODE_15				"WorkMode_15"
#define ATTRIBUTE_NAME_MODE_16				"WorkMode_16"

#define ATTRIBUTE_NAME_LOCK_STATE			"LockState"
#define ATTRIBUTE_NAME_LOCK_ALARM			"LockAlarm"
#define ATTRIBUTE_NAME_ARM_MODE				"ArmMode"
#define ATTRIBUTE_NAME_USR_ADD				"UserAdd"
#define ATTRIBUTE_NAME_USR_DELETE			"UserDelete"
#define ATTRIBUTE_NAME_OPEN_DOOR			"OpenDoor"
#define ATTRIBUTE_NAME_ARMMODE				"ArmMode"

#define ATTRIBUTE_NAME_ACTUATOR_OPERATION	"ActuatorOperation"
#define ATTRIBUTE_NAME_ACTUATOR_POSITION	"ActuatorPosition"

#define ATTRIBUTE_NAME_SATURATION           "Saturation"
#define ATTRIBUTE_NAME_HUE					"Hue"
#define ATTRIBUTE_NAME_LIGHTNESS			"Lightness"

#define ATTRIBUTE_NAME_KEYMODE				"KeyMode"

#define ATTRIBUTE_NAME_FULL_SCENE_DLT_GROUP_ENABLE 	"DltEnable"
#define ATTRIBUTE_NAME_FULL_SCENE_DLT_SWITCH			"DltSwitch"
#define ATTRIBUTE_NAME_FULL_SCENE_DLT_LEVEL				"DltLevel"
#define ATTRIBUTE_NAME_FULL_SCENE_DLT_TEMPERATURE		"DltTemp"
#define ATTRIBUTE_NAME_FULL_SCENE_SWITCH_GROUP_ENABLE 	"LightSwitchEnable"
#define ATTRIBUTE_NAME_FULL_SCENE_SWITCH_GROUP_ENDPOINT 	"LightEndpoint"
#define ATTRIBUTE_NAME_FULL_SCENE_LIGHT_SWITCH 	"LightSwitch"
#define ATTRIBUTE_NAME_FULL_SCENE_LIGHT_SWITCH_1 	"LightSwitch_1"
#define ATTRIBUTE_NAME_FULL_SCENE_LIGHT_SWITCH_2 	"LightSwitch_2"
#define ATTRIBUTE_NAME_FULL_SCENE_LIGHT_SWITCH_3 	"LightSwitch_3"
#define ATTRIBUTE_NAME_FULL_SCENE_LIGHT_SWITCH_4 	"LightSwitch_4"

#define ATTRIBUTE_NAME_FULL_SCENE_CURTAIN_ENDPOINT 	"CurtainEndpoint"

#define ATTRIBUTE_NAME_FULL_SCENE_CURTAIN_ENABLE 	"CurtainEnable"
#define ATTRIBUTE_NAME_FULL_SCENE_SCENE_NAME 	"SceName"
#define ATTRIBUTE_NAME_FULL_SCENE_SCENE_PHOTO 	"ScePhoto"
#define ATTRIBUTE_NAME_FULL_SCENE_ENABLE 	"Enable"
#define ATTRIBUTE_NAME_FULL_SCENE_REFACTORY "Refactory"


/**************************属性键值对结构体********************************/
#define ATTRIBUTE_KEY_MAX_LENGTH		(40)
#define ATTRIBUTE_VALUE_MAX_LENGTH		(200)
#define ATTRIBUTE_UNIT_MAX_LENGTH		(20)

typedef struct Attribute
{
	char key[ATTRIBUTE_KEY_MAX_LENGTH];
	char value[ATTRIBUTE_VALUE_MAX_LENGTH];
	char unit[ATTRIBUTE_UNIT_MAX_LENGTH];
	struct timeval tv;
	struct Attribute *next;
}Attribute;


/****************************设备节点*****************************************/
#define DEVICE_DEVICE_ID_LENGTH				(20)
#define DEVICE_NAME_LENGTH					(200)
#define DEVICE_HARDWARE_VERSION_LENGTH		(40)
#define DEVICE_SOFTEWARE_VERSION_LENGTH		(40)
#define DEVICE_MODEL_ID_LENGTH				(60)
#define DEVICE_SECRET_LENGTH				(44)
#define DEVICE_VER_KEY_LENGTH				(20)
#define DEVICE_GROUP_ID_LEN					(40)


/*设备在/离线节点结构体*/
typedef struct DeviceOnlineInformantion
{
	char deviceId[DEVICE_DEVICE_ID_LENGTH];		/*设备地址*/
	int online;									/*0-离线或断电，1-在线*/
	int offCount;								/*离线次数，3次时上报子设备离线*/
	int leaveGateway;							/*1在网，0离网*/
	struct timeval lastUpdateTime;				/*最后一次上报更新时间*/
	int reportTime;
	struct DeviceOnlineInformantion *next;
}DevOnlineInfo;


/*设备信息节点结构体*/
typedef struct DeviceInformantion
{
	char deviceId[DEVICE_DEVICE_ID_LENGTH];		/*设备地址*/
	//char devType[8];							/*rex设备类型*/
	int subDevType;								/*设备类型，参看RexGateway SDK开发技术说明文档*/
	char name[DEVICE_NAME_LENGTH];				/*设备名称,英文*/
	char modelId[DEVICE_MODEL_ID_LENGTH];							/*标准HA产品系列不同的区分*/
	char secret[DEVICE_SECRET_LENGTH];			/*设备唯一签名*/
	//unsigned short reportTime;					/*上报周期*/
	char hardwareVersion[DEVICE_HARDWARE_VERSION_LENGTH]; /*硬件版本*/
	char softwareVersion[DEVICE_SOFTEWARE_VERSION_LENGTH]; /*软件版本*/
	char verKey[DEVICE_VER_KEY_LENGTH];
	int registerStatus;							/*0 没有注册，1 已经注册*/
	DevOnlineInfo *onlineInfo;					/*在离线信息节点指针*/
	Attribute *attrHead;						/*属性链表头节点结构体指针*/
	int attrNum;								/*属性个数*/

	char realModelId[DEVICE_MODEL_ID_LENGTH];
	struct DeviceInformantion  *next;			/*下一节点指针*/
}DevInfo;

#endif 
