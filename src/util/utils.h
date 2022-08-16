#pragma once

#if __WINDOWS_OS__
#pragma warning (disable: 4251 4244)
#else
#define _access access

#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#pragma GCC diagnostic ignored "-Wreorder"
#endif

#ifndef SENTRYSVR_NAMESPACE_BEGIN
#define SENTRYSVR_NAMESPACE_BEGIN namespace cvcam { namespace SentrySvr { 
#define SENTRYSVR_NAMESPACE_END } }
#endif

#ifndef MULTIDOME_NAMESPACE_BEGIN
#define MULTIDOME_NAMESPACE_BEGIN namespace cvcam { namespace MultiDome { 
#define MULTIDOME_NAMESPACE_END } }
#endif

//#include "sys_inc.h"
#include <sys/timeb.h>
#include "rapidjson.h"
#include "document.h"
#include <vector>
#include "defs.h"

#define C_MAX_CAMERA (18)

enum ALARM_FROM {
	MSG_FROM_WS = 0x00, //websocket
	MSG_FROM_MQ = 0x01, //消息队列
	MSG_FROM_WESTSTATION = 0x02 //北京西站特例
};

extern const std::vector<std::string> MsgTypeChar;
extern const std::vector<std::string> c_log_level_str;

std::string UTF8ToGBK(const char* strUTF8);
std::string GBKToUTF8(const char* strGBK);

time_t StringToTimet(std::string str);
tm StringToTm(std::string str);

void changeWorkingDirectory();

inline bool PtInRect(double x1, double y1, double x2, double y2, double x, double y)
{
	if (x1 < x && x < x2 && y1 < y && y < y2)
		return true;
	else
		return false;
}

inline int SetLogLevelStr(const char* levelstr)
{
//	const LogLevel OFF_LOG_LEVEL     = 60000;
//const LogLevel FATAL_LOG_LEVEL = 50000;
//const LogLevel ERROR_LOG_LEVEL = 40000;
//const LogLevel WARN_LOG_LEVEL = 30000;
//const LogLevel INFO_LOG_LEVEL = 20000;
//const LogLevel DEBUG_LOG_LEVEL = 10000;
//const LogLevel TRACE_LOG_LEVEL = 0;
	int log_level = 3;
	for (int ii = 0; ii < 7; ii++)
	{
		if (strcmp(levelstr, c_log_level_str[ii].c_str()) == 0)
		{
			log_level = ii * 10000;
			return log_level;
		}
	}

	return log_level;
}

void BinToHexStr(unsigned char * Bin, int nlen, unsigned char * Hex);
bool HexStrToBin(char * str, unsigned char * Bin);

//从Linux程序中执行shell（程序、脚本）并获得输出结果（转）
int GetShellRes(char* szCmd, std::stringstream &sstream);

int SetSystemTime(char *dt);

bool powerOffProc();
//注销
bool logOffProc();
//重启
bool reBootProc();
void WriteReportEvent(char* szName, char* szFunction);
//根据域名获取ip
int get_ip_by_domain(const char *domain, char *ip);

//获取本机mac
int get_local_mac(const char *eth_inf, char *mac);
// 获取本机ip
int get_local_ip(const char *eth_inf, char *ip);

//int getifInfo(char* ifr_name, char* addr, char* mask, char* mac, char* states);

//int SetIfAddr(char *ifname, char *Ipaddr, char *mask, char *gateway);
//int set_gateway(unsigned long gw);
//int get_gateway(unsigned long *p, char* gateway);

//int SetDns(char *dns);
//int GetDns(char *dns);

//cpu
double get_sysCpuUsage();

int IsDir(std::string path);

void CopyFile(std::string sourcePath, std::string destPath);

void CopyFolder(std::string sourcePath, std::string destPath);
std::string GetSysFormatTime();
