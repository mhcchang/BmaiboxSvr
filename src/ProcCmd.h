#pragma once

#if __WINDOWS_OS__
#pragma warning (push)
#pragma warning (disable: 4251 4244)
#else
#define _access access
/////gunc
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#pragma GCC diagnostic ignored "-Wreorder"
#endif

#include "defs.h"
#include "utils.h"

#include <functional>
#include <string>
#include <atomic>
#include <mutex>
#include <vector>
#include "Config.h"
#include <document.h>
#include "stringbuffer.h"
#include "prettywriter.h"
#include <thread>

#include "MqSvr.h"
#include "HardDiskManager.h"
struct algParam {
	int channel;
	std::string createTime;
	int objectTypes;
	float confidence;
	float imageQuality;
	std::string snapshotUrl;
};

WSMQ_NAMESPACE_BEGIN

class ProcCmd
{
public:
	ProcCmd(Config * config);
	~ProcCmd();

	Config * m_config;

	struct FaultMsg {
		int type;
		float threshold;
	};

	struct DiskAlarm {
		char disk[256];
		double mB;
		double percent;
	};

public:
	bool GetBoxId(std::string &boxId);
	bool SetBoxId(std::string boxId);

	bool GetBaseInfo(Config::BaseParam &base);

	bool GetChipTemp(float &ft);
	bool GetBoradTemp(float &ft);
	bool GetPower(char * pwd, float &cput, float &gput);

	bool CheckCamera(int64_t &nOnline1, int64_t &nOnline2, int64_t &nOnline3, int64_t &nOnline4);
	bool CheckSystem(float &temperature, float &fcpu, float &ftpu, int &ram, int &ramUsed,
		int &disk, int &diskUsed, std::string &sysTime, int &runTime);

	int SetNetwork(char *ifname, const char* ip, const char* mask, const char* gateway, const char* dns);
	int GetNetwork(char *ifname, char* ip, char* mask, char* gateway, char* dns);

	bool GetCount(std::string path, int channel, int &count);
	bool GetAlgParam(std::string path, int channel, int pageNbr, int pageSize, std::vector<algParam> & params);

	int start();
	int stop();

public:
	volatile int64_t m_nOnline1;
	volatile int64_t m_nOnline2;
	volatile int64_t m_nOnline3;
	volatile int64_t m_nOnline4;
private:
	void WorkProcess();

	bool checkDisks(DiskAlarm & diskAlarm);
	void CheckFirst(bool bl);
	std::string GetAlarm(int ch, int status);
	std::string GetFault(int type, float threshold);

	void CheckSysAlarm(float temperature, float fcpu, float ftpu, int ram, int ramUsed,
		int disk, int diskUsed, std::string sysTime);
private:
	std::thread m_thread;

	std::atomic_bool m_blRunning;
	
	std::mutex m_mutex;
	uint64_t m_tmLastCamera[C_MAXCHANNEL];
	DiskAlarm m_diskAlarmParam;
	WHardDiskManager m_disk;

	MqSvr *m_mqSvr;
}; 

WSMQ_NAMESPACE_END

#if __WINDOWS_OS__
#pragma warning (pop)
#else
#pragma GCC diagnostic pop
#endif
