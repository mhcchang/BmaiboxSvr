/*****************************************************************************
*  @projectName: WsMqSvr													 *
*  @file  WsMqSvr.cpp														 *
*  @brief   服务程序															 *
*  @author   mhchang	                                                     *
*  @version  1.0.0.0		                                                 *
*****************************************************************************/
#include "defs.h"
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "SvrMain.h"
//#include "sys_inc.h"

#include "service_base.h"
#if __WINDOWS_OS__
#include <Windows.h>
#else
#include <string.h>

#define VERSION_STR v1.0.0
#define BUILDER_STR 1
#define STRINGIFY(x) XSTRINGIFY(x)
#define XSTRINGIFY(x) #x
static char LIB_INFO[] = {
	"$" "WsMqSvr Version:" STRINGIFY(VERSION_STR)
	"  Built: " STRINGIFY(BUILDER_STR)
	__DATE__
	" "
	__TIME__
	" $"
};
#endif

//#include "sys_log.h"

#include <thread>
#include <chrono>
#if __WINDOWS_OS__
#if 0
#include "WinService.h"
#else

#include <DbgHelp.h>
#pragma comment(lib, "dbghelp.lib")

LONG __stdcall ExceptCallBack(EXCEPTION_POINTERS *pExcPointer)
{
	//MessageBox(NULL, "程序崩溃！相关信息记录在C:\\Test.dmp文件中。", NULL, MB_OK);

	//创建dump文件
	HANDLE hFile = CreateFile("WsMqSvr.dmp", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	//向文件写下当前程序崩溃相关信息
	MINIDUMP_EXCEPTION_INFORMATION loExceptionInfo;
	loExceptionInfo.ExceptionPointers = pExcPointer;
	loExceptionInfo.ThreadId = GetCurrentThreadId();
	loExceptionInfo.ClientPointers = TRUE;
	MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &loExceptionInfo, NULL, NULL);
	CloseHandle(hFile);

	return EXCEPTION_EXECUTE_HANDLER;
}

#endif
#elif __LINUX_OS__
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <printf.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#define LOCKFILE "/var/run/wsmqsvr.pid"
#define LOCKMODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

#endif
#include <iostream>
#include <csignal>

#if __WINDOWS_OS__
#if 0
class WsMqSvr : protected WinService
{
public:
	int main(int argc, char* argv[])
	{
		//if (auth() != 0)
		//	return -1;

		if (argc >= 2 && (argv[1][0] == '-' || argv[1][0] == '/'))
		{
			char* cmd = &argv[1][1];
			if (_stricmp(cmd, "install") == 0
				|| _stricmp(cmd, "uninstall") == 0
				|| _stricmp(cmd, "service") == 0
				|| _stricmp(cmd, "daemon") == 0)
			{
				WinService::setName(TEXT("CvcamSentryService")).setArgs(TEXT("-service"));
				return WinService::main(argc, argv);
			}
			else if (_stricmp(cmd, "stop") == 0)
			{
				Stop_Server("CvcamSentryService");
				//sig_handler(SIGINT);
				log("CvcamSentryService set stop\n");
				return 42;
			}
		}

		return runInConsole(argc, argv);
	}

	bool Stop_Server(const std::string& strServiceName)
	{
		bool bResult = false;
		if (strServiceName.empty())
		{
			return bResult;
		}
		SC_HANDLE sc_Manager = ::OpenSCManagerA(NULL, NULL, GENERIC_EXECUTE);
		if (sc_Manager)
		{
			SC_HANDLE sc_service = ::OpenServiceA(sc_Manager, strServiceName.c_str(), SERVICE_ALL_ACCESS);
			if (sc_service)
			{
				SERVICE_STATUS_PROCESS service_status;
				ZeroMemory(&service_status, sizeof(SERVICE_STATUS_PROCESS));
				DWORD dwpcbBytesNeeded = sizeof(SERVICE_STATUS_PROCESS);
				if (::QueryServiceStatusEx(sc_service, SC_STATUS_PROCESS_INFO,
					(LPBYTE)&service_status,
					dwpcbBytesNeeded,
					&dwpcbBytesNeeded))
				{
					SERVICE_CONTROL_STATUS_REASON_PARAMSA service_control_status;
					DWORD dwerror = NULL;
					ZeroMemory(&service_control_status, sizeof(SERVICE_CONTROL_STATUS_REASON_PARAMSA));
					if (service_status.dwCurrentState == SERVICE_RUNNING)
					{
						service_control_status.dwReason = SERVICE_STOP_REASON_FLAG_PLANNED | SERVICE_STOP_REASON_MAJOR_APPLICATION | SERVICE_STOP_REASON_MINOR_NONE;;
						if (!::ControlServiceExA(sc_service, SERVICE_CONTROL_STOP, SERVICE_CONTROL_STATUS_REASON_INFO, &service_control_status))
						{
							dwerror = ::GetLastError();
							::CloseServiceHandle(sc_service);
							::CloseServiceHandle(sc_Manager);
							return bResult;
						}
						while (::QueryServiceStatusEx(sc_service, SC_STATUS_PROCESS_INFO,
							(LPBYTE)&service_status,
							dwpcbBytesNeeded,
							&dwpcbBytesNeeded))
						{
							Sleep(service_status.dwWaitHint);
							if (service_status.dwCurrentState == SERVICE_STOPPED)
							{
								bResult = true;
								break;
							}
						}
					}
				}
				::CloseServiceHandle(sc_service);
			}
			::CloseServiceHandle(sc_Manager);
		}
		return bResult;
	}

protected:
	int runInConsole(int argc, char* argv[])
	{
		if (!SentrySvr_Start(""))
		{
			log("SentrySvr_start failed\n");
		}

		//signal(SIGINT, &sig_handler);

		printf("Press Ctrl+C to terminate.\n");

		while (keepRunning)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}

		Svr_Stop();

		//getchar();
		return 0;
	}

	static void sig_handler(int sig)
	{
		if (sig == SIGINT)
		{
			keepRunning = false;
		}
	}

	static volatile bool keepRunning;

	// service interface
protected:
	virtual bool doStart(DWORD dwNumServiceArgs, LPSTR* lpServiceArgVectors)
	{
		const char* configPath = dwNumServiceArgs >= 2 ? lpServiceArgVectors[1] : nullptr;
		return SentrySvr_Start("");
	}

	virtual void doStop()
	{
		Svr_Stop();
	}

public:
};

volatile bool WsMqSvr::keepRunning = true;
#else

class WsMqSvr : public cvcam::base::ServiceBase
{
protected:
	// 启动服务
	virtual int onStartService(int argc, char* argv[]) override
	{

		OutputDebugStringA("WsMqSvr::onStartService");
		SetUnhandledExceptionFilter(ExceptCallBack);
		//m_app.reset(new WsMqSvr(serviceConfig().id)); // NOTE: 需要将PushService换成自己的类
		if (!m_app->start()) // NOTE: 需要将start换成自己的启动方法
			return -1;
		return 0;
	}

	// 停止服务
	virtual void onStopService() override
	{
		OutputDebugStringA("onStopService");
		m_app->stop(); // NOTE: 需要将stop换成自己的停止方法
	}

protected:
	std::shared_ptr<WsMqSvr> m_app; // NOTE: 需要将PushService换成自己的类
};
#endif

// 服务程序既可以在控制台中运行，也可以以后台服务方式运行
// 包含的命令：
// -install 将程序安装为服务
// -uninstall 卸载服务
// -start 启动服务
// -stop 停止服务
// -restart 重启服务
// 其它参数则以在控制台中直接运行
int main(int argc, char* argv[])
{
	Config m_config;

	changeWorkingDirectory();
	if (!m_config.load(""))
	{
		//log_print(ZH_LOG_ERROR, "%s, load configure file failed! \r\n", __FUNCTION__);
		OutputDebugStringA("load failed");
		return false;
	}

	cvcam::base::ServiceConfig config;
	//config.name = "B_Service"; // NOTE: 需要将CvcamPushService换成自己服务的名称
	//config.win_dependencies = "Tcpip"; // 在Windows上所依赖的其它服务，根据需要配置
	// 这里将来会加一些其它配置参数

	config.name.assign(m_config.base.serviceName);
	config.displayName.assign(m_config.base.serviceDisplayName);
	config.description.assign(m_config.base.serviceDescription);
	config.win_startType = (cvcam::base::WinServiceStartType)m_config.base.startType;
	config.debug = true;

	::WsMqSvr service;

	return service.run(config, argc, argv);
}

#elif __LINUX_OS__
#if 0
int lockfile(int fd)
{
	struct flock fl;

	fl.l_type = F_WRLCK;  /* write lock */
	fl.l_start = 0;
	fl.l_whence = SEEK_SET;
	fl.l_len = 0;  //lock the whole file
	return(fcntl(fd, F_SETLK, &fl));
}

int already_running(const char *filename)
{
	int fd;
	char buf[16];

	fd = open(filename, O_RDWR | O_CREAT, LOCKMODE);
	if (fd < 0) {
		printf("can't open %s: %m\n", filename);
		exit(1);
	}
	/* 先获取文件锁 */
	if (lockfile(fd) == -1) {
		if (errno == EACCES || errno == EAGAIN) {
			printf("file: %s already locked", filename);
			close(fd);
			return 1;
		}
		printf("can't lock %s: %m\n", filename);
		exit(1);
	}

	/* 写入运行实例的pid */

	ftruncate(fd, 0);
	sprintf(buf, "%ld", (long)getpid());
	write(fd, buf, strlen(buf) + 1);
	return 0;
}
#endif 
#endif

void init_network()
{
#if __WINDOWS_OS__
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
}

void print_help()
{
	printf("Usage : CvcamSentryService -c config\r\n");
}

bool parse_cmd_opts(int argc, char *argv[])
{
	bool ret = false;
	//opts->exefile = argv[0];

	if (argc == 3)
	{
#if __WINDOW_OS__
		//		strcasecmp
		if (!stricmp(argv[1], "-c") || !stricmp(argv[1], "/c"))
#else

		if (!strcasecmp(argv[1], "-c") || !strcasecmp(argv[1], "/c"))
#endif		
		{
			//opts->cfgfile = argv[2];
			ret = true;
		}
		else
		{
			print_help();
		}
	}
	else if (argc == 1)
	{
		ret = true; // use the default config
	}
	else
	{
		print_help();
	}

	return ret;
}

#if __LINUX_OS__

void sig_handler(int sig)
{
	//log_print(ZH_LOG_DEBUG, "%s, sig=%d\r\n", __FUNCTION__, sig);

	Svr_stop();

	exit(0);
}

int daemon_init()
{
	pid_t pid;

	pid = fork();

	if (pid == -1)
	{
		return -1;
	}
	else if (pid > 0)
	{
		exit(0);
	}

	setsid();
	return 0;
}

#elif __WINDOWS_OS__

bool WINAPI sig_handler(DWORD dwCtrlType)
{
	switch (dwCtrlType)
	{
	case CTRL_C_EVENT:      // Ctrl+C
		log_print(ZH_LOG_DEBUG, "%s, CTRL+C\r\n", __FUNCTION__);
		break;

	case CTRL_BREAK_EVENT: // Ctrl+Break
		log_print(ZH_LOG_DEBUG, "%s, CTRL+Break\r\n", __FUNCTION__);
		break;

	case CTRL_CLOSE_EVENT: // Closing the consolewindow
		log_print(ZH_LOG_DEBUG, "%s, Closing\r\n", __FUNCTION__);
		break;
	}

	Svr_stop();

	// Return true if handled this message,further handler functions won't be called.
	// Return false to pass this message to further handlers until default handler calls ExitProcess().

	return false;
}
#endif

int main(int argc, char *argv[])
{
	//memset(&opts, 0, sizeof(opts));

	//if (!parse_cmd_opts(argc, argv, &opts))
	//{
	//	return 0;
	//}

	init_network();

#if __LINUX_OS__
	printf(LIB_INFO);
#if !defined(DEBUG) && !defined(_DEBUG)
	if (daemon_init() == -1)
	{
		printf("can't fork self \n");
		exit(0);
	}
#endif
	/*
	if (already_running(LOCKFILE))
	{
		return 0;
	}*/
	// Ignore broken pipes
	signal(SIGPIPE, SIG_IGN);

#endif

#if __LINUX_OS__
	signal(SIGINT, sig_handler);
	signal(SIGKILL, sig_handler);
	signal(SIGTERM, sig_handler);

	Svr_start();
	{
		for (;;)
		{
#if defined(DEBUG) || defined(_DEBUG)		
			if (getchar() == 'q')
			{
				Svr_stop();
				break;
			}
#endif
			sleep(5);
		}
	}
	return 0;
#elif __WINDOWS_OS__
	WsMqSvr app;
	int res = app.main(argc, argv);
	return res;
	//SetConsoleCtrlHandler(sig_handler, true);
#endif
}
