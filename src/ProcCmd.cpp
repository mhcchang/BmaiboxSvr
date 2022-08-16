#include "ProcCmd.h"
#include "Config.h"

#include <stdio.h>
#include <stdlib.h>

#include <regex>
#include <sstream>
#include <iostream>
#include <string>
#include <time.h>

#include "utils.h"
#if __WINDOWS_OS__
#else
#include <sys/time.h>
#include <unistd.h>

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/types.h>
#include<arpa/inet.h>
#include<net/if.h>
#include<string.h>
#include<signal.h>
#include<sys/wait.h>
#include<sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include<regex.h>

//#include <sys/sock.h>
//#include <linux/sock.h>

//#define SIOCGIFNAME	0x8910		/* get iface name		*/
//#define SIOCSIFLINK	0x8911		/* set iface channel		*/
//#define SIOCGIFCONF	0x8912		/* get iface list		*/
//#define SIOCGIFFLAGS	0x8913		/* get flags			*/
//#define SIOCSIFFLAGS	0x8914		/* set flags			*/
//#define SIOCGIFADDR	0x8915		/* get PA address		*/
//#define SIOCSIFADDR	0x8916		/* set PA address		*/
//#define SIOCGIFDSTADDR	0x8917		/* get remote PA address	*/
//#define SIOCSIFDSTADDR	0x8918		/* set remote PA address	*/
//#define SIOCGIFBRDADDR	0x8919		/* get broadcast PA address	*/
//#define SIOCSIFBRDADDR	0x891a		/* set broadcast PA address	*/
//#define SIOCGIFNETMASK	0x891b		/* get network PA mask		*/
//#define SIOCSIFNETMASK	0x891c		/* set network PA mask		*/

#endif
#include <time.h>
#include <sstream>
#include <regex>

#include "utils.h"
//int GetSystemTime()
//{
//	time_t timer;
//	struct tm* t_tm;
//	time(&timer);
//	t_tm = localtime(&timer);
//	printf("today is %4d%02d%02d%02d%02d%02d/n", t_tm->tm_year + 1900,
//		t_tm->tm_mon + 1, t_tm->tm_mday, t_tm->tm_hour, t_tm->tm_min, t_tm->tm_sec);
//	return 0;
//}
//
///************************************************
//	char *pt="2006-4-20 20:30:30";
//	SetSystemTime(pt);
//**************************************************/
//int SetSystemTime(char *dt)
//{
//	struct tm rtctm;
//	struct tm _tm;
//	struct timeval tv;
//	time_t timep;
//	sscanf(dt, "%d-%d-%d %d:%d:%d", &rtctm.tm_year,
//		&rtctm.tm_mon, &rtctm.tm_mday, &rtctm.tm_hour,
//		&rtctm.tm_min, &rtctm.tm_sec);
//	_tm.tm_sec = rtctm.tm_sec;
//	_tm.tm_min = rtctm.tm_min;
//	_tm.tm_hour = rtctm.tm_hour;
//	_tm.tm_mday = rtctm.tm_mday;
//	_tm.tm_mon = rtctm.tm_mon - 1;
//	_tm.tm_year = rtctm.tm_year - 1900;
//
//	timep = mktime(&_tm);
//	tv.tv_sec = timep;
//	tv.tv_usec = 0;
//	if (settimeofday(&tv, (struct timezone *) 0) < 0)
//	{
//		printf("Set system datatime error!/n");
//		return -1;
//	}
//	return 0;
//}

#include "SvrMain.h"

extern SvrMain* g_pMain;


WSMQ_NAMESPACE_BEGIN

ProcCmd::ProcCmd(Config * config)
{
	m_config = config;
	m_blRunning = false;

	m_mqSvr = new MqSvr();
	m_mqSvr->Init(m_config->stMqtt.ip + ":" + std::to_string(m_config->stMqtt.port), "mqsvr", m_config->stMqtt.topic);
	memset(m_tmLastCamera, 0, sizeof(m_tmLastCamera));
}

ProcCmd::~ProcCmd()
{
	delete m_mqSvr;
}

bool ProcCmd::GetBoxId(std::string &boxId)
{
	if (!m_config)
		return false;

	boxId = m_config->getBoxId();
	return true;
}

bool ProcCmd::SetBoxId(std::string boxId)
{
	if (!m_config)
		return false;

	if (!m_config->setBoxId(boxId))
		return false;
	return true;
}

bool ProcCmd::GetBaseInfo(Config::BaseParam &base)
{
	/*
	{
		"model": "SM5-S",
		"chip": "BM1684",
		"mcu": "STM32",
		"product sn": "HQDZKM6BBJCJH0471",
		"board type": "0x05",
		"mcu version": "0x35",
		"pcb version": "0x11",
		"reset count": 0
	}
	*/

	using namespace std;

	char* szCmd = "cat /sys/bus/i2c/devices/1-0017/information";
	stringstream sstream;
	GetShellRes(szCmd, sstream);
	
	string szs = sstream.str();
	printf("sz = %s", szs.c_str());
	rapidjson::Document doc;
	doc.Parse<rapidjson::kParseStopWhenDoneFlag>(szs.c_str());

	if (doc.HasParseError())
	{
		//MHC_ERROR("Config file Error rapidjson Parse error\n");
		return false;
	}
	if (!doc.IsObject())
	{
		//MHC_ERROR("Config file Error json not object \n");
		return false;
	}
	if (!doc.HasMember("model") || !doc["model"].IsString())
		return false;
	base.model = doc["model"].GetString();
	if (!doc.HasMember("product sn") || !doc["product sn"].IsString())
		return false;
	base.snbr = doc["product sn"].GetString();

	return true;
}

bool ProcCmd::GetChipTemp(float &ft)
{
	//cat /sys/class/thermal/thermal_zone0/temp
	char* szCmd = "cat /sys/class/thermal/thermal_zone0/temp";
	std::stringstream sstream;
	GetShellRes(szCmd, sstream);
	ft = 0.0f;
	sstream >> ft;
	ft /= 1000.0f;
	
	return true;
}

bool ProcCmd::GetBoradTemp(float &ft)
{
	//cat /sys/class/thermal/thermal_zone1/temp
	char* szCmd = "cat /sys/class/thermal/thermal_zone1/temp";
	std::stringstream sstream;
	GetShellRes(szCmd, sstream);
	ft = 0.0f;
	sstream >> ft;
	ft /= 1000.0f;

	return true;
}

bool ProcCmd::GetPower(char * pwd, float &cput, float &gput)
{
	//echo mypassword | sudo pmbus -d 0 -s 0x5c -i
	char szCmd[1024] = { 0 };
	sprintf(szCmd, "echo %s | sudo pmbus -d 0 -s 0x5c -i", pwd);

	using namespace std;

	std::stringstream sstream;
	GetShellRes(szCmd, sstream);
	string szs = sstream.str();
	/* 
	I2C port 0, addr 0x5c, type 0x3, reg 0x0, value 0x0
	ISL68127 revision 0x33
	ISL68127 switch to output 0, ret=0
	ISL68127 output voltage: 617mV
	ISL68127 output current: 1400mA
	ISL68127 temperature 1: 40`C  //TPU
	ISL68127 output power: 1W
	ISL68127 switch to output 1, ret=0
	ISL68127 output voltage: 923mV
	ISL68127 output current: 1000mA
	ISL68127 temperature 1: 42`C
	ISL68127 output power: 1W	//cpu
	*/

	smatch result;
	regex reg("output power : (\\d+)W");
	string::const_iterator iter_begin = szs.cbegin();
	string::const_iterator iter_end = szs.cend();
	int ip = 0;
	while (regex_search(iter_begin, iter_end, result, reg))
	{
		if (ip == 0)
		{
			cput = atof(result[1].str().c_str());
			ip++;
		}
		else
		{
			gput = atof(result[1].str().c_str());
		}
		iter_begin = result[0].second;	
	}

	return true;
}

int ProcCmd::start()
{
	m_blRunning = false;

	m_thread = std::thread(&ProcCmd::WorkProcess, this);

	return 1;
}

int ProcCmd::stop()
{
	m_blRunning = false;
	
	m_thread.join();

	return 1;
}

std::string ProcCmd::GetAlarm(int ch, int status)
{
	//{”boxId”:”B42”,”domain”:”ai.status”,"channel":1,"createTime":"2020-01-01 12:10:10","status":1}

	std::string data = GetSysFormatTime();
	if (data == "")
	{
		data = "1970-01-01 00:01:01";
	}

	using namespace rapidjson;

	StringBuffer buffer;
	PrettyWriter<StringBuffer> writer(buffer);

	writer.StartObject(); // root {

	writer.Key("boxId");
	writer.String(m_config->stBase.boxId.c_str());
	writer.Key("domain");
	writer.String("ai.status");
	writer.Key("channel");
	writer.Int(ch);
	writer.Key("createTime");
	writer.String(data.c_str());

	writer.Key("status");
	writer.Int(status);
	writer.EndObject(); // root }

	return buffer.GetString();
}

std::string ProcCmd::GetFault(int type, float threshold)
{
	//{”boxId”:”B42”,”domain”:”ai.fault”,"type":1,"createTime":"2020-01-01 12:10:10","data":xx}

	std::string data = GetSysFormatTime();
	if (data == "")
	{
		data = "1970-01-01 00:01:01";
	}

	using namespace rapidjson;

	StringBuffer buffer;
	PrettyWriter<StringBuffer> writer(buffer);
	writer.SetMaxDecimalPlaces(2);
	writer.StartObject(); // root {

	writer.Key("boxId");
	writer.String(m_config->stBase.boxId.c_str());
	writer.Key("domain");
	writer.String("ai.fault");
	writer.Key("type");
	writer.Int(type);
	writer.Key("createTime");
	writer.String(data.c_str());

	writer.Key("data");
	writer.Double(threshold);
	writer.EndObject(); // root }

	return buffer.GetString();
}

void ProcCmd::CheckFirst(bool bl)
{
	int64_t nOnline1;
	int64_t nOnline2;
	int64_t nOnline3;
	int64_t nOnline4;
	CheckCamera(nOnline1, nOnline2, nOnline3, nOnline4);

	if (bl)
	{
		g_pMain->PushAlarm(GetAlarm(1, nOnline1 == -1 ? 0 : 1));
	}
	else if (nOnline1 == -1);
	{
		g_pMain->PushAlarm(GetAlarm(1, 0));
	}

	if (bl)
	{
		g_pMain->PushAlarm(GetAlarm(2, nOnline2 == -1 ? 0 : 1));
	}
	else if (nOnline2 == -1);
	{
		g_pMain->PushAlarm(GetAlarm(2, 0));
	}

	if (bl)
	{
		g_pMain->PushAlarm(GetAlarm(3, nOnline3 == -1 ? 0 : 1));
	}
	else if (nOnline3 == -1);
	{
		g_pMain->PushAlarm(GetAlarm(3, 0));
	}

	if (bl)
	{
		g_pMain->PushAlarm(GetAlarm(4, nOnline4 == -1 ? 0 : 1));
	}
	else if (nOnline4 == -1);
	{
		g_pMain->PushAlarm(GetAlarm(4, 0));
	}

	m_nOnline1 = nOnline1;
	m_nOnline2 = nOnline2;
	m_nOnline3 = nOnline3;
	m_nOnline4 = nOnline4;

	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}

void ProcCmd::WorkProcess()
{
	float temperature;
	float fcpu;
	float ftpu;
	int ram;
	int ramUsed;
	int disk;
	int diskUsed;
	std::string sysTime;
	int runTime;

	int64_t nOnline1;
	int64_t nOnline2;
	int64_t nOnline3;
	int64_t nOnline4;

	CheckFirst(true);

	while (m_blRunning)
	{
		CheckFirst(false);

		if (!CheckSystem(temperature, fcpu, ftpu, ram, ramUsed,
			disk, diskUsed, sysTime, runTime))
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		}
		//float cput, cpuw, gput, gpuw;
		//GetPower("linaro", cput, cpuw, gput, gpuw);
		CheckSysAlarm(temperature, fcpu, ftpu, ram, ramUsed, disk, diskUsed, sysTime);

		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
}

void ProcCmd::CheckSysAlarm(float temperature, float fcpu, float ftpu, int ram, int ramUsed,
	int disk, int diskUsed, std::string sysTime)
{
	//type：故障类型，1-cpu过高：2-tpu过高；3-内存过高；4-存储过高；5-温度过高
	if (fcpu > m_config->stThreshold.fcpu)
	{
		g_pMain->PushAlarm(GetFault(1, m_config->stThreshold.fcpu));
	}

	if (ftpu > m_config->stThreshold.ftpu)
	{
		g_pMain->PushAlarm(GetFault(2, m_config->stThreshold.ftpu));
	}

	if (ramUsed * 1.0f / ram > m_config->stThreshold.ram)
	{
		g_pMain->PushAlarm(GetFault(3, m_config->stThreshold.ram));
	}

	if (diskUsed * 1.0f / disk > m_config->stThreshold.disk)
	{
		g_pMain->PushAlarm(GetFault(4, m_config->stThreshold.disk));
	}

	if (temperature > m_config->stThreshold.temperature)
	{
		g_pMain->PushAlarm(GetFault(5, m_config->stThreshold.temperature));
	}
}

bool ProcCmd::CheckCamera(int64_t &nOnline1, int64_t  &nOnline2, int64_t  &nOnline3, int64_t  &nOnline4)
{
	// 返回最后修改时间到现在的差值
	//  "#channelStatusPath": "目录下建一个四个空文件 channel1/channel2/channel3/channel4 设备在线建一遍，掉线删掉",
	time_t tmn = time(NULL);

	std::string szp = m_config->channelStatusPath + "/channel1";
	struct stat filestate;
	if (access(szp.c_str(), F_OK) != 0)
	{
		nOnline1 = -1;
		return false;
	}
	else
	{
		stat(szp.c_str(), &filestate);
		nOnline1 = tmn - filestate.st_mtime;
	}
	szp = m_config->channelStatusPath + "/channel2";
	if (access(szp.c_str(), F_OK) != 0)
	{
		nOnline2 = -1;
		return false;
	}
	else
	{
		stat(szp.c_str(), &filestate);
		nOnline2 = tmn - filestate.st_mtime;
	}
	szp = m_config->channelStatusPath + "/channel3";
	if (access(szp.c_str(), F_OK) != 0)
	{
		nOnline3 = -1;
		return false;
	}
	else
	{
		stat(szp.c_str(), &filestate);
		nOnline3 = tmn - filestate.st_mtime;
	}
	szp = m_config->channelStatusPath + "/channel4";
	if (access(szp.c_str(), F_OK) != 0)
	{
nOnline4 = -1;
return false;
	}
	else
	{
	stat(szp.c_str(), &filestate);
	nOnline4 = tmn - filestate.st_mtime;
	}

	return true;
}

bool ProcCmd::CheckSystem(float &temperature, float &fcpu, float &ftpu, int &ram, int &ramUsed,
	int &disk, int &diskUsed, std::string &sysTime, int &runTime)
{
	using namespace std;
	//"data":{"temperature":36.7,"cpu":0.64,"tpu":0.75,"ram":2048,
	//"ramUsed":1024,"disk":8192,"diskUsed":1024,"sysTime":"2022-01-01 10:00:00","runTime":1000}
	char buf1[255] = { 0 };
	char buf2[255] = { 0 };
	char buf3[255] = { 0 };
	char buf4[255] = { 0 };

	GetBoradTemp(temperature);

	//disk
	string szCmd = "df -m /";
	std::stringstream sstream;
	GetShellRes((char*)szCmd.c_str(), sstream);
	string str;
	string szf = sstream.str();
	getline(sstream, str, '\n');
	getline(sstream, str, '\n');

	unsigned long Size, Used, Avail;
	if (sscanf(str.c_str(), "%s%d%d%d", buf1, &Size, &Used, &Avail) != 4)
	{
		printf("disk error! \n");
		return false;
	}
	else
	{
		disk = Size;
		diskUsed = Used;
	}
	sstream.clear();
	sstream.str("");
	//mem
	szCmd = "free -m";
	GetShellRes((char*)szCmd.c_str(), sstream);
	szf = sstream.str();
	getline(sstream, str, '\n');
	getline(sstream, str, '\n');
	if (sscanf(str.c_str(), "%s%d%d%d", buf1, &Size, &Used, &Avail) != 4)
	{
		printf("mem error! \n");
		return false;
	}
	else
	{
		ram = Size;
		ramUsed = Used;
	}
	sstream.clear();
	sstream.str("");
	//sysTime
	time_t cur_time = time(NULL);
	struct tm local_tm;

	if (NULL == localtime_r(&cur_time, &local_tm))
	{
		printf("time error! \n");
		return false;
	}

	fcpu = get_sysCpuUsage();

	//todo tpu
	ftpu = 0.00;
	char stm[30] = { 0 };
	sprintf(stm, "%4d-%02d-%02d %02d:%02d:%02d", local_tm.tm_year + 1900,
		local_tm.tm_mon + 1, local_tm.tm_mday, local_tm.tm_hour, local_tm.tm_min, local_tm.tm_sec);
	sysTime = stm;

	sstream.clear();
	sstream.str("");
	szCmd = "uptime";
	GetShellRes((char*)szCmd.c_str(), sstream);
	szf = sstream.str();
	getline(sstream, str, '\n');

	printf("uptime \n");
	printf("%s\n", szf.c_str());
	//up 15 min
	int is = sscanf(szf.c_str(), "%s%s%d:%d,", buf1, buf2, buf3, buf4);
	if (is != 4)
		//if (sscanf(szf.c_str(), "%s%s%d:%d,", buf1, buf2, buf3, buf4) != 4)
	{
		is = sscanf(szf.c_str(), "%s%s%dmin,", buf1, buf2, buf3);
		if (is != 3)
		{
			printf("runtime error! %s, %s, %d\n", buf1, buf2, buf3[0]);
			return false;
		}
		else
		{
			runTime = buf3[0] * 60;
		}
		//printf("runtime error! %s, %s, %d, %d\n", buf1, buf2, buf3[0], buf4[0]);

		//return false;
	}
	else
		runTime = buf3[0] * 3600 + buf4[0] * 60;
	return true;
}

bool ProcCmd::checkDisks(DiskAlarm & diskAlarm)
{
	std::lock_guard<std::mutex> lock(m_mutex);

#if __WINDOWS_OS__
	//m_disk.CheckFreeSpace((LPCTSTR)inf.disk);
#elif __LINUX_OS__
	m_disk.refreshInfo();
#endif
	if (m_disk.GetTotalFreeOfPercentage() <= m_diskAlarmParam.percent || m_disk.GetFreeMBytesAvailable() <= m_diskAlarmParam.mB)
	{
		diskAlarm.mB = m_disk.GetFreeMBytesAvailable();
		diskAlarm.percent = m_disk.GetTotalFreeOfPercentage();
		return true;
	}
	else
		return false;
}

int ProcCmd::SetNetwork(char *ifname, const char* ip, const char* mask, const char* gateway, const char* dns)
{
	system("echo linaro | sudo cp /etc/network/interfaces.d/eth0 /etc/network/interfaces.d/eth0.bak");

	char buf[500] = { 0 };
	//FILE *fp;
	//fp = fopen("/etc/network/interfaces.d/eth0", "rw");
	//if (!fp)
	//	return -1;
	//fprintf(fp, "auto eth0\n");
	//fprintf(fp, "	iface eth0 inet static\n");
	//fprintf(fp, "	address %s\n", ip);
	//fprintf(fp, "	netmask %s\n", mask);
	//fprintf(fp, "	gateway %s\n", gateway);
	//fprintf(fp, "	dns-nameservers %s\n", dns);
	//fclose(fp);

	sprintf(buf, "/data/bm_bin/bm_set_ip %s %s %s %s", ip, mask, gateway, dns);

	system(buf);
	return 0;
}

int ProcCmd::GetNetwork(char *ifname, char* ip, char* mask, char* gateway, char* dns)
{
	char* szCmd = "cat /etc/network/interfaces.d/eth0";
	std::stringstream sstream;
	GetShellRes(szCmd, sstream);
	char buf[260];
	//auto eth0
	//	iface eth0 inet static
	//	address 192.168.1.92
	//	netmask 255.255.255.0
	//	gateway 192.168.1.1
	//	dns-nameservers 192.168.1.1
	using namespace std;
	string str;
	getline(sstream, str, '\n');
	getline(sstream, str, '\n');
	getline(sstream, str, '\n');
	
	int Arr[4];
	printf("ip=str %s \n", str.c_str());
	if (sscanf(str.c_str(), "%s%d.%d.%d.%d", buf, &Arr[0], &Arr[1], &Arr[2], &Arr[3]) != 5)
		return -1;
	sprintf(ip, "%d.%d.%d.%d", Arr[0], Arr[1], Arr[2], Arr[3]);
	printf("ip=%s \n", ip);
	
	getline(sstream, str, '\n');
	if (sscanf(str.c_str(), "%s%d.%d.%d.%d", buf, &Arr[0], &Arr[1], &Arr[2], &Arr[3]) != 5)
		return -2;
	sprintf(mask, "%d.%d.%d.%d", Arr[0], Arr[1], Arr[2], Arr[3]);

	getline(sstream, str, '\n');
	if (sscanf(str.c_str(), "%s%d.%d.%d.%d", buf, &Arr[0], &Arr[1], &Arr[2], &Arr[3]) != 5)
		return -3;
	sprintf(gateway, "%d.%d.%d.%d", Arr[0], Arr[1], Arr[2], Arr[3]);

	getline(sstream, str, '\n');
	if (sscanf(str.c_str(), "%s%d.%d.%d.%d", buf, &Arr[0], &Arr[1], &Arr[2], &Arr[3]) != 5)
		return -4;
	sprintf(dns, "%d.%d.%d.%d", Arr[0], Arr[1], Arr[2], Arr[3]);
	return 0;
}

bool ProcCmd::GetCount(std::string path, int channel, int &count)
{
	//cat /sys/class/thermal/thermal_zone1/temp
	std::string sz = std::to_string(channel);
	if (channel == 0)
		sz = "";
	std::string szCmd = "ls " + path + " |grep " +  sz + ".jpg | wc -l";
	std::stringstream sstream;

	//todo channel
	GetShellRes((char*)szCmd.c_str(), sstream);
	count = 0;
	sstream >> count;

	return true;
}

int filter_fn(const struct dirent * ent)
{
	if (ent->d_type != DT_REG)
		return 0;

	//return (strncmp(ent->d_name, "lib", 3) == 0);
	return 0;
}


bool ProcCmd::GetAlgParam(std::string path, int channel, int pageNbr, int pageSize, std::vector<algParam> & params)
{
	struct dirent **namelist;
	int n;
	n = scandir(path.c_str(), &namelist, filter_fn, alphasort);
	if (n < 0)
		perror("scandir error");
	else
	{
		while (n--)
		{
			algParam param;
			param.snapshotUrl = "http://" + (std::string)namelist[n]->d_name;
			//todo channel
			params.push_back(param);
			//printf("%s\n", namelist[n]->d_name);
			//free(namelist[n]);
		}
		free(namelist);
	}

	return true;
}

#if 0
int ProcCmd::SetNetwork(char *ifname, const char* ip, const char* mask, const char* gateway, const char* dns)
{
	int ires = (SetIfAddr(ifname, (char*)ip, (char*)mask, (char*)gateway));
	if (ires < 0)
	{
		//双数的是ip不对
		if (ires % 2 == 0)
		{
			return -2;
		}
		else
			return -1;
	}

	if (!SetDns((char*)dns))
	{
		return -3;
	}

	return 0;
}

int ProcCmd::GetNetwork(char *ifname, char* ip, char* mask, char* gateway, char* dns)
{
	char mac[32] = { 0 };
	char stats[100] = { 0 };
	if (getifInfo(ifname, ip, mask, mac, stats) != 0)
		return -1;
	unsigned long p;
	char gateway[32] = { 0 };
	if (get_gateway(&p, gateway) != 0)
		return -2;

	if (SetDns(dns) != 0)
		return -3;

	return 0;
}
#endif

WSMQ_NAMESPACE_END
