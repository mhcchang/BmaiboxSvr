#include "Config.h"
#include "utils.h"
#include "logger.h"
#include <fstream>
#include <unordered_map>
#include <unordered_set>

#include <fstream>
#include <experimental/filesystem>

#include <document.h>
#include "stringbuffer.h"
#include "prettywriter.h"
#include "ostreamwrapper.h"

// #define LOG_FATAL_P1_NOFOUND(x); \
// 		MHC_LOG_FMT_LEVEL(SVR_NAME, FATAL, "Config file Error json %s node not found\n", x);
// #define LOG_FATAL_P1_NOOBJECT(x); \
// 		MHC_LOG_FMT_LEVEL(SVR_NAME, FATAL, "Config file Error json %s node not object\n", x);

// #define LOG_FATAL_P2_NOFOUND(x, y); \
// 		MHC_LOG_FMT_LEVEL(SVR_NAME, FATAL, "Config file Error json %s node is not found sub node %s\n", x, y);
// #define LOG_FATAL_P2_NOOBJECT(x, y); \
// 		MHC_LOG_FMT_LEVEL(SVR_NAME, FATAL, "Config file Error json %s node subnode[%s] is not object\n", x, y);
#define LOG_FATAL_P1_NOFOUND(x); \
		MHC_CRITICAL("Config file Error json {} node not found\n", x);
#define LOG_FATAL_P1_NOOBJECT(x); \
		MHC_CRITICAL("Config file Error json {} node not object\n", x);

#define LOG_FATAL_P2_NOFOUND(x, y); \
		MHC_CRITICAL("Config file Error json {} node is not found sub node {}\n", x, y);
#define LOG_FATAL_P2_NOOBJECT(x, y); \
		MHC_CRITICAL("Config file Error json {} node subnode {} is not object\n", x, y);

namespace std {
	namespace fs = std::experimental::filesystem;
}

//#define DEFAULT_RETENTION_PERIOD	7
//#define DEFAULT_FILE_DURATION		5
//#define DEFAULT_FREE_DISK_SPACE_LIMIT	25

static void replace(std::string& str, const std::string& from, const std::string& to) 
{
	if (from.empty())
		return;

	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos)
	{
		str.replace(start_pos, from.length(), to);
		start_pos += to.length();
	}
}

std::vector<std::string> str_split(const std::string& in, const std::string& delim)
{
	std::regex re{ delim };
	return std::vector<std::string>	{ std::sregex_token_iterator(in.begin(), in.end(), re, -1), std::sregex_token_iterator() };
}

std::string Config::readFile(const char* configPath)
{
	std::ifstream ifs(configPath);
	if (!ifs)
		return std::string();

	ifs.seekg(0, std::ios::end);
	std::size_t len = ifs.tellg();
	char* buf = new char[len + 1];
	memset(buf, 0, len + 1);
	ifs.seekg(0, std::ios::beg);
	ifs.read(buf, len);

	std::string content(buf, len);
	delete [] buf;

	return content;
}

void Config::changeEventImp(int64_t id, const std::set<std::pair<std::wstring, uint32_t> >& notifications)
{
	std::wstring sz;
	for (const auto& notif : notifications) 
	{
		sz = L"Change on watcher with ID=" + std::to_wstring(id) + L", relative path: \"" + notif.first.c_str()
			+ L"\"event: " + std::to_wstring(notif.second).c_str() + L"\n";
		//std::wcout << L"Change on watcher with ID=" << id
		//	<< L", relative path: \"" << notif.first.c_str() << L"\""
		//	L"event: " << std::to_wstring(notif.second).c_str() << std::endl;
		//OutputDebugStringW(sz.c_str());
		if (m_wFileName == notif.first)
		{
			//std::wcout << " !!!! Get change event " << m_wFileName << std::endl;
			sz = L"!!!! Get change event " + m_wFileName;
			//OutputDebugStringW(sz.c_str());
			load(nullptr);

			MHC_ERROR( "Config file reload\n");
			//锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷息
			//if (m_pUpdateCallback)
			//	m_pUpdateCallback();
		}
	}
}

bool Config::Init(ChangeEvent changeEvent)
{
	return true;
}

bool Config::load(const char* configPath)
{
	if (configPath == nullptr || strlen(configPath) == 0)
		configPath = CONFIG_FILE_PATH;
	m_configPath = (char*)configPath;
	printf("Config file %s\n", m_configPath);
	std::string content = readFile(configPath);
	if (content.empty())
	{
		MHC_ERROR("Config file Error readFile error\n");
		return false;
	}

	this->configPath = configPath;

	rapidjson::Document doc;
	doc.Parse<rapidjson::kParseStopWhenDoneFlag>(content.c_str());

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

	if (!doc.HasMember("httpsvr") || !doc["httpsvr"].IsObject())
	{
		MHC_ERROR("Config file Error json no httpsvr node\n");
		return false;
	}
	//------------httpsvr
	if (doc["httpsvr"].HasMember("port") && doc["httpsvr"]["port"].IsNumber())
		stHttp.port = doc["httpsvr"]["port"].GetInt();
	else
	{
		MHC_ERROR("Config file Error json no stHttp port node\n");
		return false;
	}

	if (!doc.HasMember("base") || !doc["base"].IsObject())
	{
		MHC_ERROR("Config file Error json no base node\n");
		return false;
	}
	//------------base
	if (doc["base"].HasMember("boxId") && doc["base"]["boxId"].IsString())
		stBase.boxId = doc["base"]["boxId"].GetString();
	else
	{
		MHC_ERROR("Config file Error json no boxId node\n");
		return false;
	}
	if (doc["base"].HasMember("model") && doc["base"]["model"].IsString())
		stBase.model = doc["base"]["model"].GetString();
	else
	{
		MHC_ERROR("Config file Error json no model node\n");
		return false;
	}
	if (doc["base"].HasMember("snbr") && doc["base"]["snbr"].IsString())
		stBase.snbr = doc["base"]["snbr"].GetString();
	else
	{
		MHC_ERROR("Config file Error json no snbr node\n");
		return false;
	}
	if (doc["base"].HasMember("softVer") && doc["base"]["softVer"].IsString())
		stBase.softVer = doc["base"]["softVer"].GetString();
	else
	{
		MHC_ERROR("Config file Error json no softVer node\n");
		return false;
	}
	if (doc["base"].HasMember("algoVer") && doc["base"]["algoVer"].IsString())
		stBase.algoVer = doc["base"]["algoVer"].GetString();
	else
	{
		MHC_ERROR("Config file Error json no algoVer node\n");
		return false;
	}

	//------------ntp
	if (!doc.HasMember("ntp") || !doc["ntp"].IsObject())
	{
		MHC_ERROR("Config file Error json no ntp node\n");
		return false;
	}
	if (doc["ntp"].HasMember("used") && doc["ntp"]["used"].IsBool())
		stNtp.used = doc["ntp"]["used"].GetBool();
	else
	{
		MHC_ERROR("Config file Error json no ntp used node\n");
		return false;
	}
	if (doc["ntp"].HasMember("ip") && doc["ntp"]["ip"].IsString())
		stNtp.ip = doc["ntp"]["ip"].GetString();
	else
	{
		MHC_ERROR("Config file Error json no stNtp ip node\n");
		return false;
	}

	//------------mqtt
	if (!doc.HasMember("mqtt") || !doc["mqtt"].IsObject())
	{
		MHC_ERROR("Config file Error json no mqtt node\n");
		return false;
	}
	if (doc["mqtt"].HasMember("ip") && doc["mqtt"]["ip"].IsString())
		stMqtt.ip = doc["mqtt"]["ip"].GetString();
	else
		stMqtt.ip = "0.0.0.0";

	if (doc["mqtt"].HasMember("port") && doc["mqtt"]["port"].IsInt())
		stMqtt.port = doc["mqtt"]["port"].GetInt();
	else
		stMqtt.port = 1883;
	if (doc["mqtt"].HasMember("user") && doc["mqtt"]["user"].IsString())
		stMqtt.user = doc["mqtt"]["user"].GetString();
	else
		stMqtt.user = "admin";
	if (doc["mqtt"].HasMember("password") && doc["mqtt"]["password"].IsString())
		stMqtt.password = doc["mqtt"]["password"].GetString();
	else
		stMqtt.password = "123456";
	if (doc["mqtt"].HasMember("topic") && doc["mqtt"]["topic"].IsString())
		stMqtt.topic = doc["mqtt"]["topic"].GetString();
	else
		stMqtt.topic = "ai.event";
	if (doc["mqtt"].HasMember("user") && doc["mqtt"]["user"].IsString())
		stMqtt.user = doc["mqtt"]["user"].GetString();
	else
		stMqtt.user = "admin";

	//cameras
	if (!doc.HasMember("camera_info"))
	{
		MHC_ERROR("Config file Error  json no cameras node\n");
		return false;
	}
	if (!doc["camera_info"].IsArray())
	{
		MHC_ERROR("Config file Error json cameras node is not array\n");
		return false;
	}
	else
	{
		rapidjson::Value arrDev = doc["camera_info"].GetArray();
		int ll = arrDev.Size();
		for (int k = 0; k < (int)arrDev.Size(); k++)
		{
			if (arrDev[k].IsObject())
			{
				DeviceParam dev;
				if (arrDev[k].HasMember("devid") && arrDev[k]["devid"].IsNumber())
					dev.devid = arrDev[k]["devid"].GetInt();
				else
					continue;
				
				if (!arrDev[k].HasMember("cameras") || !arrDev[k]["cameras"].IsObject())
				{
					continue;
				}

				rapidjson::Value cameraObj = arrDev[k]["cameras"].GetObjectX();
				if (cameraObj.HasMember("address") && cameraObj["address"].IsString())
					dev.camera.address = cameraObj["address"].GetString();
				else
					continue;

				if (cameraObj.HasMember("chan_num") && cameraObj["chan_num"].IsNumber())
					dev.camera.chan_num = cameraObj["chan_num"].GetInt();
				else
					continue;
				if (cameraObj.HasMember("ptzType") && cameraObj["ptzType"].IsNumber())
					dev.camera.ptzType = cameraObj["ptzType"].GetInt();
				else
					continue;
				if (cameraObj.HasMember("imgType") && cameraObj["imgType"].IsNumber())
					dev.camera.imgType = cameraObj["imgType"].GetInt();
				else
					continue;
				if (cameraObj.HasMember("algSensitivity") && cameraObj["algSensitivity"].IsNumber())
					dev.camera.algSensitivity = cameraObj["algSensitivity"].GetDouble();
				else
					continue;
				if (cameraObj.HasMember("vqaSensitivity") && cameraObj["vqaSensitivity"].IsNumber())
					dev.camera.vqaSensitivity = cameraObj["vqaSensitivity"].GetDouble();
				else
					continue;
				if (cameraObj.HasMember("retentionTime") && cameraObj["retentionTime"].IsNumber())
					dev.camera.retentionTime = cameraObj["retentionTime"].GetInt();
				else
					dev.camera.retentionTime = 3;
				if (cameraObj.HasMember("alarmDelayedTime") && cameraObj["alarmDelayedTime"].IsNumber())
					dev.camera.alarmDelayedTime = cameraObj["alarmDelayedTime"].GetInt();
				else
					dev.camera.alarmDelayedTime = 5;
				if (cameraObj.HasMember("objectSize") && cameraObj["objectSize"].IsNumber())
					dev.camera.objectSize = cameraObj["objectSize"].GetInt();
				else
					dev.camera.objectSize = 300;
				if (cameraObj.HasMember("frameNbr") && cameraObj["frameNbr"].IsNumber())
					dev.camera.frameNbr = cameraObj["frameNbr"].GetInt();
				else
					dev.camera.frameNbr = 10;
				//todo!!!
				if (cameraObj.HasMember("confidence") && cameraObj["confidence"].IsNumber())
					dev.camera.confidence = cameraObj["confidence"].GetInt();
				else
					dev.camera.confidence = 0.7;
				if (cameraObj.HasMember("imageQuality") && cameraObj["confidence"].IsNumber())
					dev.camera.imageQuality = cameraObj["imageQuality"].GetInt();
				else
					dev.camera.imageQuality = 0.7;
				if (cameraObj.HasMember("name") && cameraObj["name"].IsString())
					dev.camera.name = cameraObj["name"].GetString();
				if (cameraObj.HasMember("width") && cameraObj["width"].IsNumber())
					dev.camera.width = cameraObj["width"].GetInt();
				else
					continue;
				if (cameraObj.HasMember("height") && cameraObj["height"].IsNumber())
					dev.camera.height = cameraObj["height"].GetInt();
				else
					continue;

				if (cameraObj.HasMember("rois") && cameraObj["rois"].IsArray())
				{
					rapidjson::Value arrRois = cameraObj["rois"].GetArray();
					
					for (int k = 0; k < (int)arrRois.Size(); k++)
					{
						RoiParam roi;
						if (!arrRois[k].IsObject())
						{
							continue;
						}

						if (arrRois[k].HasMember("roiId") && arrRois[k]["roiId"].IsNumber())
						{
							roi.roiId = arrRois[k]["roiId"].GetInt();
						}

						if (!arrRois[k].HasMember("points") || !arrRois[k]["points"].IsArray())
						{ 
							continue;
						}

						rapidjson::Value arrx2 = arrRois[k]["points"].GetArray();
						Pt pt;
						if (arrx2.Size() % 2 != 0)
							continue;
						for (int n = 0; n < (int)arrx2.Size(); n += 2)
						{
							if (arrx2[n].IsNumber())
								pt.x = arrx2[n].GetDouble();
							if (arrx2[n + 1].IsNumber())
								pt.y = arrx2[n + 1].GetDouble();
							roi.points.push_back(pt);
						}

						dev.camera.rois.push_back(roi);
					}
				}

				camera_infos.push_back(dev);
			}
		}
	}

	//roi
	//if (!doc.HasMember("rois"))
	//{
	//	MHC_ERROR("Config file Error  json no rois node\n");
	//	return false;
	//}
	//if (!doc["rois"].IsArray())
	//{
	//	MHC_ERROR("Config file Error json rois node is not array\n");
	//	return false;
	//}
	//else
	//{
	//	rapidjson::Value arrx1 = doc["rois"].GetArray();
	//	int ll = arrx1.Size();
	//	for (int k = 0; k < (int)arrx1.Size(); k++)
	//	{
	//		if (arrx1[k].IsObject())
	//		{
	//			RuleParam roi;
	//			if (arrx1.HasMember("roiId") && arrx1["roiId"].IsNumber())
	//				roi.roiId = arrx1["roiId"].GetInt();
	//			else
	//				continue;

	//			if (arrx1.HasMember("points") && arrx1["points"].IsArray())
	//			{
	//				rapidjson::Value arrx2 = arrx1["points"].GetArray();
	//				Pt pt;
	//				if (arrx2.Size() % 2 != 0)
	//					continue;
	//				for (int n = 0; n < (int)arrx2.Size(); n+=2)
	//				{
	//					if (arrx2[n].IsNumber())
	//						pt.x = arrx2[n].GetDouble();
	//					if (arrx2[n + 1].IsNumber())
	//						pt.y = arrx2[n + 1].GetDouble();
	//					roi.points.push_back(pt);
	//				}
	//			}
	//			else
	//				continue;

	//			rules.push_back(roi);
	//		}
	//	}
	//}

	if (!doc.HasMember("guard"))
	{
		MHC_ERROR("Config file Error  json no rois node\n");
		return false;
	}
	if (!doc["guard"].IsObject())
	{
		MHC_ERROR("Config file Error json rois node is not array\n");
		return false;
	}
	if (doc["guard"].HasMember("startTime") && doc["guard"]["startTime"].IsString())
		stGuard.startTime = doc["guard"]["startTime"].GetString();
	if (doc["guard"].HasMember("endTime") && doc["guard"]["endTime"].IsString())
		stGuard.endTime = doc["guard"]["endTime"].GetString();

	if (doc.HasMember("private") && doc["private"].IsObject())
	{
		if (doc["private"].HasMember("logLevel") && doc["private"]["logLevel"].IsString())
			stPrivate.logLevel = doc["private"]["logLevel"].GetString();
	}

	if (doc.HasMember("algVersionPath") && doc["algVersionPath"].IsString())
	{
		algVersionPath = doc["algVersionPath"].GetString();
	}
	if (doc.HasMember("classifyModelPath") && doc["classifyModelPath"].IsString())
	{
		classifyModelPath = doc["classifyModelPath"].GetString();
	}	
	if (doc.HasMember("exeAlgPath") && doc["exeAlgPath"].IsString())
	{
		exeAlgPath = doc["exeAlgPath"].GetString();
	}	
	if (doc.HasMember("detectModelPath") && doc["detectModelPath"].IsString())
	{
		detectModelPath = doc["detectModelPath"].GetString();
	}	
	if (doc.HasMember("snapshotPath") && doc["snapshotPath"].IsString())
	{
		snapshotPath = doc["snapshotPath"].GetString();
	}	
	if (doc.HasMember("alarmImagePath") && doc["alarmImagePath"].IsString())
	{
		alarmImagePath = doc["alarmImagePath"].GetString();
	}
	if (doc.HasMember("softUpdatePath") && doc["softUpdatePath"].IsString())
	{
		softUpdatePath = doc["softUpdatePath"].GetString();
	}
	if (doc.HasMember("algoUpdatePath") && doc["algoUpdatePath"].IsString())
	{
		algoUpdatePath = doc["algoUpdatePath"].GetString();
	}
	if (doc.HasMember("channelStatusPath") && doc["channelStatusPath"].IsString())
	{
		channelStatusPath = doc["channelStatusPath"].GetString();
	}
	if (doc.HasMember("nginxPath") && doc["nginxPath"].IsString())
	{
		nginxPath = doc["nginxPath"].GetString();
	}
	else
		nginxPath = "/usr/local/nginx/sbin/";
	if (doc.HasMember("nginxImgPath") && doc["nginxImgPath"].IsString())
	{
		nginxImgPath = doc["nginxImgPath"].GetString();
	}
	else
		nginxImgPath = "/usr/local/nginx/html/static/";

	return true;
}

std::string Config::getBoxId()
{
	return stBase.boxId;
}

bool Config::getNtpStatus()
{
	return stNtp.used;
}

std::string Config::getNtpIp()
{
	return stNtp.ip;
}

Config::Config()
{
	m_blInit = false;
}

bool Config::setBoxId(std::string boxId)
{
	std::string content = readFile(CONFIG_FILE_PATH);
	if (content.empty())
	{
		MHC_ERROR("Config file Error readFile error\n");
		return false;
	}
	rapidjson::Document doc;
	doc.Parse<rapidjson::kParseStopWhenDoneFlag>(content.c_str());

	std::ofstream ofs(CONFIG_FILE_PATH);
	if (!ofs.is_open())
		return false;

	rapidjson::OStreamWrapper osw(ofs);
	rapidjson::PrettyWriter<rapidjson::OStreamWrapper> writer(osw);

	rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

	doc["base"]["boxId"].SetString(boxId.c_str(), boxId.length());

	//doc.SetObject();

	//rapidjson::Value baseObj(rapidjson::kObjectType);
	//baseObj.AddMember("retn", 0, allocator);
	//baseObj.AddMember("info", "success", allocator);

	//rapidjson::Value data(rapidjson::kObjectType);
	//data.AddMember("boxId", boxId, allocator);

	//baseObj.AddMember("data", data, allocator);

	//doc.AddMember("base", baseObj, allocator);

	//rapidjson::Value externeObj(rapidjson::kObjectType);
	//for (auto &it = modes.begin(); it != modes.end(); it++)
	//{
	//	rapidjson::Value modeRoadObj(rapidjson::kObjectType);
	//	modeRoadObj.AddMember("threadNum", it->second.threadNum, allocator);
	//	modeRoadObj.AddMember("interval", it->second.interval, allocator);
	//	modeRoadObj.AddMember("firstOutResult", it->second.firstOutResult, allocator);
	//	modeRoadObj.AddMember("weightFrame", it->second.weightFrame, allocator);
	//	modeRoadObj.AddMember("missMillsecond", it->second.missMillsecond, allocator);
	//	externeObj.AddMember(rapidjson::StringRef(it->first.c_str()), modeRoadObj, allocator);
	//}

	//doc.AddMember("externe", externeObj, allocator);

	////multiTrackMq 
	//rapidjson::Value consumerMqObj(rapidjson::kObjectType);
	//consumerMqObj.AddMember("host", consumer.host, allocator);
	//consumerMqObj.AddMember("port", consumer.port, allocator);
	//consumerMqObj.AddMember("username", consumer.username, allocator);
	//consumerMqObj.AddMember("password", consumer.password, allocator);
	//consumerMqObj.AddMember("path", consumer.path, allocator);
	//consumerMqObj.AddMember("queuename", consumer.queuename, allocator);
	//rapidjson::Value consumerMqexchangeObj(rapidjson::kArrayType);
	//for (auto &exchangename : consumer.exchangename)
	//{
	//	rapidjson::Value strVal;
	//	strVal.SetString(exchangename.c_str(), exchangename.length(), allocator);
	//	consumerMqexchangeObj.PushBack(strVal, allocator);
	//}
	//consumerMqObj.AddMember("exchangename", consumerMqexchangeObj, allocator);

	//consumerMqObj.AddMember("consumertag", consumer.consumertag, allocator);

	//rapidjson::Value consumerMqroutingkeyObj(rapidjson::kArrayType);
	//for (auto &itroutingkey : consumer.routingkey)
	//{
	//	rapidjson::Value strVal;
	//	strVal.SetString(itroutingkey.c_str(), itroutingkey.length(), allocator);
	//	consumerMqroutingkeyObj.PushBack(strVal, allocator);
	//}
	//consumerMqObj.AddMember("routingkey", consumerMqroutingkeyObj, allocator);
	//consumerMqObj.AddMember("missqueuename", consumer.missqueuename, allocator);
	////missRoutingkey
	//rapidjson::Value consumerMqmissRoutingkeyObj(rapidjson::kArrayType);
	//for (auto &itmissroutingkey : consumer.missRoutingkey)
	//{
	//	rapidjson::Value strVal;
	//	strVal.SetString(itmissroutingkey.c_str(), itmissroutingkey.length(), allocator);
	//	consumerMqmissRoutingkeyObj.PushBack(strVal, allocator);
	//}

	//consumerMqObj.AddMember("missRoutingkey", consumerMqmissRoutingkeyObj, allocator);
	//consumerMqObj.AddMember("publisherConfirms", consumer.publisherconfirms, allocator);

	//doc.AddMember("multiTrackMq", consumerMqObj, allocator);

	////publisherMq
	//rapidjson::Value publisherMqObj(rapidjson::kObjectType);
	//publisherMqObj.AddMember("host", producer.host, allocator);
	//publisherMqObj.AddMember("port", producer.port, allocator);
	//publisherMqObj.AddMember("username", producer.username, allocator);
	//publisherMqObj.AddMember("password", producer.password, allocator);
	//publisherMqObj.AddMember("path", producer.path, allocator);
	//rapidjson::Value publisherMqexchangeObj(rapidjson::kArrayType);
	//for (auto &exchangename : producer.exchangename)
	//{
	//	rapidjson::Value strVal;
	//	strVal.SetString(exchangename.c_str(), exchangename.length(), allocator);
	//	publisherMqexchangeObj.PushBack(strVal, allocator);
	//}
	//publisherMqObj.AddMember("exchangename", publisherMqexchangeObj, allocator);
	////publisherMqObj.AddMember("consumertag", producer.consumertag, allocator);

	//rapidjson::Value publisherMqroutekeyObj(rapidjson::kArrayType);
	//for (auto &routingkey : producer.routingkey)
	//{
	//	rapidjson::Value strVal;
	//	strVal.SetString(routingkey.c_str(), routingkey.length(), allocator);
	//	publisherMqroutekeyObj.PushBack(strVal, allocator);
	//}
	//publisherMqObj.AddMember("routingkey", publisherMqroutekeyObj, allocator);
	//publisherMqObj.AddMember("queuename", producer.queuename, allocator);
	//publisherMqObj.AddMember("web", producer.web, allocator);
	//publisherMqObj.AddMember("webInterval", producer.webInterval, allocator);

	//doc.AddMember("publisherMq", publisherMqObj, allocator);

	//wsSvr
	//rapidjson::Value wsSvrObj(rapidjson::kObjectType);
	//wsSvrObj.AddMember("ip", wsSvr.ip, allocator);
	//wsSvrObj.AddMember("port", wsSvr.port, allocator);
	//wsSvrObj.AddMember("path", wsSvr.path, allocator);
	//wsSvrObj.AddMember("handshakeTimeout", wsSvr.handshakeTimeout, allocator);
	//wsSvrObj.AddMember("idleTimeout", wsSvr.idleTimeout, allocator);
	//wsSvrObj.AddMember("enablePingPong", wsSvr.enablePingPong, allocator);
	//doc.AddMember("wsSvr", wsSvrObj, allocator);

	//save all
	doc.Accept(writer);

	return true;
}
//
//std::string file_topdir = "g:\\";
//
//std::string get_filecontent(const char* filePath)
//{
//	std::string fileDir(file_topdir);// 锟斤拷取锟斤拷锟斤拷头锟侥硷拷 锟斤拷锟斤拷使锟斤拷锟斤拷锟铰凤拷锟?
//	fileDir.append(filePath); // 锟斤拷锟斤拷路锟斤拷锟斤拷氐锟斤拷址锟斤拷锟斤拷锟斤拷锟斤拷锟饺ワ拷锟斤拷锟斤拷募锟?
//	std::ifstream in(fileDir.data()); // 锟侥硷拷锟侥讹拷取锟斤拷锟斤拷锟斤拷 copy锟斤拷  锟斤拷锟斤拷锟斤拷
//	if (!in.is_open()) {
//		printf("err here:%s,%d\r\n", __FILE__, __LINE__);
//		printf("faild open %s\r\n", fileDir.data());
//	}
//	std::string json_content((std::istreambuf_iterator<char>(in)), \
//		std::istreambuf_iterator<char>());
//	//锟斤拷锟侥硷拷锟斤拷锟斤拷锟斤拷锟斤拷转锟斤拷位std::string锟斤拷锟斤拷
//	//cout<<json_content<<endl; 
//	in.close();  // 锟截憋拷锟侥硷拷
//	return json_content;//锟斤拷锟斤拷锟斤拷之锟斤拷锟絪td::string 锟斤拷锟斤拷
//}

bool Config::setPassword(std::string pwd)
{
	std::string content = readFile(CONFIG_FILE_PATH);
	if (content.empty())
	{
		MHC_ERROR("Config file Error readFile error\n");
		return false;
	}
	rapidjson::Document doc;
	doc.Parse<rapidjson::kParseStopWhenDoneFlag>(content.c_str());

	std::ofstream ofs(CONFIG_FILE_PATH);
	if (!ofs.is_open())
		return false;

	rapidjson::OStreamWrapper osw(ofs);
	rapidjson::PrettyWriter<rapidjson::OStreamWrapper> writer(osw);

	rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

	doc["private"]["pwd"].SetString(pwd.c_str(), pwd.length());
	doc.Accept(writer);

	return true;
}
//bool prettyWritefile(const char *relatedPath, rapidjson::Document &document)
//{
//	rapidjson::StringBuffer buffer;
//	rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
//	//PrettyWriter  锟斤拷锟斤拷屎芄丶锟? 锟斤拷锟街伙拷锟絎rite锟侥伙拷  写锟斤拷慕峁癸拷锟饺斤拷锟窖匡拷锟斤拷锟斤拷锟角革拷省锟秸间？
//	document.Accept(writer);  //锟斤拷json锟斤拷锟斤拷锟斤拷锟斤拷写锟斤拷 writer?
//	std::string Path(file_topdir);  // 锟斤拷锟侥硷拷锟斤拷锟斤拷路锟斤拷转锟斤拷位std::string 锟斤拷锟斤拷
//	Path.append(relatedPath);       // 锟斤拷锟斤拷锟斤拷锟铰凤拷锟?
//	std::string temp(buffer.GetString());
//	std::ofstream outfile;
//	outfile.open(Path.data());
//	if (outfile.fail()) {
//		printf("open file %s error\r\n", Path.data());
//		printf("err here: %s,%d\r\n", __FILE__, __LINE__);
//		return false;
//	}
//	else {
//		outfile << temp;
//		outfile.close();
//		return true;
//		//锟斤拷锟斤拷锟侥硷拷写锟诫部锟街ｏ拷确实锟斤拷锟斤拷悉锟斤拷锟斤拷去锟斤拷锟斤拷慕锟斤拷停锟斤拷锟阶╋拷锟斤拷锟缴★拷
//		//锟斤拷锟斤拷锟斤拷锟斤拷拇锟斤拷锟揭诧拷锟斤拷锟?  实锟节憋拷歉
//	}
//}

bool Config::setNtp(std::string ip, bool used)
{
	std::string content = readFile(CONFIG_FILE_PATH);
	if (content.empty())
	{
		MHC_ERROR("Config file Error readFile error\n");
		return false;
	}
	rapidjson::Document doc;
	doc.Parse<rapidjson::kParseStopWhenDoneFlag>(content.c_str());

	std::ofstream ofs(CONFIG_FILE_PATH);
	if (!ofs.is_open())
		return false;

	rapidjson::OStreamWrapper osw(ofs);
	rapidjson::PrettyWriter<rapidjson::OStreamWrapper> writer(osw);

	rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

	doc["ntp"]["ip"].SetString(ip.c_str(), ip.length());
	doc["ntp"]["used"].SetBool(used);

	doc.Accept(writer);

	return true;
}

bool Config::AddChannel(DeviceParam & dev)
{
	camera_infos.push_back(dev);
	save();
	return true;
}

bool Config::DelChannel(int channel)
{
	for (auto it = camera_infos.begin(); it < camera_infos.end(); )
	{
		auto inf = *it;
		if (inf.camera.chan_num == channel)
		{
			it = camera_infos.erase(it);
			save();
			return true;
		}
		else
			it++;
	}
	return false;

}

bool Config::SetRois(int channel, std::vector<Config::RoiParam> & rois)
{
	for (auto &inf : camera_infos )
	{
		if (inf.camera.chan_num != channel)
		{
			continue;
		}
		inf.camera.rois.clear();
		inf.camera.rois.assign(rois.begin(), rois.end());

		save();
		return true;
	}
	return false;
}

bool Config::SetArmRule(std::string startTime, std::string endTime)
{
	std::string content = readFile(CONFIG_FILE_PATH);
	if (content.empty())
	{
		MHC_ERROR("Config file Error readFile error\n");
		return false;
	}
	rapidjson::Document doc;
	doc.Parse<rapidjson::kParseStopWhenDoneFlag>(content.c_str());

	std::ofstream ofs(CONFIG_FILE_PATH);
	if (!ofs.is_open())
		return false;

	rapidjson::OStreamWrapper osw(ofs);
	rapidjson::PrettyWriter<rapidjson::OStreamWrapper> writer(osw);

	rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

	doc["guard"]["startTime"].SetString(startTime.c_str(), startTime.length());
	doc["guard"]["endTime"].SetString(endTime.c_str(), endTime.length());

	doc.Accept(writer);

	return true;
}

bool Config::SetAlgoRule(int retentionTime, int alarmDelayedTime, int objectSize, int frameNbr, float confidence, float imageQuality)
{
	//std::string content = readFile(CONFIG_FILE_PATH);
	//if (content.empty())
	//{
	//	MHC_ERROR("Config file Error readFile error\n");
	//	return false;
	//}
	//rapidjson::Document doc;
	//doc.Parse<rapidjson::kParseStopWhenDoneFlag>(content.c_str());

	//std::ofstream ofs(CONFIG_FILE_PATH);
	//if (!ofs.is_open())
	//	return false;

	//rapidjson::OStreamWrapper osw(ofs);
	//rapidjson::PrettyWriter<rapidjson::OStreamWrapper> writer(osw);

	//rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

	//if (doc["camera_info"].IsArray())
	//{
	//	rapidjson::Value arrDev = doc["camera_info"].GetArray();
	//	int ll = arrDev.Size();
	//	for (int k = 0; k < (int)arrDev.Size(); k++)
	//	{
	//		if (arrDev[k].IsObject())
	//		{
	//			doc["camera_info"]["retentionTime"].SetString(startTime.c_str(), startTime.length());
	//			doc["guard"]["endTime"].SetString(endTime.c_str(), endTime.length());

	//		}
	//}


	//doc.Accept(writer);
	for (DeviceParam &dev : camera_infos)
	{
		dev.camera.retentionTime = retentionTime;
		dev.camera.alarmDelayedTime = alarmDelayedTime;
		dev.camera.objectSize = objectSize;
		dev.camera.frameNbr = frameNbr;
		dev.camera.confidence = confidence;
		dev.camera.imageQuality = imageQuality;
	}

	save();
	load(NULL);
	return true;

}

bool Config::save()
{
	std::error_code ec;

	time_t time_now = time(NULL);
	struct tm * st = localtime(&(time_now));
	char fpath[256];
	sprintf(fpath, "%s-%04d%02d%02d_%02d%02d%02d.bak", CONFIG_FILE_PATH,
		st->tm_year + 1900, st->tm_mon + 1, st->tm_mday, st->tm_hour, st->tm_min, st->tm_sec);
	std::string szt = fpath;
	
	std::fs::copy_file(CONFIG_FILE_PATH, szt , ec);

	std::ofstream ofs(CONFIG_FILE_PATH);
	if (!ofs.is_open())
		return false;

	rapidjson::OStreamWrapper osw(ofs);
	rapidjson::PrettyWriter<rapidjson::OStreamWrapper> writer(osw);

	rapidjson::Document doc;
	rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
	doc.SetObject();
	rapidjson::Value privObj(rapidjson::kObjectType);
	privObj.AddMember("logLevel", stPrivate.logLevel, allocator);
	doc.AddMember("private", privObj, allocator);

	rapidjson::Value httpObj(rapidjson::kObjectType);
	httpObj.AddMember("port", stHttp.port, allocator);
	doc.AddMember("httpsvr", httpObj, allocator);

	rapidjson::Value baseObj(rapidjson::kObjectType);
	baseObj.AddMember("boxId", stBase.boxId, allocator);
	baseObj.AddMember("model", stBase.model, allocator);
	baseObj.AddMember("snbr", stBase.snbr, allocator);
	baseObj.AddMember("softVer", stBase.softVer, allocator);
	baseObj.AddMember("algoVer", stBase.algoVer, allocator);
	doc.AddMember("base", baseObj, allocator);

	rapidjson::Value ntpObj(rapidjson::kObjectType);
	ntpObj.AddMember("used", stNtp.used, allocator);
	ntpObj.AddMember("ip", stNtp.ip, allocator);
	doc.AddMember("ntp", ntpObj, allocator);

	rapidjson::Value mqttObj(rapidjson::kObjectType);
	mqttObj.AddMember("ip", stMqtt.ip, allocator);
	mqttObj.AddMember("port", stMqtt.port, allocator);
	mqttObj.AddMember("user", stMqtt.user, allocator);
	mqttObj.AddMember("password", stMqtt.password, allocator);
	mqttObj.AddMember("topic", stMqtt.topic, allocator);
	doc.AddMember("mqtt", mqttObj, allocator);

	rapidjson::Value camerasArr(rapidjson::kArrayType);  //camerainfo [
	for (auto &device : camera_infos)
	{
		rapidjson::Value deviceObj(rapidjson::kObjectType);  //device [
		deviceObj.AddMember("devid", device.devid, allocator);

		rapidjson::Value cameraObj(rapidjson::kObjectType); //cameras [
		cameraObj.AddMember("address", device.camera.address, allocator);
		cameraObj.AddMember("chan_num", device.camera.chan_num, allocator);
		cameraObj.AddMember("ptzType", device.camera.ptzType, allocator);
		cameraObj.AddMember("imgType", device.camera.imgType, allocator);
		cameraObj.AddMember("algSensitivity", device.camera.algSensitivity, allocator);
		cameraObj.AddMember("vqaSensitivity", device.camera.vqaSensitivity, allocator);
		cameraObj.AddMember("retentionTime", device.camera.retentionTime, allocator);
		cameraObj.AddMember("alarmDelayedTime", device.camera.alarmDelayedTime, allocator);
		cameraObj.AddMember("objectSize", device.camera.objectSize, allocator);
		cameraObj.AddMember("frameNbr", device.camera.frameNbr, allocator);
		cameraObj.AddMember("imgType", device.camera.imgType, allocator);
		cameraObj.AddMember("name", device.camera.name, allocator);
		cameraObj.AddMember("width", device.camera.width, allocator);
		cameraObj.AddMember("height", device.camera.height, allocator);

		rapidjson::Value roisArr(rapidjson::kArrayType); //rois [
		for (auto &roi : device.camera.rois)
		{
			rapidjson::Value roiObj(rapidjson::kObjectType);
			roiObj.AddMember("roiId", roi.roiId, allocator);
			rapidjson::Value ptsArr(rapidjson::kArrayType);
			for (auto &pt : roi.points)
			{
				rapidjson::Value fVal;
				fVal.SetFloat(pt.x);
				ptsArr.PushBack(fVal, allocator);
				fVal.SetFloat(pt.y);
				ptsArr.PushBack(fVal, allocator);
			}
			roiObj.AddMember("points", ptsArr, allocator);

			roisArr.PushBack(roiObj, allocator);
		}

		cameraObj.AddMember("rois", roisArr, allocator);  //rois ]
		deviceObj.AddMember("cameras", cameraObj, allocator); //cameras ]

		camerasArr.PushBack(deviceObj, allocator); //device ]
	}

	doc.AddMember("camera_info", camerasArr, allocator); //camerainfo ]

	rapidjson::Value guardObj(rapidjson::kObjectType);
	guardObj.AddMember("startTime", stGuard.startTime, allocator);
	guardObj.AddMember("endTime", stGuard.endTime, allocator);
	doc.AddMember("guard", guardObj, allocator);

	doc.AddMember("algVersionPath", algVersionPath, allocator);
	doc.AddMember("exeAlgPath", exeAlgPath, allocator);
	doc.AddMember("detectModelPath", detectModelPath, allocator);
	doc.AddMember("classifyModelPath", classifyModelPath, allocator);
	doc.AddMember("snapshotPath", snapshotPath, allocator);
	doc.AddMember("alarmImagePath", alarmImagePath, allocator);
	doc.AddMember("softUpdatePath", softUpdatePath, allocator);
	doc.AddMember("algoUpdatePath", algoUpdatePath, allocator);
	doc.AddMember("channelStatusPath", channelStatusPath, allocator);
	doc.AddMember("nginxPath", nginxPath, allocator);
	doc.AddMember("nginxImgPath", nginxImgPath, allocator);

	//save all
	writer.SetMaxDecimalPlaces(6);
	doc.Accept(writer);

	return true;

}
