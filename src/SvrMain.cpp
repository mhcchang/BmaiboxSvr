#include "SvrMain.h"
#include "httpserver.h"

#include <thread>
#include <chrono>

#define MHC_CALLBACK(FUNCTION,OBJECT) std::bind(&FUNCTION,OBJECT)
#define MHC_CALLBACK_1(FUNCTION,OBJECT) std::bind(&FUNCTION,OBJECT,std::placeholders::_1)
#define MHC_CALLBACK_2(FUNCTION,OBJECT) std::bind(&FUNCTION,OBJECT,std::placeholders::_1,std::placeholders::_2)
#define MHC_CALLBACK_3(FUNCTION,OBJECT) std::bind(&FUNCTION,OBJECT,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3)
#define MHC_CALLBACK_4(FUNCTION,OBJECT) std::bind(&FUNCTION,OBJECT,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3,std::placeholders::_4)
#define MHC_CALLBACK_5(FUNCTION,OBJECT) std::bind(&FUNCTION,OBJECT,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3,std::placeholders::_4,std::placeholders::_5)

SvrMain* g_pMain;

extern std::string C_APIPATH[API_NUM];
#include "logger.h"

SvrMain::SvrMain()
{
	g_pMain = this;
	m_blRunning = false;
}

SvrMain::~SvrMain()
{
}

bool SvrMain::InitSvr()
{
	MHC_LOGGER_INIT("svrlog", MHC_SINK_ROTATING_FILE, MHC_LEVEL_DEBUG);
	MHC_INFO("SvrMain::InitSvr");
}

void SvrMain::Uninit()
{
	m_httpSvr->stopsvr();
	delete m_httpThread;
	delete m_mqSvr;

	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	delete m_httpSvr;

	delete m_config;
}

bool SvrMain::Init()
{
	m_config = new Config();
	if (!m_config->load(""))
	{
		MHC_INFO("load config file error!");

		printf("load config file error!");
		return false;
	}

	m_pro = new WsMqSvr::ProcCmd(m_config);

	//m_pro->m_config = m_config;
	m_httpThread = new HttpThread(1);
	m_httpThread->SetConfig();
	m_httpThread->SetProCmd();

	m_httpSvr = new HttpServer();

	printf("--http port %d \n", m_config->stHttp.port);
	m_httpSvr->Init(std::to_string(m_config->stHttp.port));
	//1
	m_httpSvr->AddHandler(C_APIPATH[0], MHC_CALLBACK_5(HttpThread::GetLogin, m_httpThread));
	m_httpSvr->AddHandler(C_APIPATH[1], MHC_CALLBACK_5(HttpThread::GetLogout, m_httpThread));
	m_httpSvr->AddHandler(C_APIPATH[2], MHC_CALLBACK_5(HttpThread::GetSetPassword, m_httpThread));
	m_httpSvr->AddHandler(C_APIPATH[3], MHC_CALLBACK_5(HttpThread::GetGetBoxId, m_httpThread));
	m_httpSvr->AddHandler(C_APIPATH[4], MHC_CALLBACK_5(HttpThread::GetSetBoxId, m_httpThread));
	m_httpSvr->AddHandler(C_APIPATH[5], MHC_CALLBACK_5(HttpThread::GetGetSysTime, m_httpThread));  //5
	m_httpSvr->AddHandler(C_APIPATH[6], MHC_CALLBACK_5(HttpThread::GetSetSysTime, m_httpThread));
	m_httpSvr->AddHandler(C_APIPATH[7], MHC_CALLBACK_5(HttpThread::GetReboot, m_httpThread));
	m_httpSvr->AddHandler(C_APIPATH[8], MHC_CALLBACK_5(HttpThread::GetGetBasicInfo, m_httpThread));
	m_httpSvr->AddHandler(C_APIPATH[9], MHC_CALLBACK_5(HttpThread::GetSoftUpdate, m_httpThread));
	m_httpSvr->AddHandler(C_APIPATH[10], MHC_CALLBACK_5(HttpThread::GetGetNetwork, m_httpThread)); //10
	m_httpSvr->AddHandler(C_APIPATH[11], MHC_CALLBACK_5(HttpThread::GetSetNetwork, m_httpThread));
	m_httpSvr->AddHandler(C_APIPATH[12], MHC_CALLBACK_5(HttpThread::GetGetMqtt, m_httpThread));
	m_httpSvr->AddHandler(C_APIPATH[13], MHC_CALLBACK_5(HttpThread::GetSetMqtt, m_httpThread));
	m_httpSvr->AddHandler(C_APIPATH[14], MHC_CALLBACK_5(HttpThread::GetGetDeviceStatus, m_httpThread));
	m_httpSvr->AddHandler(C_APIPATH[15], MHC_CALLBACK_5(HttpThread::GetGetCameras, m_httpThread)); //15
	m_httpSvr->AddHandler(C_APIPATH[16], MHC_CALLBACK_5(HttpThread::GetAddCamera, m_httpThread));
	m_httpSvr->AddHandler(C_APIPATH[17], MHC_CALLBACK_5(HttpThread::GetDelCamera, m_httpThread));
	m_httpSvr->AddHandler(C_APIPATH[18], MHC_CALLBACK_5(HttpThread::GetGetRoi, m_httpThread));
	m_httpSvr->AddHandler(C_APIPATH[19], MHC_CALLBACK_5(HttpThread::GetSetRoi, m_httpThread));
	m_httpSvr->AddHandler(C_APIPATH[20], MHC_CALLBACK_5(HttpThread::GetGetSnapshot, m_httpThread)); //20
	m_httpSvr->AddHandler(C_APIPATH[21], MHC_CALLBACK_5(HttpThread::GetGetArmRule, m_httpThread));
	m_httpSvr->AddHandler(C_APIPATH[22], MHC_CALLBACK_5(HttpThread::GetSetArmRule, m_httpThread));
	m_httpSvr->AddHandler(C_APIPATH[23], MHC_CALLBACK_5(HttpThread::GetGetAlgoRule, m_httpThread));
	m_httpSvr->AddHandler(C_APIPATH[24], MHC_CALLBACK_5(HttpThread::GetSetAlgoRule, m_httpThread));
	m_httpSvr->AddHandler(C_APIPATH[25], MHC_CALLBACK_5(HttpThread::GetCount, m_httpThread)); //25
	m_httpSvr->AddHandler(C_APIPATH[26], MHC_CALLBACK_5(HttpThread::GetQuery, m_httpThread));

	m_httpSvr->startsvr();

	m_mqSvr = new MqSvr();
	//tcp://mqtt.eclipseprojects.io:1883
	m_mqSvr->Init("tcp://" + m_config->stMqtt.ip + ":" + std::to_string(m_config->stMqtt.port), "ws svr", m_config->stMqtt.topic);
	return true;
}

bool SvrMain::Start()
{
	m_thread = std::thread(&SvrMain::MainLoop, this);
}

void SvrMain::Stop()
{
	m_blRunning = false;
}

//处理消息
void SvrMain::MainLoop()
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
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
}

void SvrMain::PushAlarm(std::string msg)
{
	if (m_mutex.try_lock())
	{
		m_vtMsg.push_back(msg);
		m_mutex.unlock();
	}
}

void Svr_start()
{
	printf("Svr_start \n");
	g_pMain = new SvrMain();
	g_pMain->InitSvr();
	g_pMain->Init();

	g_pMain->Start();
}

void Svr_stop()
{
	g_pMain->Stop();

	g_pMain->Uninit();
	delete g_pMain;
}
