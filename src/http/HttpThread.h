#pragma once

#include "defs.h"

#if __WINDOWS_OS__
#pragma warning (push)
#pragma warning (disable: 4251 4305 4996 4267 4244 4838)
#else
#define _access access
/////gunc
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#pragma GCC diagnostic ignored "-Wreorder"
#endif

//服务程序逻辑主体
//#include "PlateDetectorSdk.h"
//#include "WsDetectClient.h"
//#include "MngClient.h"
#include <thread>
#include <atomic>
#include <mutex>
#include <list>
#include <map>

#include "Config.h"
#include "ProcCmd.h"

class HttpThread
{
public:
	HttpThread(/*PlateDetectorSvr* main, const Config* config, */int nThreadNum);
	~HttpThread();

	void SetConfig();
	void SetProCmd();

	bool start();
	void stop(bool blExit = false);
	//获取返回json
	std::string GetJson(int code, std::string info, std::string data);

	bool GetLogin(std::string query, uint32_t inaddr, std::string token, std::string body, std::string& res);
	bool GetLogout(std::string query, uint32_t inaddr, std::string token, std::string body, std::string& res);
	bool GetSetPassword(std::string query, uint32_t inaddr, std::string token, std::string body, std::string& res);
		//3.1、
	bool GetGetBoxId(std::string query, uint32_t inaddr, std::string token, std::string body, std::string& res);
	//3.2、
	bool GetSetBoxId(std::string query, uint32_t inaddr, std::string token, std::string body, std::string& res);
	//3.3、获取系统时间
	bool GetGetSysTime(std::string query, uint32_t inaddr, std::string token, std::string body, std::string& res);
	//3.4、设置系统时间
	bool GetSetSysTime(std::string query, uint32_t inaddr, std::string token, std::string body, std::string& res);
	//3.5、设置NTP（自动）
	bool GetSetSysNtp(std::string query, std::string body, std::string& res);
	//std::string MakeSetSysNtp(stru_WsSvrSend &msgSend);
	//3.6、盒子重启
	bool GetReboot(std::string query, uint32_t inaddr, std::string token, std::string body, std::string& res);
	//std::string MakeReboot(stru_WsSvrSend &msgSend);
	//3.7、获取基本信息
	bool GetGetBasicInfo(std::string query, uint32_t inaddr, std::string token, std::string body, std::string& res);
	//std::string MakeGetBasicInfo(stru_WsSvrSend &msgSend);
	//3.8、后端服务远程升级
	bool GetSoftUpdate(std::string query, uint32_t inaddr, std::string token, std::string body, std::string& res);
	//std::string MakeSoftUpdate(stru_WsSvrSend &msgSend);
	//3.9、算法（模型）升级
	bool GetAlgoUpdate(std::string query, uint32_t inaddr, std::string token, std::string body, std::string& res);
	//std::string MakeAlgoUpdate(stru_WsSvrSend &msgSend);
	//3.10、网络设置
	bool GetSetNetwork(std::string query, uint32_t inaddr, std::string token, std::string body, std::string& res);
	//std::string MakeSetNetwork(stru_WsSvrSend &msgSend);
	//3.11、读取网络设置
	bool GetGetNetwork(std::string query, uint32_t inaddr, std::string token, std::string body, std::string& res);
	//std::string MakeGetNetwork(stru_WsSvrSend &msgSend);
	//3.12、mqtt设置
	bool GetSetMqtt(std::string query, uint32_t inaddr, std::string token, std::string body, std::string& res);
	//std::string MakeSetMqtt(stru_WsSvrSend &msgSend);
	//3.13、读取mqtt设置
	bool GetGetMqtt(std::string query, uint32_t inaddr, std::string token, std::string body, std::string& res);
	//std::string MakeGetMqtt(stru_WsSvrSend &msgSend);
	//3.15、密码登录鉴权
	bool GetCheckPassword(std::string query, uint32_t inaddr, std::string token, std::string body, std::string& res);
	//4.1、获取设备状态
	bool GetGetDeviceStatus(std::string query, uint32_t inaddr, std::string token, std::string body, std::string& res);
	//5.1、添加摄像机
	bool GetAddCamera(std::string query, uint32_t inaddr, std::string token, std::string body, std::string& res);
	//5.2、删除摄像机
	bool GetDelCamera(std::string query, uint32_t inaddr, std::string token, std::string body, std::string& res);
	//5.3、获取摄像机及其状态
	bool GetGetCameras(std::string query, uint32_t inaddr, std::string token, std::string body, std::string& res);
	//5.4、添加检测区域
	bool GetAddRoi(std::string query, uint32_t inaddr, std::string token, std::string body, std::string& res);
	//5.5、删除检测区域
	bool GetSetRoi(std::string query, uint32_t inaddr, std::string token, std::string body, std::string& res);
	//5.6、获取检测区域
	bool GetGetRoi(std::string query, uint32_t inaddr, std::string token, std::string body, std::string& res);
	//5.7、获取摄像机截图
	bool GetGetSnapshot(std::string query, uint32_t inaddr, std::string token, std::string body, std::string& res);
	//6.1、设置布控时间
	bool GetSetArmRule(std::string query, uint32_t inaddr, std::string token, std::string body, std::string& res);
	//6.2、读取布控时间
	bool GetGetArmRule(std::string query, uint32_t inaddr, std::string token, std::string body, std::string& res);
	//6.3、设置算法规则
	bool GetSetAlgoRule(std::string query, uint32_t inaddr, std::string token, std::string body, std::string& res);
	//6.4、读取算法规则
	bool GetGetAlgoRule(std::string query, uint32_t inaddr, std::string token, std::string body, std::string& res);
	//7.1、报警数量查询
	bool GetCount(std::string query, uint32_t inaddr, std::string token, std::string body, std::string& res);
	bool GetQuery(std::string query, uint32_t inaddr, std::string token, std::string body, std::string& res);

private:
	int m_nThreadNum;
	rapidjson::Document doc;
	const Config * m_config;
	//unsigned char * m_pImageData;
	//unsigned char * m_pImageDataSync;
	//unsigned char * m_buf;
	char * m_strBase64;

	bool m_blInit;
	std::thread m_thread;
	//std::timed_mutex m_timemutex;
	std::mutex m_timemutex;
	volatile std::atomic_bool m_blRunning;
	
	//处理消息
	void MainLoop();
	void HttpProcess();
	std::string GeneralReturn();
	std::string GenMsgId();

	//Config* m_config;
	volatile uint64_t m_tmWeb;
	std::string m_szToken;
	int m_inaddr;
	//外部config传入 md5后的密码 
	std::string m_szPwd;

	WsMqSvr::ProcCmd * m_pro;
};
