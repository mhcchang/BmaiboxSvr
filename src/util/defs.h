#pragma once
#ifdef UNICODE
#undef UNICODE
#endif

#define __LINUX_OS__ 1
//#define __WINDOWS_OS__ 0
#include <string.h>
#include <string>
#include <map>
#include <vector>

#define SVR_NAME "WsMqSvr.json"
#define C_MAXCHANNEL 4

#define API_NUM 27

#define MHC_STLNOW_SECOND  std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count()
#define MHC_STLNOW_MILLISEC  std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count()
#define MHC_STLNOW_MICROSEC  std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count()

enum MsgType {
	MsgType_unknown = 0,
	MsgType_login = 1,
	MsgType_logout,
	MsgType_setPassword,
	MsgType_getboxId,
	MsgType_setboxId, //5
	MsgType_getSysTime,
	MsgType_setSysTime,
	//MsgType_setSysNtp,
	MsgType_reboot,
	MsgType_getBasicInfo,
	MsgType_softUpdate, //10
	MsgType_algoUpdate,
	MsgType_getNetwork,
	MsgType_setNetwork,
	MsgType_getMqtt,
	MsgType_setMqtt, //15
	//MsgType_checkPassword = 15,

	//MsgType_getDeviceStatus,
	MsgType_addCamera,
	MsgType_delCamera,
	MsgType_getCameras,
	MsgType_addRoi,
	MsgType_delRoi, //20
	MsgType_getRoi,
	MsgType_getSnapshot,
	MsgType_setArmRule,
	MsgType_getArmRule,
	MsgType_setAlgoRule,
	MsgType_getAlgoRule,
	MsgType_count,
	MsgType_query,
	//MsgType_alarm,
	//MsgType_channel,
	//MsgType_aiBox
};

struct stru_WsHead {
	std::string token;
	int retn;
	std::string info;
	std::string type;

	stru_WsHead() : retn(0) {};
};

struct BaseInf {
	std::string boxId;
	std::string model;
	std::string snbr;
	std::string softVer;
	std::string algoVer;
	std::string mac;
};

struct NetworkInf {
	std::string ip;
	std::string mask;
	std::string gateway;
	std::string dns;
};

struct MqttInf {
	std::string ip;
	std::string port;
	std::string user;
	std::string password;
	std::string topic;
};

struct DeviceStatus {
	//"T":36.20, "CPU" : 64.66, "Ram" : 2048, "RamUsed" : 1024, 
	//"Disk" : 8192, "DiskUsed" : 1024, "sysTime" : "2022-01-01 10:00:00", "runTime"
	float temp;
	float cpuTemp;
	uint64_t ram;
	uint64_t ramused;
	uint64_t disk;
	uint64_t diskUsed;
	std::string sysTime;
	uint64_t runTime;
};

struct CameraInf {
	//"channel":0 / 1 / 2 / 3, "ptzType" : 1, "imgType" : 1, "rtspUrl" :
	std::string channel;
	int ptzType;
	int imgType;
	std::string rtspUrl;
};

struct Point {
	float x1;
	float y1;
};

struct RoiInf {
	std::string channel;
	int roiId;

	std::vector<Point> pts;
};

struct CtlInf {
	std::string boxId;
	std::string sysTime;
	std::string sysNtp;

	BaseInf baseInf;
	NetworkInf networkInf;
	MqttInf mqttInf;
	DeviceStatus deviceStatus;
	CameraInf cameraInf;
	RoiInf roiInf;

	std::string reserve;
};

//server receive info /client send
struct stru_WsSvrRecv {
	stru_WsHead head;
	MsgType msgType;
	CtlInf ctlinf;
};

typedef stru_WsSvrRecv stru_WsCtlSend;
typedef stru_WsSvrRecv stru_WsSvrSend;
typedef stru_WsSvrSend stru_WsCtlRecv;

#ifndef WSMQ_NAMESPACE_BEGIN
#define WSMQ_NAMESPACE_BEGIN namespace WsMqSvr { 
#define WSMQ_NAMESPACE_END } 
#endif
