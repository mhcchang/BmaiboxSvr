#ifndef __SVRMAIN_H__
#define __SVRMAIN_H__

#include "defs.h"
#include "httpserver.h"
#include "HttpThread.h"
#include "Config.h"

#include <thread>
#include <atomic>
#include <mutex>

void Svr_start();
void Svr_stop();

class SvrMain
{
   // Q_OBJECT

public:
    explicit SvrMain();
    ~SvrMain();

public:
	bool InitSvr();
	bool Init();
	void Uninit();
	Config * GetConfig() { return m_config; };
	WsMqSvr::ProcCmd * GetProcCmd() { return m_pro; };
	void PushAlarm(std::string msg);

	bool Start();
	void Stop();
private:
	void MainLoop();

	//http api interface
	HttpServer * m_httpSvr;
	HttpThread * m_httpThread;
	Config * m_config;

	std::thread m_thread;
	std::mutex m_mutex;
	volatile std::atomic_bool m_blRunning;
	WsMqSvr::ProcCmd * m_pro;

	MqSvr * m_mqSvr;

	std::vector<std::string> m_vtMsg;
};

#endif // MAINWINDOW_H
