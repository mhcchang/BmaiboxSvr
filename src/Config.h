/*****************************************************************************
* projectName: tracking demo												 *
* file  Config.h															 *
* brief   										 							 *
* date: 2019/11/11 13:00													 *
* copyright(c) 2019 zhuohe													 *
*****************************************************************************/
#ifndef ___CONFIG_H__
#define ___CONFIG_H__


#include <string>
#include <vector>
#include <atomic>

#if __WINDOWS_OS__
#pragma warning (push)
#pragma warning (disable: 4251 4305 4996 4267 4244 4838 4305)
#else
/////gunc
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#pragma GCC diagnostic ignored "-Wreorder"
#endif

#include "defs.h"
#include <map>
#include <set>

#include <iostream>
#include <vector>
#include <iterator>
#include <regex>
#include <mutex>

#include <unordered_map>
#include <unordered_set>

#if __WINDOWS_OS__
//#include "rdc_fs_watcher.h"
#else
//#include "watchdog.hxx"
#endif
#define CONFIG_FILE_PATH "/data/exeAlg/cameras_configure.json"
#define L_CONFIG_FILE_PATH L"/data/exeAlg/cameras_configure.json"

std::vector<std::string> str_split(const std::string& in, const std::string& delim);

class Config
{
public:
	typedef std::function<void(int64_t, const std::set<std::pair<std::wstring /* path */, uint32_t /* action */>>&)> ChangeEvent;

	//std::string boxId;
	struct PrivateParam {
		std::string logLevel;
	};
	PrivateParam stPrivate;

	struct HttpParam {
		int port;
	};
	HttpParam stHttp;

	struct BaseParam
	{
		std::string serviceName;
		std::string serviceDisplayName;
		std::string serviceDescription;

		int startType;

		std::string boxId;
		std::string model;
		std::string snbr;
		std::string softVer;
		std::string algoVer;
	};
	BaseParam stBase;

	struct NtpParam {
		bool used;
		std::string ip;
	};
	NtpParam stNtp;

	struct MqttParam
	{
		std::string ip;
		int port;
		std::string user;
		std::string password;
		std::string topic;
	};
	MqttParam stMqtt;

	struct ThresholdParam {
		float temperature;
		float fcpu;
		float ftpu;
		int ram;
		int ramUsed;
		int disk; 
		int diskUsed;
	};
	ThresholdParam stThreshold;
	struct Pt {
		float x;
		float y;
	};
	struct RoiParam {
		int roiId;
		std::vector<Pt> points;
	};

	struct CameraParam
	{
		std::string address;
		int chan_num;
		int ptzType;
		int imgType;
		float algSensitivity;
		float vqaSensitivity;
		int retentionTime;
		int alarmDelayedTime;
		int objectSize;
		int frameNbr;
		float confidence;
		float imageQuality;
		std::string name;
		int width;
		int height;
		std::vector<RoiParam> rois;
	};

	struct DeviceParam {
		int devid;
		CameraParam camera;
	};
	std::vector<DeviceParam> camera_infos;

	struct GuardParam {
		std::string startTime;
		std::string endTime;
	};
	GuardParam stGuard;

	std::string algVersionPath; // : "/data/algversion",
	std::string detectModelPath;// : "/data/modelPath/detect.bmodel",
	std::string classifyModelPath; // " : " / data / modelPath / resnet50.bmodel",
	std::string exeAlgPath;//" : "/data/exeAlg",
	std::string snapshotPath; // " : " / data / shot",
	std::string alarmImagePath;// " : " / data / alarm",
	std::string softUpdatePath;//" : "/data/updateBin",
	std::string algoUpdatePath;// " : " / data / updateModel",
	std::string channelStatusPath;//" : "/data/channelStatus"
	//struct RuleBusiness {
	//	int ruleId;
	//	std::string algoType;
	//	std::string ruleType;
	//	std::string ruleName;
	//	std::string ruleDescription;
	//	std::string objectCategory;
	//	std::string objectClass;
	//};
	std::string nginxPath;
	std::string nginxImgPath;

public:
	/**配置文件位置 */
	std::string configPath;
	Config();

private:

public:

	char* m_configPath;
#ifdef _WIN32
	//RdcFSWatcher m_watch;
#else
	//Watchdog m_watch;
#endif

public:
	bool Init(ChangeEvent changeEvent = nullptr);
	/**
	* @brief 加载配置文件 失败原因都是配置文件不符合json规则问题导致的
	* @param configPath 配置文件位置
	* @return 成功返回true 失败返回false
	* @throws 无
	*/
	bool load(const char* configPath);
	bool save();

	std::string getBoxId();
	bool getNtpStatus();
	std::string getNtpIp();

	bool setBoxId(std::string boxId);
	bool setPassword(std::string pwd);

	bool setNtp(std::string ip, bool used);

	bool AddChannel(DeviceParam & dev);
	bool DelChannel(int channel);

	bool SetRois(int channel, std::vector<Config::RoiParam> & rois);

	bool SetArmRule(std::string startTime, std::string endTime);
	bool SetAlgoRule(int retentionTime, int alarmDelayedTime, int objectSize, int frameNbr, float confidence, float imageQuality );
	
protected:
	/**
	* @brief 读取配置文件 返回字符串 将文件数据加载到内存中 供json解析
	* @param configPath  配置文件位置
	* @return std::string 配置文件内的所有字符串
	* @throws 无
	*/
	std::string readFile(const char* configPath);

	//bool GetConfigGuardTime(rapidjson::Value *arrg, CFG_CROSSFENCEDETECTION_INFO &crossFence);

	std::atomic_bool m_blInit;
	std::wstring m_wFileName;
	void changeEventImp(int64_t id, const std::set<std::pair<std::wstring, uint32_t> >& notifications);

private:
};

#if __WINDOWS_OS__
#pragma warning (pop)
#else
#pragma GCC diagnostic pop
#endif
#endif//__GM_RECORD_CONFIG_H__
