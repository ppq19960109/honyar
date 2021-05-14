#include <stdio.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "cJSON.h"
#include "log.h"
#include "error_no.h"
#include "json_key.h"
#include "len.h"

#define TEST_DEV_NUMBER	52
#define TEST_EP_NUMBER	92

#define TEST_TIME	50

extern int g_iClientFd;

typedef struct statue_info_s
{
	char acDeviceId[DEV_ID_MAX_LEN];
	char acKey[KEY_MAX_LEN];
	char acValue[VALUE_MAX_LEN];
	
	int iFlag;
}statue_info_t;

statue_info_t statue_all[TEST_EP_NUMBER] = {
{"000D6F000F7140CE", "SwitchMultiple_1", "", 0},
{"000D6F000F7140CE", "SwitchMultiple_2", "", 0},
{"000D6F000F71373E", "SwitchMultiple_1", "", 0},
{"000D6F000F71373E", "SwitchMultiple_2", "", 0},
{"000D6F000F717506", "SwitchMultiple_1", "", 0},
{"000D6F000F717506", "SwitchMultiple_2", "", 0},
{"000D6F000FA02CB8", "SwitchMultiple_1", "", 0},
{"000D6F000FA02CB8", "SwitchMultiple_2", "", 0},
{"000D6F000F716BDA", "SwitchMultiple_1", "", 0},
{"000D6F000F716BDA", "SwitchMultiple_2", "", 0},
{"000D6F000F7161AE", "SwitchMultiple_1", "", 0},
{"000D6F000F7161AE", "SwitchMultiple_2", "", 0},
{"000D6F000F6D7B20", "SwitchMultiple_1", "", 0},
{"000D6F000F6D7B20", "SwitchMultiple_2", "", 0},
{"000D6F000F713F16", "SwitchMultiple_1", "", 0},
{"000D6F000F713F16", "SwitchMultiple_2", "", 0},
{"000D6F000F715B7D", "SwitchMultiple_1", "", 0},
{"000D6F000F715B7D", "SwitchMultiple_2", "", 0},
{"000D6F000F715E6D", "SwitchMultiple_1", "", 0},
{"000D6F000F715E6D", "SwitchMultiple_2", "", 0},
{"000D6F0012A08B2E", "SwitchMultiple_1", "", 0},
{"000D6F0012A08B2E", "SwitchMultiple_2", "", 0},
{"000D6F0012A087B2", "SwitchMultiple_1", "", 0},
{"000D6F0012A087B2", "SwitchMultiple_2", "", 0},
//{"000D6F0010FEDEFB", "Switch", "", 0},
{"000D6F0010FF4FE2", "Switch", "", 0},
{"000D6F0010FEED87", "Switch", "", 0},
{"000D6F001236CDD4", "Switch", "", 0},
{"000D6F001222983C", "Switch", "", 0},
{"000D6F000E4E6A0B", "Switch", "", 0},
{"000D6F000E4E4055", "Switch", "", 0},
{"000D6F0010FF4252", "Switch", "", 0},
{"000D6F000E4E4333", "Switch", "", 0},
{"000D6F0010FEE12E", "Switch", "", 0},
{"000D6F0010FF198D", "Switch_1", "", 0},
{"000D6F0010FF198D", "Switch_2", "", 0},
{"000D6F0010FF198D", "Switch_3", "", 0},
{"000D6F0010FF418B", "Switch_1", "", 0},
{"000D6F0010FF418B", "Switch_2", "", 0},
{"000D6F0010FF418B", "Switch_3", "", 0},
{"000D6F001100AF3A", "Switch_1", "", 0},
{"000D6F001100AF3A", "Switch_2", "", 0},
{"000D6F001100AF3A", "Switch_3", "", 0},
{"000D6F0010FF31C0", "Switch_1", "", 0},
{"000D6F0010FF31C0", "Switch_2", "", 0},
{"000D6F0010FF31C0", "Switch_3", "", 0},
{"000D6F0010FED3F9", "Switch_1", "", 0},
{"000D6F0010FED3F9", "Switch_2", "", 0},
{"000D6F0010FED3F9", "Switch_3", "", 0},
{"000D6F0010FF04A8", "Switch_1", "", 0},
{"000D6F0010FF04A8", "Switch_2", "", 0},
{"000D6F0010FF04A8", "Switch_3", "", 0},
{"000D6F0010FF3F17", "Switch_1", "", 0},
{"000D6F0010FF3F17", "Switch_2", "", 0},
{"000D6F0010FF3F17", "Switch_3", "", 0},
{"000D6F0010FF1FEA", "Switch_1", "", 0},
{"000D6F0010FF1FEA", "Switch_2", "", 0},
{"000D6F0010FF1FEA", "Switch_3", "", 0},
{"000D6F0010FF2E5D", "Switch_1", "", 0},
{"000D6F0010FF2E5D", "Switch_2", "", 0},
{"000D6F0010FF2E5D", "Switch_3", "", 0},
{"000D6F0010FF2D6A", "Switch_1", "", 0},
{"000D6F0010FF2D6A", "Switch_2", "", 0},
{"000D6F0010FF2D6A", "Switch_3", "", 0},
//{"000D6F001102B2E1", "Switch_1", "", 0},
{"000D6F001102B2E1", "Switch_2", "", 0},
{"000D6F0010FEC709", "Switch_1", "", 0},
{"000D6F0010FEC709", "Switch_2", "", 0},
{"000D6F001102E440", "Switch_1", "", 0},
{"000D6F001102E440", "Switch_2", "", 0},
{"000D6F0010FF4019", "Switch_1", "", 0},
{"000D6F0010FF4019", "Switch_2", "", 0},
{"000D6F001102E1D7", "Switch_1", "", 0},
{"000D6F001102E1D7", "Switch_2", "", 0},
{"000D6F0010FF57E1", "Switch_1", "", 0},
{"000D6F0010FF57E1", "Switch_2", "", 0},
{"000D6F0011002690", "Switch_1", "", 0},
{"000D6F0011002690", "Switch_2", "", 0},
{"000D6F001100305E", "Switch_1", "", 0},
{"000D6F001100305E", "Switch_2", "", 0},
{"000D6F0011002EEA", "Switch_1", "", 0},
{"000D6F0011002EEA", "Switch_2", "", 0},
{"000D6F0011003382", "Switch_1", "", 0},
{"000D6F0011003382", "Switch_2", "", 0},
{"000D6F0012228F56", "Switch", "", 0},
{"000D6F001222EB62", "Switch", "", 0},
{"000D6F001226327C", "Switch", "", 0},
{"000D6F001225E19D", "Switch", "", 0},
{"000D6F00122278B5", "Switch", "", 0},
{"000D6F0012272701", "Switch", "", 0},
{"000D6F001236BDCF", "Switch", "", 0},
{"000D6F001222B69A", "Switch", "", 0},
{"000D6F00122257EC", "Switch", "", 0},
{"000D6F001225DC2E", "Switch", "", 0}
};
char open_all[TEST_EP_NUMBER][512] = {
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F000F7140CE\",\"Key\":\"SwitchMultiple_1\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F000F7140CE\",\"Key\":\"SwitchMultiple_2\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F000F71373E\",\"Key\":\"SwitchMultiple_1\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F000F71373E\",\"Key\":\"SwitchMultiple_2\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F000F717506\",\"Key\":\"SwitchMultiple_1\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F000F717506\",\"Key\":\"SwitchMultiple_2\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F000FA02CB8\",\"Key\":\"SwitchMultiple_1\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F000FA02CB8\",\"Key\":\"SwitchMultiple_2\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F000F716BDA\",\"Key\":\"SwitchMultiple_1\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F000F716BDA\",\"Key\":\"SwitchMultiple_2\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F000F7161AE\",\"Key\":\"SwitchMultiple_1\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F000F7161AE\",\"Key\":\"SwitchMultiple_2\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F000F6D7B20\",\"Key\":\"SwitchMultiple_1\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F000F6D7B20\",\"Key\":\"SwitchMultiple_2\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F000F713F16\",\"Key\":\"SwitchMultiple_1\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F000F713F16\",\"Key\":\"SwitchMultiple_2\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F000F715B7D\",\"Key\":\"SwitchMultiple_1\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F000F715B7D\",\"Key\":\"SwitchMultiple_2\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F000F715E6D\",\"Key\":\"SwitchMultiple_1\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F000F715E6D\",\"Key\":\"SwitchMultiple_2\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0012A08B2E\",\"Key\":\"SwitchMultiple_1\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0012A08B2E\",\"Key\":\"SwitchMultiple_2\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0012A087B2\",\"Key\":\"SwitchMultiple_1\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0012A087B2\",\"Key\":\"SwitchMultiple_2\",\"Value\":\"1\"}]}",
//"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FEDEFB\",\"Key\":\"Switch\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FF4FE2\",\"Key\":\"Switch\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FEED87\",\"Key\":\"Switch\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F001236CDD4\",\"Key\":\"Switch\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F001222983C\",\"Key\":\"Switch\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F000E4E6A0B\",\"Key\":\"Switch\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F000E4E4055\",\"Key\":\"Switch\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FF4252\",\"Key\":\"Switch\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F000E4E4333\",\"Key\":\"Switch\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FEE12E\",\"Key\":\"Switch\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FF198D\",\"Key\":\"Switch_1\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FF198D\",\"Key\":\"Switch_2\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FF198D\",\"Key\":\"Switch_3\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FF418B\",\"Key\":\"Switch_1\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FF418B\",\"Key\":\"Switch_2\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FF418B\",\"Key\":\"Switch_3\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F001100AF3A\",\"Key\":\"Switch_1\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F001100AF3A\",\"Key\":\"Switch_2\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F001100AF3A\",\"Key\":\"Switch_3\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FF31C0\",\"Key\":\"Switch_1\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FF31C0\",\"Key\":\"Switch_2\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FF31C0\",\"Key\":\"Switch_3\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FED3F9\",\"Key\":\"Switch_1\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FED3F9\",\"Key\":\"Switch_2\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FED3F9\",\"Key\":\"Switch_3\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FF04A8\",\"Key\":\"Switch_1\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FF04A8\",\"Key\":\"Switch_2\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FF04A8\",\"Key\":\"Switch_3\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FF3F17\",\"Key\":\"Switch_1\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FF3F17\",\"Key\":\"Switch_2\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FF3F17\",\"Key\":\"Switch_3\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FF1FEA\",\"Key\":\"Switch_1\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FF1FEA\",\"Key\":\"Switch_2\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FF1FEA\",\"Key\":\"Switch_3\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FF2E5D\",\"Key\":\"Switch_1\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FF2E5D\",\"Key\":\"Switch_2\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FF2E5D\",\"Key\":\"Switch_3\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FF2D6A\",\"Key\":\"Switch_1\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FF2D6A\",\"Key\":\"Switch_2\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FF2D6A\",\"Key\":\"Switch_3\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F001102B2E1\",\"Key\":\"Switch_1\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F001102B2E1\",\"Key\":\"Switch_2\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FEC709\",\"Key\":\"Switch_1\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FEC709\",\"Key\":\"Switch_2\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F001102E440\",\"Key\":\"Switch_1\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F001102E440\",\"Key\":\"Switch_2\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FF4019\",\"Key\":\"Switch_1\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FF4019\",\"Key\":\"Switch_2\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F001102E1D7\",\"Key\":\"Switch_1\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F001102E1D7\",\"Key\":\"Switch_2\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FF57E1\",\"Key\":\"Switch_1\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FF57E1\",\"Key\":\"Switch_2\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0011002690\",\"Key\":\"Switch_1\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0011002690\",\"Key\":\"Switch_2\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F001100305E\",\"Key\":\"Switch_1\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F001100305E\",\"Key\":\"Switch_2\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0011002EEA\",\"Key\":\"Switch_1\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0011002EEA\",\"Key\":\"Switch_2\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0011003382\",\"Key\":\"Switch_1\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0011003382\",\"Key\":\"Switch_2\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0012228F56\",\"Key\":\"Switch\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F001222EB62\",\"Key\":\"Switch\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F001226327C\",\"Key\":\"Switch\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F001225E19D\",\"Key\":\"Switch\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F00122278B5\",\"Key\":\"Switch\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0012272701\",\"Key\":\"Switch\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F001236BDCF\",\"Key\":\"Switch\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F001222B69A\",\"Key\":\"Switch\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F00122257EC\",\"Key\":\"Switch\",\"Value\":\"1\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F001225DC2E\",\"Key\":\"Switch\",\"Value\":\"1\"}]}"
};


char close_all[TEST_EP_NUMBER][512] = {
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F000F7140CE\",\"Key\":\"SwitchMultiple_1\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F000F7140CE\",\"Key\":\"SwitchMultiple_2\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F000F71373E\",\"Key\":\"SwitchMultiple_1\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F000F71373E\",\"Key\":\"SwitchMultiple_2\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F000F717506\",\"Key\":\"SwitchMultiple_1\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F000F717506\",\"Key\":\"SwitchMultiple_2\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F000FA02CB8\",\"Key\":\"SwitchMultiple_1\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F000FA02CB8\",\"Key\":\"SwitchMultiple_2\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F000F716BDA\",\"Key\":\"SwitchMultiple_1\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F000F716BDA\",\"Key\":\"SwitchMultiple_2\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F000F7161AE\",\"Key\":\"SwitchMultiple_1\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F000F7161AE\",\"Key\":\"SwitchMultiple_2\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F000F6D7B20\",\"Key\":\"SwitchMultiple_1\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F000F6D7B20\",\"Key\":\"SwitchMultiple_2\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F000F713F16\",\"Key\":\"SwitchMultiple_1\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F000F713F16\",\"Key\":\"SwitchMultiple_2\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F000F715B7D\",\"Key\":\"SwitchMultiple_1\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F000F715B7D\",\"Key\":\"SwitchMultiple_2\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F000F715E6D\",\"Key\":\"SwitchMultiple_1\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F000F715E6D\",\"Key\":\"SwitchMultiple_2\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0012A08B2E\",\"Key\":\"SwitchMultiple_1\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0012A08B2E\",\"Key\":\"SwitchMultiple_2\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0012A087B2\",\"Key\":\"SwitchMultiple_1\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0012A087B2\",\"Key\":\"SwitchMultiple_2\",\"Value\":\"0\"}]}",
//"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FEDEFB\",\"Key\":\"Switch\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FF4FE2\",\"Key\":\"Switch\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FEED87\",\"Key\":\"Switch\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F001236CDD4\",\"Key\":\"Switch\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F001222983C\",\"Key\":\"Switch\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F000E4E6A0B\",\"Key\":\"Switch\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F000E4E4055\",\"Key\":\"Switch\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FF4252\",\"Key\":\"Switch\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F000E4E4333\",\"Key\":\"Switch\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FEE12E\",\"Key\":\"Switch\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FF198D\",\"Key\":\"Switch_1\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FF198D\",\"Key\":\"Switch_2\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FF198D\",\"Key\":\"Switch_3\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FF418B\",\"Key\":\"Switch_1\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FF418B\",\"Key\":\"Switch_2\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FF418B\",\"Key\":\"Switch_3\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F001100AF3A\",\"Key\":\"Switch_1\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F001100AF3A\",\"Key\":\"Switch_2\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F001100AF3A\",\"Key\":\"Switch_3\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FF31C0\",\"Key\":\"Switch_1\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FF31C0\",\"Key\":\"Switch_2\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FF31C0\",\"Key\":\"Switch_3\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FED3F9\",\"Key\":\"Switch_1\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FED3F9\",\"Key\":\"Switch_2\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FED3F9\",\"Key\":\"Switch_3\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FF04A8\",\"Key\":\"Switch_1\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FF04A8\",\"Key\":\"Switch_2\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FF04A8\",\"Key\":\"Switch_3\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FF3F17\",\"Key\":\"Switch_1\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FF3F17\",\"Key\":\"Switch_2\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FF3F17\",\"Key\":\"Switch_3\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FF1FEA\",\"Key\":\"Switch_1\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FF1FEA\",\"Key\":\"Switch_2\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FF1FEA\",\"Key\":\"Switch_3\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FF2E5D\",\"Key\":\"Switch_1\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FF2E5D\",\"Key\":\"Switch_2\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FF2E5D\",\"Key\":\"Switch_3\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FF2D6A\",\"Key\":\"Switch_1\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FF2D6A\",\"Key\":\"Switch_2\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FF2D6A\",\"Key\":\"Switch_3\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F001102B2E1\",\"Key\":\"Switch_1\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F001102B2E1\",\"Key\":\"Switch_2\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FEC709\",\"Key\":\"Switch_1\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FEC709\",\"Key\":\"Switch_2\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F001102E440\",\"Key\":\"Switch_1\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F001102E440\",\"Key\":\"Switch_2\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FF4019\",\"Key\":\"Switch_1\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FF4019\",\"Key\":\"Switch_2\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F001102E1D7\",\"Key\":\"Switch_1\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F001102E1D7\",\"Key\":\"Switch_2\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FF57E1\",\"Key\":\"Switch_1\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FF57E1\",\"Key\":\"Switch_2\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0011002690\",\"Key\":\"Switch_1\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0011002690\",\"Key\":\"Switch_2\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F001100305E\",\"Key\":\"Switch_1\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F001100305E\",\"Key\":\"Switch_2\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0011002EEA\",\"Key\":\"Switch_1\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0011002EEA\",\"Key\":\"Switch_2\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0011003382\",\"Key\":\"Switch_1\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0011003382\",\"Key\":\"Switch_2\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0012228F56\",\"Key\":\"Switch\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F001222EB62\",\"Key\":\"Switch\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F001226327C\",\"Key\":\"Switch\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F001225E19D\",\"Key\":\"Switch\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F00122278B5\",\"Key\":\"Switch\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0012272701\",\"Key\":\"Switch\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F001236BDCF\",\"Key\":\"Switch\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F001222B69A\",\"Key\":\"Switch\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F00122257EC\",\"Key\":\"Switch\",\"Value\":\"0\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F001225DC2E\",\"Key\":\"Switch\",\"Value\":\"0\"}]}"
};


char get_true_all[TEST_DEV_NUMBER][512] = {
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F000F7140CE\",\"Key\":\"GetStatus\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F000F71373E\",\"Key\":\"GetStatus\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F000F717506\",\"Key\":\"GetStatus\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F000FA02CB8\",\"Key\":\"GetStatus\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F000F716BDA\",\"Key\":\"GetStatus\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F000F7161AE\",\"Key\":\"GetStatus\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F000F6D7B20\",\"Key\":\"GetStatus\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F000F713F16\",\"Key\":\"GetStatus\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F000F715B7D\",\"Key\":\"GetStatus\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F000F715E6D\",\"Key\":\"GetStatus\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0012A08B2E\",\"Key\":\"GetStatus\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0012A087B2\",\"Key\":\"GetStatus\"}]}",
//"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FEDEFB\",\"Key\":\"GetStatus\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FF4FE2\",\"Key\":\"GetStatus\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FEED87\",\"Key\":\"GetStatus\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F001236CDD4\",\"Key\":\"GetStatus\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F001222983C\",\"Key\":\"GetStatus\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F000E4E6A0B\",\"Key\":\"GetStatus\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F000E4E4055\",\"Key\":\"GetStatus\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FF4252\",\"Key\":\"GetStatus\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F000E4E4333\",\"Key\":\"GetStatus\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FEE12E\",\"Key\":\"GetStatus\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FF198D\",\"Key\":\"GetStatus\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FF418B\",\"Key\":\"GetStatus\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F001100AF3A\",\"Key\":\"GetStatus\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FF31C0\",\"Key\":\"GetStatus\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FED3F9\",\"Key\":\"GetStatus\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FF04A8\",\"Key\":\"GetStatus\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FF3F17\",\"Key\":\"GetStatus\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FF1FEA\",\"Key\":\"GetStatus\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FF2E5D\",\"Key\":\"GetStatus\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FF2D6A\",\"Key\":\"GetStatus\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F001102B2E1\",\"Key\":\"GetStatus\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FEC709\",\"Key\":\"GetStatus\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F001102E440\",\"Key\":\"GetStatus\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FF4019\",\"Key\":\"GetStatus\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F001102E1D7\",\"Key\":\"GetStatus\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0010FF57E1\",\"Key\":\"GetStatus\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0011002690\",\"Key\":\"GetStatus\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F001100305E\",\"Key\":\"GetStatus\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0011002EEA\",\"Key\":\"GetStatus\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0011003382\",\"Key\":\"GetStatus\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0012228F56\",\"Key\":\"GetStatus\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F001222EB62\",\"Key\":\"GetStatus\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F001226327C\",\"Key\":\"GetStatus\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F001225E19D\",\"Key\":\"GetStatus\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F00122278B5\",\"Key\":\"GetStatus\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F0012272701\",\"Key\":\"GetStatus\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F001236BDCF\",\"Key\":\"GetStatus\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F001222B69A\",\"Key\":\"GetStatus\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F00122257EC\",\"Key\":\"GetStatus\"}]}",
"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F001225DC2E\",\"Key\":\"GetStatus\"}]}"
};



/*新风测试*/

char xinfengceshi_on[4][512] = {
	"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F000ABF9FD6\",\"Key\":\"Switch\",\"Value\":\"1\"}]}",
	"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F000ABF9FD6\",\"Key\":\"WindSpeed\",\"Value\":\"2\"}]}",
	"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F000ABF9FD6\",\"Key\":\"WorkMode\",\"Value\":\"1\"}]}",
	"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F000ABF9FD6\",\"Key\":\"Temperature\",\"Value\":\"16.00\"}]}"
};
char xinfengceshi_off[4][512] = {
	"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F000ABF9FD6\",\"Key\":\"Switch\",\"Value\":\"0\"}]}",
	"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F000ABF9FD6\",\"Key\":\"WindSpeed\",\"Value\":\"4\"}]}",
	"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F000ABF9FD6\",\"Key\":\"WorkMode\",\"Value\":\"2\"}]}",
	"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F000ABF9FD6\",\"Key\":\"Temperature\",\"Value\":\"32.00\"}]}"
};

char xinfengceshi_2_on[4][512] = {
	"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F000F715A82\",\"Key\":\"Switch\",\"Value\":\"1\"}]}",
	"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F000F715A82\",\"Key\":\"WindSpeed\",\"Value\":\"2\"}]}",
	"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F000F715A82\",\"Key\":\"WorkMode\",\"Value\":\"1\"}]}",
	"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F000F715A82\",\"Key\":\"Temperature\",\"Value\":\"16.00\"}]}"
};
char xinfengceshi_2_off[4][512] = {
	"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F000F715A82\",\"Key\":\"Switch\",\"Value\":\"0\"}]}",
	"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F000F715A82\",\"Key\":\"WindSpeed\",\"Value\":\"4\"}]}",
	"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F000F715A82\",\"Key\":\"WorkMode\",\"Value\":\"2\"}]}",
	"{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F000F715A82\",\"Key\":\"Temperature\",\"Value\":\"32.00\"}]}"
};

char scene_on[512] = "{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F001102B2E1\",\"Key\":\"Switch_1\",\"Value\":\"1\"}]}";

char scene_off[512] = "{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F001102B2E1\",\"Key\":\"Switch_1\",\"Value\":\"0\"}]}";


/*12V开关器测试*/

char switch12v_on[512] = "{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F000E8A7CAF\",\"Key\":\"CurtainOperation\",\"Value\":\"1\"}]}";

char switch12v_off[512] = "{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"000D6F000E8A7CAF\",\"Key\":\"CurtainOperation\",\"Value\":\"0\"}]}";

void switch12v_on_test(void)
{
	LOG(LOG_DEBUG, "switch12v_on_test\n");
	int i = 0, j = 0;
	int iRet = 0;

	unsigned int send_count = 0;
	unsigned int send_error_count = 0;
	unsigned int error_count = 0;

	for(j = 0; j < TEST_TIME; ++j)
	{
		/*全开*/
		printf("switch12v_on\n");
		printf("Crtl: %s\n", switch12v_on);
		if(g_iClientFd > 0)
		{
			iRet = send(g_iClientFd, switch12v_on, strlen(switch12v_on) + 1, 0);
		}
		if(iRet > 0)
		{
			send_count += 1;
		}
		else
		{
			send_error_count += 1;
		}
		sleep(15);
		printf("switch12v_off\n");
		/*全关*/
		printf("Crtl: %s\n", switch12v_off);
		if(g_iClientFd > 0)
		{
			iRet = send(g_iClientFd, switch12v_off, strlen(switch12v_off) + 1, 0);
		}
		if(iRet > 0)
		{
			send_count += 1;
		}
		else
		{
			send_error_count += 1;
		}
		sleep(15);
		
		LOG(LOG_DEBUG, "switch12v_on_test send_count = %d, send_error_count = %d\n", send_count, send_error_count);
	}
}

void scene_on_test(void)
{
	LOG(LOG_DEBUG, "scene_on_test\n");
	int i = 0, j = 0;
	int iRet = 0;

	unsigned int send_count = 0;
	unsigned int send_error_count = 0;
	unsigned int error_count = 0;

	for(j = 0; j < TEST_TIME; ++j)
	{
		/*全开*/
		printf("scene_on\n");
		printf("Crtl: %s\n", scene_on);
		if(g_iClientFd > 0)
		{
			iRet = send(g_iClientFd, scene_on, strlen(scene_on) + 1, 0);
		}
		if(iRet > 0)
		{
			send_count += TEST_EP_NUMBER;
		}
		else
		{
			send_error_count += TEST_EP_NUMBER;
		}
		
		sleep(60);
		
		printf("get_all\n");
		/*获取状态*/
		get_statue_all(200);
		for(i = 0; i < TEST_EP_NUMBER; ++i)
		{
			printf("1acState = %s\n", statue_all[i].acValue);
			if(!strcmp(statue_all[i].acValue, "0"))
			{
				LOG(LOG_DEBUG,"Ctrl error, DeviceId = %s, Key = %s, ctrl status = 1, true status = %s\n", statue_all[i].acDeviceId, statue_all[i].acKey, statue_all[i].acValue);
				//return;
				error_count++;
			}
		}
		//sleep(5);

		printf("scene_off\n");
		/*全关*/
		printf("Crtl: %s\n", scene_off);
		if(g_iClientFd > 0)
		{
			iRet = send(g_iClientFd, scene_off, strlen(scene_off) + 1, 0);
		}
		if(iRet > 0)
		{
			send_count += TEST_EP_NUMBER;
		}
		else
		{
			send_error_count += TEST_EP_NUMBER;
		}

		sleep(60);
		
		printf("get_all\n");
		/*获取状态*/
		get_statue_all(200);
		for(i = 0; i < TEST_EP_NUMBER; ++i)
		{
			printf("0acState = %s\n", statue_all[i].acValue);
			if(!strcmp(statue_all[i].acValue, "1"))
			{
				LOG(LOG_DEBUG,"Ctrl error, DeviceId = %s, Key = %s, ctrl status = 0, true status = %s\n", statue_all[i].acDeviceId, statue_all[i].acKey, statue_all[i].acValue);
				//return;
				error_count++;
			}
		}
		//sleep(5);

		//if(0 ==  ((j + 1) % 10))
		{
			LOG(LOG_DEBUG, "scene_on_test send_count = %d, send_error_count = %d, error_count = %d\n", send_count, send_error_count, error_count);
		}

	}
}

void ctrl_test_xinfeng_on(int time)
{
	LOG(LOG_DEBUG, "ctrl_test_xinfeng_on_%dms\n",time);
	int i = 0, j = 0;
	int iRet = 0;

	unsigned int send_count = 0;
	unsigned int send_error_count = 0;
	unsigned int error_count = 0;


	/*全开*/
	for(i = 0; i < 4; ++i)
	{
		printf("Crtl: %s\n", xinfengceshi_2_on[i]);
		if(g_iClientFd > 0)
		{
			iRet = send(g_iClientFd, xinfengceshi_2_on[i], strlen(xinfengceshi_2_on[i]) + 1, 0);
		}
		if(iRet > 0)
		{
			send_count ++;
		}
		else
		{
			send_error_count ++;
		}
		usleep(time * 1000);
	}
}
void ctrl_test_xinfeng_off(int time)
{
	LOG(LOG_DEBUG, "ctrl_test_xinfeng_off_%dms\n",time);
	int i = 0, j = 0;
	int iRet = 0;

	unsigned int send_count = 0;
	unsigned int send_error_count = 0;
	unsigned int error_count = 0;

	/*全开*/
	for(i = 0; i < 4; ++i)
	{
		printf("Crtl: %s\n", xinfengceshi_2_off[i]);
		if(g_iClientFd > 0)
		{
			iRet = send(g_iClientFd, xinfengceshi_2_off[i], strlen(xinfengceshi_2_off[i]) + 1, 0);
		}
		if(iRet > 0)
		{
			send_count ++;
		}
		else
		{
			send_error_count ++;
		}
		usleep(time * 1000);
	}
}
/*控制下发测试*/
/*获取所有开关状态*/
void get_statue_all(int time)
{
	//LOG(LOG_DEBUG, "get_all\n");
	int i = 0;
	int iFlag = 0;
	for(i = 0; i < TEST_EP_NUMBER; ++i)
	{
		statue_all[i].iFlag = 0;
	}
	
	while(1)
	{
		
		for(i = 0; i < TEST_DEV_NUMBER; ++i)
		{
			if(g_iClientFd > 0)
			{
				printf("Get statue: %s\n", get_true_all[i]);
				send(g_iClientFd, get_true_all[i], strlen(get_true_all[i]) + 1, 0);
			}
			usleep(time * 1000);
		}
				
		iFlag = 0;
		for(i = 0; i < TEST_EP_NUMBER; ++i)
		{
			if(0 == statue_all[i].iFlag)
			{
				printf("get_statue_all error, DeviceId = %s, Key = %s, Flag = %d\n", statue_all[i].acDeviceId, statue_all[i].acKey, statue_all[i].iFlag);
				iFlag = 1;
				sleep(10);
				break;
			}
		}
		if(0 == iFlag)
		{
			break;
		}
	}
}

/*复位所有开关状态*/
void reset_all(void)
{
	LOG(LOG_DEBUG, "reset_all\n");
	int i = 0;
	int iFlag = 0;

	while(1)
	{
		iFlag = 0;
		
		/*全关*/
		for(i = 0; i < TEST_EP_NUMBER; ++i)
		{
			printf("Crtl: %s\n", close_all[i]);
			if(g_iClientFd > 0)
			{
				send(g_iClientFd, close_all[i], strlen(close_all[i]) + 1, 0);
			}
			usleep(550 * 1000);
		}
		sleep(5);
		
		/*获取状态*/
		get_statue_all(550);
		for(i = 0; i < TEST_EP_NUMBER; ++i)
		{
			printf("reset acState = %s\n", statue_all[i].acValue);
			if(!strcmp(statue_all[i].acValue, "1"))
			{
				//LOG(LOG_DEBUG,"Reset error, DeviceId = %s, Key = %s, ctrl status = 1, true status = %s\n", statue_all[i].acDeviceId, statue_all[i].acKey, statue_all[i].acValue);
				//return;
				iFlag = 1;
			}
		}
		sleep(5);
		if(0 == iFlag)
		{
			break;
		}
	}
	
}


void ctrl_test_once(int time)
{
	LOG(LOG_DEBUG, "ctrl_test_once_%dms\n",time);
	int i = 0, j = 0;
	int iRet = 0;

	unsigned int send_count = 0;
	unsigned int send_error_count = 0;
	unsigned int error_count = 0;

	for(j = 0; j < TEST_TIME; ++j)
	{
		/*全开*/
		printf("open_all\n");
		for(i = 0; i < TEST_EP_NUMBER; ++i)
		{
			printf("Crtl: %s\n", open_all[i]);
			if(g_iClientFd > 0)
			{
				iRet = send(g_iClientFd, open_all[i], strlen(open_all[i]) + 1, 0);
			}
			if(iRet > 0)
			{
				send_count ++;
			}
			else
			{
				send_error_count ++;
			}
			usleep(time * 1000);
		}

		//sleep(5);
		
		printf("get_all\n");
		/*获取状态*/
		get_statue_all(time);
		for(i = 0; i < TEST_EP_NUMBER; ++i)
		{
			printf("1acState = %s\n", statue_all[i].acValue);
			if(!strcmp(statue_all[i].acValue, "0"))
			{
				LOG(LOG_DEBUG,"Ctrl error, DeviceId = %s, Key = %s, ctrl status = 1, true status = %s\n", statue_all[i].acDeviceId, statue_all[i].acKey, statue_all[i].acValue);
				//return;
				error_count++;
			}
		}
		//sleep(5);

		printf("close_all\n");
		/*全关*/
		for(i = 0; i < TEST_EP_NUMBER; ++i)
		{
			printf("Crtl: %s\n", close_all[i]);
			if(g_iClientFd > 0)
			{
				iRet = send(g_iClientFd, close_all[i], strlen(close_all[i]) + 1, 0);
			}
			if(iRet > 0)
			if(0 == iRet)
			{
				send_count ++;
			}
			else
			{
				send_error_count ++;
			}
			usleep(time * 1000);
		}

		//sleep(5);
		
		printf("get_all\n");
		/*获取状态*/
		get_statue_all(time);
		for(i = 0; i < TEST_EP_NUMBER; ++i)
		{
			printf("0acState = %s\n", statue_all[i].acValue);
			if(!strcmp(statue_all[i].acValue, "1"))
			{
				LOG(LOG_DEBUG,"Ctrl error, DeviceId = %s, Key = %s, ctrl status = 0, true status = %s\n", statue_all[i].acDeviceId, statue_all[i].acKey, statue_all[i].acValue);
				//return;
				error_count++;
			}
		}
		//sleep(5);

		//if(0 ==  ((j + 1) % 10))
		{
			LOG(LOG_DEBUG, "ctrl_test_once_%dms send_count = %d, send_error_count = %d, error_count = %d\n", time, send_count, send_error_count, error_count);
		}

	}
}

int testReportExec(char *buff)
{
	/*解析返回结果*/
	int iValueType = 0xFF;
	cJSON *pstJson = NULL;
	cJSON *pstDataArry = NULL;
	cJSON *pstData = NULL;
	char acType[TYPE_MAX_LEN] = {0};
	char acDeviceId[DEV_ID_MAX_LEN] = {0};
	char acKey[KEY_MAX_LEN] = {0};
	char acState[VALUE_MAX_LEN] = {0};
	
	pstJson = cJSON_Parse(buff);
	if(NULL != pstJson)
	{
		if(ERROR_SUCCESS == JSON_value_get(
			JSON_KEY_TYPE, 
			acType, 
			TYPE_MAX_LEN, 
			NULL,
			&iValueType,
			pstJson)
		)
		{
			if(!strncmp(acType,
				JSON_VALUE_TYPE_ATTRIBUTE, 
				TYPE_MAX_LEN)
			)
			{
				/*属性上报指令*/
				if(ERROR_SUCCESS == JSON_value_get(
						JSON_KEY_DATA, 
						NULL, 
						0, 
						&pstDataArry, 
						&iValueType, 
						pstJson
					) &&
					cJSON_Array == iValueType
				)
				{
					int i = 0;
					int iCount = cJSON_GetArraySize(pstDataArry);
					for(i = 0; i < iCount; ++i)
					{
						if(NULL != (pstData = cJSON_GetArrayItem(pstDataArry,i)))
						{
							/*获取DeviceId*/
							if(ERROR_SUCCESS != JSON_value_get(
								JSON_KEY_DATA_DAVICE_ID, 
								acDeviceId, 
								DEV_ID_MAX_LEN, 
								NULL,
								&iValueType,
								pstData)
							)
							{
								continue;
							}
							/*获取Key*/
							if(ERROR_SUCCESS != JSON_value_get(
								JSON_KEY_DATA_KEY, 
								acKey, 
								KEY_MAX_LEN, 
								NULL,
								&iValueType,
								pstData)
							)
							{
								continue;
							}
							/*获取Value*/
							if(ERROR_SUCCESS != JSON_value_get(
								JSON_KEY_DATA_VALUE, 
								acState, 
								VALUE_MAX_LEN, 
								NULL,
								&iValueType,
								pstData)
							)
							{
								continue;
							}
							
							
							int j = 0;
							for(j = 0; j < TEST_EP_NUMBER; ++j)
							{
								if(!strcmp(statue_all[j].acDeviceId, acDeviceId) &&
								!strcmp(statue_all[j].acKey, acKey))
								{
									printf("Report DeviceId = %s, Key = %s, Value = %s\n", acDeviceId, acKey, acState);
									strcpy(statue_all[j].acValue, acState);
									statue_all[j].iFlag = 1;
								}
							}
						}
					}
				}
			}
		}
		
		cJSON_Delete(pstJson);
	}
	
	return 0;
}