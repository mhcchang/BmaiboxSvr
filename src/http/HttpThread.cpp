#include "defs.h"
#include "HttpThread.h"
#include "utils.h"
#include <time.h>
//#include "MD5.hpp"
//#include "sole.hpp"
//#include "sys_inc.h"
//#include "sys_log.h"
//#include "PubUtil.h"
//#include "base64.h"

#include "document.h"
#include "stringbuffer.h"
#include "writer.h"
#include "ostreamwrapper.h"

#include <iostream>
#include "writer.h"
#include "sole.hpp"
#include "ProcCmd.h"
#include "logger.h"

#define APPLOG printf

#ifdef GetObject
#undef GetObject
#endif // GetObject #undef GetObject

#include "SvrMain.h"

extern SvrMain* g_pMain;

HttpThread::HttpThread(/*PlateDetectorSvr* main, const Config* config,*/ int nThreadNum)
{
	m_nThreadNum = nThreadNum;
	m_blInit = false;
	m_tmWeb = 0;
	m_config = nullptr;
	m_pro = nullptr;
	m_szPwd = "21232f297a57a5a743894a0e4a801fc3";
}

HttpThread::~HttpThread()
{
	m_szToken = "";
}

//处理消息
void HttpThread::MainLoop()
{
	m_blRunning = true;
	//log_print(ZH_LOG_DEBUG, "HttpThread MainLoop\n");
	while (m_blRunning)
	{
		//m_pLprDetectInfo = m_pMain->GetWaitLpr(m_nThreadNum);
		////log_print(ZH_LOG_TRACE, "HttpThread m_pMain->GetWaitLpr %p\n", m_pLprDetectInfo);
		//if (m_pLprDetectInfo != nullptr)
		//{
		//	//todo
		//	m_pLprDetectInfo->msgData.trackTimestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		//	LprProcess();
		//	log_print(ZH_LOG_DEBUG, "HttpThread MainLoop %p\n", m_pLprDetectInfo);
		//	delete m_pLprDetectInfo;
		//}
		//else
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}

std::string HttpThread::GeneralReturn()
{
	return "{\"retn\":0, \"info\" : \"success\", \"data\" : {} }";
}

std::string HttpThread::GetJson(int code, std::string info, std::string data)
{
	std::string res = "{\"retn\":" + std::to_string(code) +  ", \"info\":\"" + info + "\", \"data\":" + data +"}";
	return res;
}

std::string HttpThread::GenMsgId()
{
	return sole::uuid0().str();
}

bool HttpThread::GetLogin(std::string query, uint32_t inaddr, std::string token, std::string body, std::string& res)
{
	printf("login \n");
	doc.Parse<rapidjson::kParseStopWhenDoneFlag>(body.c_str());
	res = "{\"retn\":1002, \"info\":\"invalid token\", \"data\":null}";

	if (!doc.HasMember("password") || !doc["password"].IsString())
	{
		res = "{\"retn\":1006, \"info\":\"invalid parameter\", \"data\":null}";
		return false;
	}
	else
	{
		std::string sz = doc["password"].GetString();
		if (m_szPwd == sz)
		{
			m_szToken = GenMsgId();
			m_inaddr = inaddr;
		}
		//std::string data = "\"data\":{\"token\":\"" + m_szToken + "\"}\"";
		std::string data = "{\"token\":\"" + m_szToken + "\"}";
		res = GetJson(0, "success", data); // "data":{"token":”c6958189 - 0ffe - 4836 - bf31 - 87eebefabaff”}";
		return true;
	}
}

bool HttpThread::GetLogout(std::string query, uint32_t inaddr, std::string token, std::string body, std::string& res)
{
	printf("logout \n");
	doc.Parse<rapidjson::kParseStopWhenDoneFlag>(body.c_str());

	std::string data = "\"data\":{}";
	res = GetJson(0, "success", data);

	m_szToken = "";
	m_szPwd = "";
	m_inaddr = 0;

	return true;
}

bool HttpThread::GetSetPassword(std::string query, uint32_t inaddr, std::string token, std::string body, std::string& res)
{
	std::string data = "\"data\":{}";
	//{"retn":0,"info":"success","data":{...]}}
	//if (inaddr != m_inaddr)
	//{
	//	res = GetJson(1004, "invalid token ip not login", data);
	//	return false;
	//}

	//if (m_szToken != token)
	//{
	//	res = GetJson(1002, "invalid token", data);
	//	return false;
	//}

	doc.Parse<rapidjson::kParseStopWhenDoneFlag>(body.c_str());
	if (doc.HasParseError())
	{
		MHC_ERROR("Config file Error rapidjson Parse error\n");
		return false;
	}
	if (!doc.IsObject())
	{
		MHC_ERROR("Config file Error json not object \n");
		return false;
	}
	if (!doc.HasMember("password"))
		return false;

	if (!doc["password"].IsString())
	{
		res = "{\"retn\":1006, \"info\":\"invalid parameter\", \"data\":null}";
		return false;
	}
	else
	{
		std::string sz = doc["password"].GetString();
		
		g_pMain->GetConfig()->setPassword(sz);

		res = GeneralReturn();
		return true;
	}
}

bool HttpThread::GetGetBoxId(std::string query, uint32_t inaddr, std::string token, std::string body, std::string& res)
{
	printf("GetBoxId \n");
	std::string data = "\"data\":{}";
	//{"retn":0,"info":"success","data":{...]}}
	if (inaddr != m_inaddr)
	{
		res = GetJson(1004, "invalid token ip not login", data);
		return false;
	}

	//if (m_szToken != token)
	//{
	//	res = GetJson(1002, "invalid token", data);
	//	return false;
	//}

	//"data":{"boxId":"BM1684-180"}
	res = "{\"retn\":0, \"info\" : \"success\", \"data\" : {\"boxId\":\" " + g_pMain->GetConfig()->getBoxId() + "\" } }";
	return true;
}

bool HttpThread::GetSetBoxId(std::string query, uint32_t inaddr, std::string token, std::string body, std::string& res)
{
	printf("SetBoxId \n");
	std::string data = "\"data\":{}";
	//{"retn":0,"info":"success","data":{...]}}
	//if (inaddr != m_inaddr)
	//{
	//	res = GetJson(1002, "invalid token ip not login", data);
	//	return false;
	//}
	//
	//if (m_szToken != token)
	//{
	//	res = GetJson(1002, "invalid token", data);
	//	return false;
	//}

	doc.Parse<rapidjson::kParseStopWhenDoneFlag>(body.c_str());
	if (doc.HasParseError())
	{
		MHC_ERROR("Config file Error rapidjson Parse error\n");
		return false;
	}
	if (!doc.IsObject())
	{
		MHC_ERROR("Config file Error json not object \n");
		return false;
	}
	if (!doc.HasMember("boxId"))
		return false;

	if (!doc["boxId"].IsString())
	{
		res = "{\"retn\":1006, \"info\":\"invalid parameter\", \"data\":null}";
		return false;
	}
	else
	{
		std::string sz = doc["boxId"].GetString();

		g_pMain->GetConfig()->setBoxId(sz);
		g_pMain->GetConfig()->load(NULL);

		res = GeneralReturn();
		return true;
	}
}

bool HttpThread::GetGetSysTime(std::string query, uint32_t inaddr, std::string token, std::string body, std::string& res)
{
	std::string data = "\"data\":{}";
	//{"retn":0,"info":"success","data":{...]}}
	//if (inaddr != m_inaddr)
	//{
	//	res = GetJson(1002, "invalid token ip not login", data);
	//	return false;
	//}

	//if (m_szToken != token)
	//{
	//	res = GetJson(1002, "invalid token", data);
	//	return false;
	//}

	time_t cur_time = time(NULL);
	struct tm local_tm;

	if (NULL == localtime_r(&cur_time, &local_tm))
	{
		//1007
		res = GetJson(1007, "inter error", data);
		return false;
	}
	
	char stm[20] = { 0 };
	sprintf(stm, "%4d%02d%02d%02d%02d%02d", local_tm.tm_year + 1900,
		local_tm.tm_mon + 1, local_tm.tm_mday, local_tm.tm_hour, local_tm.tm_min, local_tm.tm_sec);
	data = stm;
	res = "{\"retn\":0, \"info\" : \"success\", \"data\" : {\"sysTime\":\"" + data + "\", \"sysNtp\":\"" + g_pMain->GetConfig()->getNtpIp() + "\", \"useNtp\" :";
	if (g_pMain->GetConfig()->getNtpStatus())
		res = res + "true} }";
	else
		res = res + "false} }";
	return true;
}

bool HttpThread::GetSetSysTime(std::string query, uint32_t inaddr, std::string token, std::string body, std::string& res)
{
	std::string data = "\"data\":{}";
	//{"retn":0,"info":"success","data":{...]}}
	//if (inaddr != m_inaddr)
	//{
	//	res = GetJson(1004, "invalid token ip not login", data);
	//	return false;
	//}

	//if (m_szToken != token)
	//{
	//	res = GetJson(1002, "invalid token", data);
	//	return false;
	//}

	doc.Parse<rapidjson::kParseStopWhenDoneFlag>(body.c_str());

	if (!doc.HasMember("sysTime") || !doc["sysTime"].IsString())
	{
		res = "{\"retn\":1006, \"info\":\"invalid parameter\", \"data\":null}";
		return false;
	}
	else
	{
		std::string sz = doc["sysTime"].GetString();

		if (SetSystemTime((char*)sz.c_str()) < 0)
		{
			res = "{\"retn\":1006, \"info\":\"invalid parameter\", \"data\":null}";
			return false;
		}
	}

	std::string sz;
	bool bl;
	if (!doc.HasMember("sysNtp") || !doc["sysNtp"].IsString())
	{
		res = "{\"retn\":1006, \"info\":\"invalid parameter\", \"data\":null}";
		return false;
	}
	else
	{
		 sz = doc["sysNtp"].GetString();
	}
	if (!doc.HasMember("useNtp") || !doc["useNtp"].IsBool())
	{
		res = "{\"retn\":1006, \"info\":\"invalid parameter\", \"data\":null}";
		return false;
	}
	else
	{
		bl = doc["useNtp"].GetBool();
	}
	g_pMain->GetConfig()->setNtp(sz, bl);
	g_pMain->GetConfig()->load(NULL);
	res = GeneralReturn();
	return true;
}

bool HttpThread::GetReboot(std::string query, uint32_t inaddr, std::string token, std::string body, std::string& res)
{
	std::string data = "\"data\":{}";
	//if (inaddr != m_inaddr)
	//{
	//	res = GetJson(1004, "invalid token ip not login", data);
	//	return false;
	//}

	//if (m_szToken != token)
	//{
	//	res = GetJson(1002, "invalid token", data);
	//	return false;
	//}

	MHC_INFO("reboot");
	res = GeneralReturn();
	//todo!
	//reBootProc();
	system("touch .re");
	return true;
}

bool HttpThread::GetGetBasicInfo(std::string query, uint32_t inaddr, std::string token, std::string body, std::string& res)
{
	std::string data = "null";
	
	//if (inaddr != m_inaddr)
	//{
	//	res = GetJson(1004, "invalid token ip not login", data);
	//	return false;
	//}
	//
	//
	//if (m_szToken != token)
	//{
	//	res = GetJson(1002, "invalid token", data);
	//	return false;
	//}
	

	//"data":{"model":"K4","snbr":"202201001","softVer":"V1.0","algoVer":"V1.0","mac":"02:00:..."} 
	Config::BaseParam base;
	if (!m_pro->GetBaseInfo(base))
	{
		res = GetJson(1007, "can't get board info", data);
		return false;
	}

	char mac[32] = "";
	if (0 != get_local_mac("eth0", mac))
	{
		res = GetJson(1007, "can't get mac info", data);
		return false;
	}

	using namespace rapidjson;

	StringBuffer buffer;
	PrettyWriter<StringBuffer> writer(buffer);

	writer.StartObject(); // root {

	writer.Key("retn");
	writer.Int(0);

	writer.Key("info");
	writer.String("success");

	writer.Key("data");
	writer.StartObject();

	writer.Key("model");
	writer.String(base.model.c_str());

	writer.Key("snbr");
	writer.String(base.snbr.c_str());

	writer.Key("softVer");
	writer.String(m_config->stBase.softVer.c_str());

	writer.Key("algoVer");
	writer.String(m_config->stBase.algoVer.c_str());
	writer.Key("mac");
	writer.String(mac);

	writer.EndObject(); // data }

	writer.EndObject(); // root }

	res = buffer.GetString();

	return true;
}

bool HttpThread::GetSoftUpdate(std::string query, uint32_t inaddr, std::string token, std::string body, std::string& res)
{
	//todo
	std::string data = "\"data\":{}";
	res = GetJson(1007, "not implemented", data);

	return false;
}

bool HttpThread::GetGetNetwork(std::string query, uint32_t inaddr, std::string token, std::string body, std::string& res)
{
	std::string data = "\"data\":{}";
	//if (inaddr != m_inaddr)
	//{
	//	res = GetJson(1004, "invalid token ip not login", data);
	//	return false;
	//}

	//if (m_szToken != token)
	//{
	//	res = GetJson(1002, "invalid token", data);
	//	return false;
	//}
	
	//"data":{"ip":"192.168.1.10", "port":1883,"user":"admin","password":"123456","topic":"ai.event"}

	char ip[32] = { 0 };
	char mask[32] = { 0 };  
	char gateway[32] = { 0 }; 
	char dns[32] = { 0 };

#if 0
	if (!m_pro->GetNetwork("ens33", ip, mask, gateway, dns) 
		&& !m_pro->GetNetwork("eth0", ip, mask, gateway, dns))
#else
	int nr = m_pro->GetNetwork("eth0", ip, mask, gateway, dns);
	if (0 != nr)
#endif
	{
		printf("GetNetwork error = %d \n", nr);
		res = GetJson(1007, "can't get network info", data);
		return false;
	}

	using namespace rapidjson;
	//"data":{"ip":"192.168.1.10","mask":"255.255.255.0","gateway":"192.168.1.1","dns":"127.0.0.1"}

	StringBuffer buffer;
	PrettyWriter<StringBuffer> writer(buffer);

	writer.StartObject(); // root {

	writer.Key("retn");
	writer.Int(0);

	writer.Key("info");
	writer.String("success");

	writer.Key("data");
	writer.StartObject();

	writer.Key("ip");
	writer.String(ip);

	writer.Key("mask");
	writer.String(mask);

	writer.Key("gateway");
	writer.String(gateway);

	writer.Key("dns");
	writer.String(dns);

	writer.EndObject(); // data }

	writer.EndObject(); // root }

	res = buffer.GetString();

	MHC_DEBUG("GetNetwork");
	return true;
}

bool HttpThread::GetSetNetwork(std::string query, uint32_t inaddr, std::string token, std::string body, std::string& res)
{
	std::string data = "\"data\":{}";
	//if (inaddr != m_inaddr)
	//{
	//	res = GetJson(1004, "invalid token ip not login", data);
	//	return false;
	//}

	//if (m_szToken != token)
	//{
	//	res = GetJson(1002, "invalid token", data);
	//	return false;
	//}
	
	//{"ip":"192.168.1.10","mask":"255.255.255.0","gateway":"192.168.1.1","dns":"127.0.0.1"}
	doc.Parse<rapidjson::kParseStopWhenDoneFlag>(body.c_str());
	if (doc.HasParseError())
	{
		MHC_ERROR("Config file Error rapidjson Parse error\n");
		return false;
	}
	if (!doc.IsObject())
	{
		MHC_ERROR("Config file Error json not object \n");
		return false;
	}
	if (!doc.HasMember("ip") || !doc["ip"].IsString())
	{
		res = "{\"retn\":1006, \"info\":\"invalid parameter\", \"data\":null}";
		return false;
	}
	if (!doc.HasMember("mask") || !doc["mask"].IsString())
	{
		res = "{\"retn\":1006, \"info\":\"invalid parameter\", \"data\":null}";
		return false;
	}
	if (!doc.HasMember("gateway") || !doc["gateway"].IsString())
	{
		res = "{\"retn\":1006, \"info\":\"invalid parameter\", \"data\":null}";
		return false;
	}
	if (!doc.HasMember("dns") || !doc["dns"].IsString())
	{
		res = "{\"retn\":1006, \"info\":\"invalid parameter\", \"data\":null}";
		return false;
	}

	int nres = m_pro->SetNetwork("eth0", doc["ip"].GetString(), doc["mask"].GetString(), 
		doc["gateway"].GetString(), doc["dns"].GetString());

	switch (nres)
	{
	case -1:
		res = "{\"retn\":1006, \"info\":\"invalid ip\", \"data\":null}";
		return false;
		break;	
	case -2:
		res = "{\"retn\":1007, \"info\":\"can't set network\", \"data\":null}";
		return false;
		break;
	case -3:
		res = "{\"retn\":1007, \"info\":\"can't set dns\", \"data\":null}";
		return false;
		break;
	default:
	case 0:
		res = GeneralReturn();
		break;
	}

	MHC_INFO("SetNetwork");

	//reboot
	system("touch .re");

	return true;
}

bool HttpThread::GetGetMqtt(std::string query, uint32_t inaddr, std::string token, std::string body, std::string& res)
{
	std::string data = "\"data\":{}";
	//{"retn":0,"info":"success","data":{...]}}
	//if (inaddr != m_inaddr)
	//{
	//	res = GetJson(1004, "invalid token ip not login", data);
	//	return false;
	//}


	//"data":{"ip":"192.168.1.10", "port":1883,"user":"admin","password":"123456","topic":"ai.event"}
	using namespace rapidjson;

	StringBuffer buffer;
	PrettyWriter<StringBuffer> writer(buffer);

	writer.StartObject(); // root {

	writer.Key("retn");
	writer.Int(0);

	writer.Key("info");
	writer.String("success");

	writer.Key("data");
	writer.StartObject();

	writer.Key("ip");
	writer.String(m_config->stMqtt.ip.c_str());
	writer.Key("port");
	writer.Int(m_config->stMqtt.port);
	writer.Key("user");
	writer.String(m_config->stMqtt.user.c_str());
	writer.Key("password");
	writer.String(m_config->stMqtt.password.c_str());
	writer.Key("topic");
	writer.String(m_config->stMqtt.topic.c_str());

	writer.EndObject(); // data }

	writer.EndObject(); // root }

	res = buffer.GetString();

	MHC_DEBUG("GetNetwork");
	return true;
}

bool HttpThread::GetSetMqtt(std::string query, uint32_t inaddr, std::string token, std::string body, std::string& res)
{
	std::string data = "\"data\":{}";
	//{"retn":0,"info":"success","data":{...]}}
	//if (inaddr != m_inaddr)
	//{
	//	res = GetJson(1002, "invalid token ip not login", data);
	//	return false;
	//}

	//if (m_szToken != token)
	//{
	//	res = GetJson(1002, "invalid token", data);
	//	return false;
	//}
	//{"ip":"192.168.1.10", "port":1883,"user":"admin","password":"123456","topic":"ai.event"} 
	doc.Parse<rapidjson::kParseStopWhenDoneFlag>(body.c_str());
	if (doc.HasParseError())
	{
		MHC_ERROR("Config file Error rapidjson Parse error\n");
		return false;
	}
	if (!doc.IsObject())
	{
		MHC_ERROR("Config file Error json not object \n");
		return false;
	}
	if (!doc.HasMember("ip") || !doc["ip"].IsString())
	{
		res = "{\"retn\":1005, \"info\":\"invalid parameter\", \"data\":null}";
		return false;
	}
	if (!doc.HasMember("port") || !doc["port"].IsNumber())
	{
		res = "{\"retn\":1005, \"info\":\"invalid parameter\", \"data\":null}";
		return false;
	}
	if (!doc.HasMember("user") || !doc["user"].IsString())
	{
		res = "{\"retn\":1005, \"info\":\"invalid parameter\", \"data\":null}";
		return false;
	}
	if (!doc.HasMember("password") || !doc["password"].IsString())
	{
		res = "{\"retn\":1005, \"info\":\"invalid parameter\", \"data\":null}";
		return false;
	}
	if (!doc.HasMember("topic") || !doc["topic"].IsString())
	{
		res = "{\"retn\":1005, \"info\":\"invalid parameter\", \"data\":null}";
		return false;
	}

	g_pMain->GetConfig()->stMqtt.ip = doc["ip"].GetString();
	g_pMain->GetConfig()->stMqtt.port = doc["port"].GetInt();
	g_pMain->GetConfig()->stMqtt.user = doc["user"].GetString();
	g_pMain->GetConfig()->stMqtt.password = doc["password"].GetString();
	g_pMain->GetConfig()->stMqtt.topic = doc["topic"].GetString();

	g_pMain->GetConfig()->save();
	g_pMain->GetConfig()->load(NULL);
	res = GeneralReturn();
	return true;
}

bool HttpThread::GetGetDeviceStatus(std::string query, uint32_t inaddr, std::string token, std::string body, std::string& res)
{
	std::string data = "\"data\":{}";
	//{"retn":0,"info":"success","data":{...]}}
	//if (inaddr != m_inaddr)
	//{
	//	res = GetJson(1004, "invalid token ip not login", data);
	//	return false;
	//}

	//if (m_szToken != token)
	//{
	//	res = GetJson(1002, "invalid token", data);
	//	return false;
	//}

	//"data":{"temperature":36.7,"cpu":0.64,"tpu":0.75,"ram":2048,
	//"ramUsed":1024,"disk":8192,"diskUsed":1024,"sysTime":"2022-01-01 10:00:00","runTime":1000}
	float temperature;
	float cpu; 
	float tpu;
	int ram;
	int ramUsed;
	int disk;
	int diskUsed;
	std::string sysTime;
	int runTime;

	char tmp[1024] = { 0 };
	if (!m_pro->CheckSystem(temperature, cpu, tpu, ram, ramUsed, disk, diskUsed, sysTime, runTime))
	{
		res = "{\"retn\":1007, \"info\":\"interal error\", \"data\":null}";

		return false;
	}
	using namespace rapidjson;

	StringBuffer buffer;
	PrettyWriter<StringBuffer> writer(buffer);
	writer.SetMaxDecimalPlaces(2);
	writer.StartObject(); // root {

	writer.Key("retn");
	writer.Int(0);

	writer.Key("info");
	writer.String("success");

	writer.Key("data");
	writer.StartObject();

	writer.Key("temperature");
	writer.Double(temperature);
	writer.Key("cpu");
	writer.Double(cpu);
	writer.Key("tpu");
	writer.Double(tpu);
	writer.Key("ram");
	writer.Int(ram);
	writer.Key("ramUsed");
	writer.Int(ramUsed);
	writer.Key("disk");
	writer.Int(disk);
	writer.Key("diskUsed");
	writer.Int(diskUsed);
	writer.Key("sysTime");
	writer.String(sysTime.c_str());
	writer.Key("runTime");
	writer.Int(runTime);

	writer.EndObject(); // data }

	writer.EndObject(); // root }

	res = buffer.GetString();

	MHC_DEBUG("GetDeviceStatus");
	return true;
}

bool HttpThread::GetGetCameras(std::string query, uint32_t inaddr, std::string token, std::string body, std::string& res)
{
	std::string data = "\"data\":{}";
	//{"retn":0,"info":"success","data":{...]}}
	//if (inaddr != m_inaddr)
	//{
	//	res = GetJson(1004, "invalid token ip not login", data);
	//	return false;
	//}

	//if (m_szToken != token)
	//{
	//	res = GetJson(1002, "invalid token", data);
	//	return false;
	//}

/*
"data":{"cameras":[{"channel":1,"name":"dsfsd","ptzType":1,"imgType":1,"rtspUrl":"rtsp://192.168.1.21/av/0","status":1},
{"channel":2,"name":"meeting","ptzType":2,"imgType":1,"rtspUrl":"rtsp://192.168.1.22/av/0","status":1},
{"channel":3,"name":"office","ptzType":1,"imgType":1,"rtspUrl":"rtsp://192.168.1.23/av/0","status":1},
{"channel":4,"name":"testroom","ptzType":1,"imgType":2,"rtspUrl":"rtsp://192.168.1.24/av/0","status":0}]}
*/
	using namespace rapidjson;
	StringBuffer buffer;
	PrettyWriter<StringBuffer> writer(buffer);
	writer.SetMaxDecimalPlaces(2);
	writer.StartObject(); // root {

	writer.Key("retn");
	writer.Int(0);

	writer.Key("info");
	writer.String("success");

	writer.Key("data");
	writer.StartObject();

	writer.Key("cameras");
	writer.StartArray();  // cameras [
	
	for (Config::DeviceParam device : m_config->camera_infos)
	{
		writer.StartObject();
		writer.Key("channel");
		writer.Int(device.camera.chan_num);
		writer.Key("name");
		writer.String(device.camera.name.c_str());
		writer.Key("ptzType");
		writer.Int(device.camera.ptzType);
		writer.Key("imgType");
		writer.Int(device.camera.imgType);
		writer.Key("rtspUrl");
		writer.String(device.camera.address.c_str());
		writer.Key("status");
		switch (device.camera.chan_num)
		{
		case 1:
		default:
			writer.Int(m_pro->m_nOnline1);
			break;
		case 2:
			writer.Int(m_pro->m_nOnline2);
			break;
		case 3:
			writer.Int(m_pro->m_nOnline3);
			break;
		case 4:
			writer.Int(m_pro->m_nOnline4);
			break;
		}
		
		writer.EndObject();
	}
	writer.EndArray();  // cameras ]

	writer.EndObject(); // data }

	writer.EndObject(); // root }

	res = buffer.GetString();

	return true;
}

bool HttpThread::GetAddCamera(std::string query, uint32_t inaddr, std::string token, std::string body, std::string& res)
{
	std::string data = "\"data\":{}";
	//{"retn":0,"info":"success","data":{...]}}
	//if (inaddr != m_inaddr)
	//{
	//	res = GetJson(1004, "invalid token ip not login", data);
	//	return false;
	//}

	//if (m_szToken != token)
	//{
	//	res = GetJson(1002, "invalid token", data);
	//	return false;
	//}

	//{"channel":1,”name”:”ddd”,"ptzType":1,"imgType":1,"rtspUrl":"rtsp://..."}	//
	doc.Parse<rapidjson::kParseStopWhenDoneFlag>(body.c_str());
	if (doc.HasParseError())
	{
		MHC_ERROR("Config file Error rapidjson Parse error\n");
		return false;
	}
	if (!doc.IsObject())
	{
		MHC_ERROR("Config file Error json not object \n");
		return false;
	}
	if (!doc.HasMember("channel") || !doc["channel"].IsNumber())
	{
		res = "{\"retn\":1005, \"info\":\"invalid parameter\", \"data\":null}";
		return false;
	}
	if (!doc.HasMember("name") || !doc["name"].IsString())
	{
		res = "{\"retn\":1005, \"info\":\"invalid parameter\", \"data\":null}";
		return false;
	}
	if (!doc.HasMember("ptzType") || !doc["ptzType"].IsNumber())
	{
		res = "{\"retn\":1005, \"info\":\"invalid parameter\", \"data\":null}";
		return false;
	}
	if (!doc.HasMember("imgType") || !doc["imgType"].IsNumber())
	{
		res = "{\"retn\":1005, \"info\":\"invalid parameter\", \"data\":null}";
		return false;
	}
	if (!doc.HasMember("rtspUrl") || !doc["rtspUrl"].IsString())
	{
		res = "{\"retn\":1005, \"info\":\"invalid parameter\", \"data\":null}";
		return false;
	}

	Config::DeviceParam device;
	device.devid = doc["channel"].GetInt();
	device.camera.name = doc["name"].GetString();
	device.camera.ptzType = doc["ptzType"].GetInt();
	device.camera.imgType = doc["imgType"].GetInt();
	device.camera.address = doc["rtspUrl"].GetString();

	g_pMain->GetConfig()->AddChannel(device);

	res = GeneralReturn();
	return true;
}

bool HttpThread::GetDelCamera(std::string query, uint32_t inaddr, std::string token, std::string body, std::string& res)
{
	std::string data = "\"data\":{}";
	//{"retn":0,"info":"success","data":{...]}}
	//if (inaddr != m_inaddr)
	//{
	//	res = GetJson(1004, "invalid token ip not login", data);
	//	return false;
	//}

	//if (m_szToken != token)
	//{
	//	res = GetJson(1002, "invalid token", data);
	//	return false;
	//}

	//{"channel":1,”name”:”ddd”,"ptzType":1,"imgType":1,"rtspUrl":"rtsp://..."}	//
	doc.Parse<rapidjson::kParseStopWhenDoneFlag>(body.c_str());
	if (doc.HasParseError())
	{
		MHC_ERROR("Config file Error rapidjson Parse error\n");
		return false;
	}
	if (!doc.IsObject())
	{
		MHC_ERROR("Config file Error json not object \n");
		return false;
	}
	if (!doc.HasMember("channel") || !doc["channel"].IsNumber())
	{
		res = "{\"retn\":1005, \"info\":\"invalid parameter\", \"data\":null}";
		return false;
	}

	if (g_pMain->GetConfig()->DelChannel(doc["channel"].GetInt()))
	{
		res = GeneralReturn();
		return true;
	}
	else
	{
		res = "{\"retn\":1006, \"info\":\"not such channel id\", \"data\":null}";
		return false;
	}
}

bool HttpThread::GetSetRoi(std::string query, uint32_t inaddr, std::string token, std::string body, std::string& res)
{
	printf("GetSetRoi \n");
	std::string data = "\"data\":{}";
	//{"retn":0,"info":"success","data":{...]}}
	//if (inaddr != m_inaddr)
	//{
	//	res = GetJson(1004, "invalid token ip not login", data);
	//	return false;
	//}

	//if (m_szToken != token)
	//{
	//	res = GetJson(1002, "invalid token", data);
	//	return false;
	//}

	//{"channel":1,"rois":[{"roiId":1,"points":[768,126,1178,131,1194,512,698,494,768,126]},
	//{"roiId":2,"points":[682,586,1242,599,1322,1021,362,967,682,586]},
	//{"roiId":3,"points":[1478,100,1830,100,1901,972,1568,805,1478,100]}]}

	doc.Parse<rapidjson::kParseStopWhenDoneFlag>(body.c_str());
	if (doc.HasParseError())
	{
		MHC_ERROR("Config file Error rapidjson Parse error\n");
		return false;
	}
	if (!doc.IsObject())
	{
		MHC_ERROR("Config file Error json not object \n");
		return false;
	}
	if (!doc.HasMember("channel") || !doc["channel"].IsNumber())
	{
		res = "{\"retn\":1005, \"info\":\"invalid parameter\", \"data\":null}";
		return false;
	}

	std::vector<Config::RoiParam> rois;
	if (!doc.HasMember("rois") || !doc["rois"].IsArray())
	{
		res = "{\"retn\":1005, \"info\":\"invalid parameter\", \"data\":null}";
		return false;
	}

	for (int i = 0; i < doc["rois"].Size(); i++)
	{
		if (!doc["rois"][i].IsObject())
		{
			continue;
		}

		rapidjson::Value arr = doc["rois"][i].GetObjectX();
		Config::RoiParam roi;
		if (!arr.HasMember("roiId") || !arr["roiId"].IsNumber())
		{
			continue;
		}
		if (!arr.HasMember("points") || !arr["points"].IsArray() || arr["points"].Size() % 2 != 0)
		{
			continue;
		}
		Config::Pt pt;
		for (int n = 0; n < (int)arr["points"].Size(); n += 2)
		{
			if (arr["points"][n].IsNumber())
				pt.x = arr["points"][n].GetDouble();
			if (arr["points"][n + 1].IsNumber())
				pt.y = arr["points"][n + 1].GetDouble();
			roi.points.push_back(pt);
		}

		roi.roiId = arr["roiId"].GetInt();
		rois.push_back(roi);
	}

	if (g_pMain->GetConfig()->SetRois(doc["channel"].GetInt(), rois))
	{
		res = GeneralReturn();
		return true;
	}
	else
	{
		res = "{\"retn\":1006, \"info\":\"not such channel id\", \"data\":null}";
		return false;
	}
}

bool HttpThread::GetGetRoi(std::string query, uint32_t inaddr, std::string token, std::string body, std::string& res)
{
	printf("GetGetRoi \n");

	std::string data = "\"data\":{}";
	//{"retn":0,"info":"success","data":{...]}}
	//if (inaddr != m_inaddr)
	//{
	//	res = GetJson(1004, "invalid token ip not login", data);
	//	return false;
	//}

	//if (m_szToken != token)
	//{
	//	res = GetJson(1002, "invalid token", data);
	//	return false;
	//}

	doc.Parse<rapidjson::kParseStopWhenDoneFlag>(body.c_str());
	if (doc.HasParseError())
	{
		MHC_ERROR("Config file Error rapidjson Parse error\n");
		return false;
	}
	if (!doc.IsObject())
	{
		MHC_ERROR("Config file Error json not object \n");
		return false;
	}
	if (!doc.HasMember("channel") || !doc["channel"].IsNumber())
	{
		res = "{\"retn\":1005, \"info\":\"invalid parameter\", \"data\":null}";
		return false;
	}

	for (auto &inf : m_config->camera_infos)
	{
		if (inf.camera.chan_num != doc["channel"].GetInt())
		{
			continue;
		}

		//"data":{"rois":[{"roiId":1,"points":[768,126,1178,131,1194,512,698,494,768,126]},
		//{"roiId":2,"points":[682,586,1242,599,1322,1021,362,967,682,586]},
		//{"roiId":3,"points":[1478,100,1830,100,1901,972,1568,805,1478,100]}]}	
		using namespace rapidjson;
		StringBuffer buffer;
		PrettyWriter<StringBuffer> writer(buffer);
		writer.SetMaxDecimalPlaces(2);
		writer.StartObject(); // root {

		writer.Key("retn");
		writer.Int(0);

		writer.Key("info");
		writer.String("success");

		writer.Key("data");
		writer.StartObject();

		writer.Key("rois");
		writer.StartArray();  // rois [

		for (Config::RoiParam rio : inf.camera.rois)
		{
			writer.StartObject();
			writer.Key("roiId");
			writer.Int(rio.roiId);

			writer.Key("points");
			writer.StartArray();
			for (Config::Pt pt : rio.points)
			{
				writer.Double(pt.x);
				writer.Double(pt.y);
			}
			writer.EndArray();

			writer.EndObject();
		}
		writer.EndArray();  // cameras ]

		writer.EndObject(); // data }

		writer.EndObject(); // root }

		res = buffer.GetString();

		return true;
	}

	res = "{\"retn\":1006, \"info\":\"not such channel id\", \"data\":null}";
	return false;
}

bool HttpThread::GetGetSnapshot(std::string query, uint32_t inaddr, std::string token, std::string body, std::string& res)
{
	std::string data = "\"data\":{}";
	//{"retn":0,"info":"success","data":{...]}}
	//if (inaddr != m_inaddr)
	//{
	//	res = GetJson(1004, "invalid token ip not login", data);
	//	return false;
	//}

	//if (m_szToken != token)
	//{
	//	res = GetJson(1002, "invalid token", data);
	//	return false;
	//}

	doc.Parse<rapidjson::kParseStopWhenDoneFlag>(body.c_str());
	if (doc.HasParseError())
	{
		MHC_ERROR("Config file Error rapidjson Parse error\n");
		return false;
	}
	if (!doc.IsObject())
	{
		MHC_ERROR("Config file Error json not object \n");
		return false;
	}
	if (!doc.HasMember("channel") || !doc["channel"].IsNumber())
	{
		res = "{\"retn\":1005, \"info\":\"invalid parameter\", \"data\":null}";
		return false;
	}

	char ip[32] = { 0 };
	char mask[32] = { 0 };
	char gateway[32] = { 0 };
	char dns[32] = { 0 };
	m_pro->GetNetwork("eth0", ip, mask, gateway, dns);
	for (auto &inf : m_config->camera_infos)
	{
		if (inf.camera.chan_num != doc["channel"].GetInt())
		{
			continue;
		}

		//copy file nigx;
		std::string sourcePath, destPath;
		sourcePath = m_config->snapshotPath + "/" + std::to_string(doc["channel"].GetInt()) + ".jpg";
		printf("sourcePath = %s", m_config->snapshotPath.c_str());
		printf("destPath = %s", m_config->nginxImgPath.c_str());
		if (access(sourcePath.c_str(), F_OK) != 0)
		{
			res = "{\"retn\":1007, \"info\":\"snapshot file not exists\", \"data\":null}";
			return false;
		}
		//fileDestPath = m_config->nginxPath + "/snapshot" + std::to_string(doc["channel"].GetInt()) + ".jpg";
		//destPath = m_config->nginxPath + "/" + m_config->nginxImgPath + "/snapshot" + std::to_string(doc["channel"].GetInt()) + ".jpg";
		destPath = m_config->nginxPath + "/" + m_config->nginxImgPath + "/ch" + std::to_string(doc["channel"].GetInt()) + ".jpg";
		CopyFile(sourcePath, destPath);

		if (access(destPath.c_str(), F_OK) != 0)
		{
			res = "{\"retn\":1007, \"info\":\"not get snapshot file\", \"data\":null}";
			return false;
		}


		using namespace rapidjson;
		StringBuffer buffer;
		PrettyWriter<StringBuffer> writer(buffer);
		writer.SetMaxDecimalPlaces(2);
		writer.StartObject(); // root {

		writer.Key("retn");
		writer.Int(0);

		writer.Key("info");
		writer.String("success");

		writer.Key("data"); // data [
		writer.StartObject();

		writer.Key("snapshotUrl"); 
		
		std::string snapshotUrl = "http://" + std::string(ip) + "/" + m_config->nginxImgPath + "/ch" + std::to_string(doc["channel"].GetInt()) + ".jpg";
		writer.String(snapshotUrl.c_str());

		writer.EndObject(); // data }

		writer.EndObject(); // root }

		res = buffer.GetString();
		printf("GetGetSnapshot %s \n", res.c_str());

		//res = "\"data\":{\"snapshotUrl\":\"http://" + std::string(ip) + "/" + m_config->nginxImgPath + "/ch" + std::to_string(doc["channel"].GetInt()) + ".jpg\"}";
		return true;
	}

	res = "{\"retn\":1006, \"info\":\"not such channel id\", \"data\":null}";
	return false;
}

bool HttpThread::GetSetArmRule(std::string query, uint32_t inaddr, std::string token, std::string body, std::string& res)
{
	std::string data = "\"data\":{}";
	//{"retn":0,"info":"success","data":{...]}}
	//if (inaddr != m_inaddr)
	//{
	//	res = GetJson(1004, "invalid token ip not login", data);
	//	return false;
	//}

	//if (m_szToken != token)
	//{
	//	res = GetJson(1002, "invalid token", data);
	//	return false;
	//}
	//{"startTime":"00:00", "endTime":"23:59"}
	doc.Parse<rapidjson::kParseStopWhenDoneFlag>(body.c_str());
	if (doc.HasParseError())
	{
		MHC_ERROR("Config file Error rapidjson Parse error\n");
		return false;
	}
	if (!doc.IsObject())
	{
		MHC_ERROR("Config file Error json not object \n");
		return false;
	}
	if (!doc.HasMember("startTime") || !doc["startTime"].IsString())
	{
		res = "{\"retn\":1005, \"info\":\"invalid parameter\", \"data\":null}";
		return false;
	}

	if (!doc.HasMember("endTime") || !doc["endTime"].IsString())
	{
		res = "{\"retn\":1005, \"info\":\"invalid parameter\", \"data\":null}";
		return false;
	}

	if (!g_pMain->GetConfig()->SetArmRule(doc["startTime"].GetString(), doc["endTime"].GetString()))
	{
		res = "{\"retn\":1005, \"info\":\"invalid parameter\", \"data\":null}";
		return false;
	}
	res = GeneralReturn();
	return true;
}

bool HttpThread::GetGetArmRule(std::string query, uint32_t inaddr, std::string token, std::string body, std::string& res)
{
	std::string data = "\"data\":{}";
	//{"retn":0,"info":"success","data":{...]}}
	//if (inaddr != m_inaddr)
	//{
	//	res = GetJson(1004, "invalid token ip not login", data);
	//	return false;
	//}

	//if (m_szToken != token)
	//{
	//	res = GetJson(1002, "invalid token", data);
	//	return false;
	//}

	// "data":{"startTime":"00:00", "endTime":"23:59"}
	using namespace rapidjson;
	StringBuffer buffer;
	PrettyWriter<StringBuffer> writer(buffer);
	writer.SetMaxDecimalPlaces(2);
	writer.StartObject(); // root {

	writer.Key("retn");
	writer.Int(0);

	writer.Key("info");
	writer.String("success");

	writer.Key("data"); // data [
	writer.StartObject();

	writer.Key("startTime");
	writer.String(m_config->stGuard.startTime.c_str());
	writer.Key("endTime");
	writer.String(m_config->stGuard.endTime.c_str());

	writer.EndObject(); // data }

	writer.EndObject(); // root }

	res = buffer.GetString();

	return true;
}

bool HttpThread::GetSetAlgoRule(std::string query, uint32_t inaddr, std::string token, std::string body, std::string& res)
{
	/*
	retentionTime：目标滞留时间，单位秒
	alarmDelayedTime：报警间隔时间，单位秒
	objectSize：目标大小，单位为像素，即小于这个设定值的不做处理
	frameNbr：每秒处理帧率
	confidence：置信概率，小于设定值的不上报告警
	imageQuality：图像质量，小于设定值的不上报告警

		"retentionTime":3,
		"alarmDelayedTime":5,
		"objectSize":300,
		"frameNbr":10,
	*/

	std::string data = "\"data\":{}";
	//{"retn":0,"info":"success","data":{...]}}
	//if (inaddr != m_inaddr)
	//{
	//	res = GetJson(1004, "invalid token ip not login", data);
	//	return false;
	//}

	//if (m_szToken != token)
	//{
	//	res = GetJson(1002, "invalid token", data);
	//	return false;
	//}
	//{"startTime":"00:00", "endTime":"23:59"}
	doc.Parse<rapidjson::kParseStopWhenDoneFlag>(body.c_str());
	if (doc.HasParseError())
	{
		MHC_ERROR("Config file Error rapidjson Parse error\n");
		return false;
	}
	if (!doc.IsObject())
	{
		MHC_ERROR("Config file Error json not object \n");
		return false;
	}
	if (!doc.HasMember("retentionTime") || !doc["retentionTime"].IsNumber())
	{
		res = "{\"retn\":1005, \"info\":\"invalid parameter\", \"data\":null}";
		return false;
	}

	if (!doc.HasMember("alarmDelayedTime") || !doc["alarmDelayedTime"].IsNumber())
	{
		res = "{\"retn\":1005, \"info\":\"invalid parameter\", \"data\":null}";
		return false;
	}

	if (!doc.HasMember("objectSize") || !doc["objectSize"].IsNumber())
	{
		res = "{\"retn\":1005, \"info\":\"invalid parameter\", \"data\":null}";
		return false;
	}

	if (!doc.HasMember("frameNbr") || !doc["frameNbr"].IsNumber())
	{
		res = "{\"retn\":1005, \"info\":\"invalid parameter\", \"data\":null}";
		return false;
	}

	if (!doc.HasMember("confidence") || !doc["confidence"].IsNumber())
	{
		res = "{\"retn\":1005, \"info\":\"invalid parameter\", \"data\":null}";
		return false;
	}
	if (!doc.HasMember("imageQuality") || !doc["imageQuality"].IsNumber())
	{
		res = "{\"retn\":1005, \"info\":\"invalid parameter\", \"data\":null}";
		return false;
	}
	/*
		writer.Key("retentionTime");
	writer.Int(m_config->camera_infos[0].camera.retentionTime);
	writer.Key("alarmDelayedTime");
	writer.Int(m_config->camera_infos[0].camera.alarmDelayedTime);
	writer.Key("objectSize");
	writer.Int(m_config->camera_infos[0].camera.objectSize);
	writer.Key("frameNbr");
	writer.Int(m_config->camera_infos[0].camera.frameNbr);
	writer.Key("confidence");
	writer.Int(m_config->camera_infos[0].camera.confidence);
	writer.Key("imageQuality");
	writer.Int(m_config->camera_infos[0].camera.imageQuality);

	*/


	if (!g_pMain->GetConfig()->SetAlgoRule(doc["retentionTime"].GetInt(), doc["alarmDelayedTime"].GetInt(), 
		doc["objectSize"].GetInt(), doc["frameNbr"].GetInt(), 
		doc["confidence"].GetDouble(), doc["imageQuality"].GetDouble()) )
	{
		res = "{\"retn\":1007, \"info\":\"save error\", \"data\":null}";
		return false;
	}

	res = GeneralReturn();
	return true;
}

bool HttpThread::GetGetAlgoRule(std::string query, uint32_t inaddr, std::string token, std::string body, std::string& res)
{
	std::string data = "\"data\":{}";
	//if (inaddr != m_inaddr)
	//{
	//	res = GetJson(1004, "invalid token ip not login", data);
	//	return false;
	//}

	//if (m_szToken != token)
	//{
	//	res = GetJson(1002, "invalid token", data);
	//	return false;
	//}

	// "data":{"startTime":"00:00", "endTime":"23:59"}
	if (m_config->camera_infos.size() == 0)
	{
		res = "{\"retn\":1007, \"info\":\"inter error\", \"data\":null}";
		return false;
	}

	using namespace rapidjson;
	StringBuffer buffer;
	PrettyWriter<StringBuffer> writer(buffer);
	writer.SetMaxDecimalPlaces(2);
	writer.StartObject(); // root {

	writer.Key("retn");
	writer.Int(0);

	writer.Key("info");
	writer.String("success");

	writer.Key("data"); // data [
	writer.StartObject();

	writer.Key("retentionTime");
	writer.Int(m_config->camera_infos[0].camera.retentionTime);
	writer.Key("alarmDelayedTime");
	writer.Int(m_config->camera_infos[0].camera.alarmDelayedTime);
	writer.Key("objectSize");
	writer.Int(m_config->camera_infos[0].camera.objectSize);
	writer.Key("frameNbr");
	writer.Int(m_config->camera_infos[0].camera.frameNbr);
	writer.Key("confidence");
	writer.Int(m_config->camera_infos[0].camera.confidence);
	writer.Key("imageQuality");
	writer.Int(m_config->camera_infos[0].camera.imageQuality);

	writer.EndObject(); // data }

	writer.EndObject(); // root }

	res = buffer.GetString();

	return true;

	return false;
}

bool HttpThread::GetCount(std::string query, uint32_t inaddr, std::string token, std::string body, std::string& res)
{
	//todo
	std::string data = "\"data\":{}";

	//if (inaddr != m_inaddr)
	//{
	//	res = GetJson(1004, "invalid token ip not login", data);
	//	return false;
	//}
	printf("GetCount %s \n", body.c_str());

	int ch = 0;
	doc.Parse<rapidjson::kParseStopWhenDoneFlag>(body.c_str());
	if (doc.HasParseError())
	{
		MHC_ERROR("Config file Error rapidjson Parse error\n");
		return false;
	}
	if (!doc.IsObject())
	{
		MHC_ERROR("Config file Error json not object \n");
		return false;
	}
	if (!doc.HasMember("channel") || !doc["channel"].IsNumber())
	{
		res = "{\"retn\":1005, \"info\":\"invalid parameter\", \"data\":null}";
		return false;
	}
	ch = doc["channel"].GetInt();
	int count = 0;
	m_pro->GetCount(m_config->alarmImagePath, ch, count);

	//"data":{"count":1000}

	using namespace rapidjson;
	StringBuffer buffer;
	PrettyWriter<StringBuffer> writer(buffer);
	writer.SetMaxDecimalPlaces(2);
	writer.StartObject(); // root {

	writer.Key("retn");
	writer.Int(0);

	writer.Key("info");
	writer.String("success");

	writer.Key("data"); // data [
	writer.StartObject();

	writer.Key("count");
	writer.Int(count);

	writer.EndObject(); // data }

	writer.EndObject(); // root }

	res = buffer.GetString();
	printf("GetCount %s \n", res.c_str());

	return true;
}

bool HttpThread::GetQuery(std::string query, uint32_t inaddr, std::string token, std::string body, std::string& res)
{
	//todo
	std::string data = "\"data\":{}";
	//if (inaddr != m_inaddr)
	//{
	//	res = GetJson(1004, "invalid token ip not login", data);
	//	return false;
	//}
	int ch = 0;

	doc.Parse<rapidjson::kParseStopWhenDoneFlag>(body.c_str());
	if (doc.HasParseError())
	{
		MHC_ERROR("Config file Error rapidjson Parse error\n");
		return false;
	}
	if (!doc.IsObject())
	{
		MHC_ERROR("Config file Error json not object \n");
		return false;
	}
	if (!doc.HasMember("channel") || !doc["channel"].IsNumber())
	{
		res = "{\"retn\":1005, \"info\":\"invalid parameter\", \"data\":null}";
		return false;
	}
	if (!doc.HasMember("pageNbr") || !doc["pageNbr"].IsNumber())
	{
		res = "{\"retn\":1005, \"info\":\"invalid parameter\", \"data\":null}";
		return false;
	}

	if (!doc.HasMember("pageSize") || !doc["pageSize"].IsNumber())
	{
		res = "{\"retn\":1005, \"info\":\"invalid parameter\", \"data\":null}";
		return false;
	}

	int channel = doc["channel"].GetInt();
	int pageNbr = doc["pageNbr"].GetInt();
	int pageSize = doc["pageSize"].GetInt();
	/*
		//{“channel”:0,"pageNbr":1, "pageSize":10}
	{   "retn":0,
		"info":"success",
		"data":[{"boxId":"bm1", "domain":"ai.alarm", "channel":1,"createTime":"2022-01-01 10:00:00","objectTypes":1,
		"confidence":0.2, "imageQuality": 0.1, "snapshotUrl":"http://192.168.1.100/images/alarm_202207010_0085.jpg",
		"info":{}
		}, {"boxId":"bm1", "domain":"ai.alarm", "channel":1,"createTime":"2022-01-01 10:00:00","objectTypes":1,
		"confidence":0.2, "imageQuality": 0.1, "snapshotUrl":"http://192.168.1.100/images/alarm_202207010_0085.jpg",
		"info":{}
		}]
	}
	*/
	std::vector<algParam> params;
	m_pro->GetAlgParam(m_config->alarmImagePath, channel, pageNbr, pageSize, params);
	using namespace rapidjson;
	StringBuffer buffer;
	PrettyWriter<StringBuffer> writer(buffer);
	writer.SetMaxDecimalPlaces(2);
	writer.StartObject(); // root {

	writer.Key("retn");
	writer.Int(0);

	writer.Key("info");
	writer.String("success");

	writer.Key("data"); // data [
	writer.StartArray();

	for (algParam & param : params)
	{
		writer.StartObject();  // {
		writer.Key("boxId");
		writer.String(m_config->stBase.boxId.c_str());

		writer.Key("domain");
		writer.String("ai.alarm");

		writer.Key("channel");
		writer.Int(param.channel);
		writer.Key("createTime");
		writer.String(param.createTime.c_str());
		writer.Key("objectTypes");
		writer.Int(param.objectTypes);
		writer.Key("confidence");
		writer.Double(param.confidence);
		writer.Key("imageQuality");
		writer.Double(param.imageQuality);
		writer.Key("snapshotUrl");
		writer.String(param.snapshotUrl.c_str());

		writer.EndObject();  //}
	}

	writer.EndArray(); // data }

	writer.EndObject(); // root }

	res = buffer.GetString();
	return true;
}


void HttpThread::HttpProcess()
{
	auto tsNowEnd = MHC_STLNOW_MILLISEC;// std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

bool HttpThread::start()
{
	m_thread = std::thread(&HttpThread::MainLoop, this);
//	std::string szName = "LprDetect-" + std::to_string(m_nThreadNum);
//	setThreadName(m_thread.native_handle(), szName.c_str());

	m_blInit = true;
//	log_print(ZH_LOG_INFO, "HttpThread::start %s! \n", szName.c_str());
	return m_blInit;
}

void HttpThread::stop(bool blExit)
{
	m_blRunning = false;

	//if (blExit)
//	log_print(ZH_LOG_INFO, "HttpThread::stop thread =%d! \n", m_nThreadNum);
}

void HttpThread::SetConfig()
{
	m_config = g_pMain->GetConfig();
}

void HttpThread::SetProCmd()
{
	m_pro = g_pMain->GetProcCmd();
}
