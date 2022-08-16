#ifdef __WINDOWS_OS__

#include "service_windows.h"

#include <string>
#include <iostream>


CVCAM_BASE_NAMESPACE_BEGIN

WindowsService* WindowsService::s_pInstance = nullptr;

WindowsService& WindowsService::instance()
{	
	return *s_pInstance;
}

WindowsService::WindowsService()
{
	s_pInstance = this;

	m_status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	m_status.dwCurrentState = SERVICE_STOPPED;
	m_status.dwControlsAccepted = SERVICE_ACCEPT_STOP;
	m_status.dwWin32ExitCode = 0;
	m_status.dwServiceSpecificExitCode = 0;
	m_status.dwCheckPoint = 0;
	m_status.dwWaitHint = 0;

	m_hStatus = NULL;
	m_threadID = 0;

    changeWorkingDirectory();
}

WindowsService::~WindowsService()
{

}

int WindowsService::installService()
{
	char path[MAX_PATH] = { 0 };
	if (!GetModuleFileNameA(NULL, path + 1, MAX_PATH - 2))
	{
		return GetLastError();
	}
	path[0] = '"';
	int len = lstrlenA(path);
	path[len] = '"';
	if (!m_config.startArgs.empty())
	{
		path[len + 1] = ' ';
		lstrcpynA(path + len + 2, m_config.startArgs.c_str(), MAX_PATH - len - 2);
	}

	SC_HANDLE hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (hSCM == NULL)
	{
		std::cout << "service install failed, cannot open SCM, error=" << GetLastError() << std::endl;
		return GetLastError();
	}

    DWORD startType = m_config.win_startType;
    if (m_config.win_startType == WIN_SERVICE_DELAYED_AUTO_START)
        startType = WIN_SERVICE_AUTO_START;

    std::string dependencies;
    if (!m_config.win_dependencies.empty())
    {
        dependencies = m_config.win_dependencies;

        // ensure string is terminated with double null
        if (dependencies.back() != '\0')
            dependencies += '\0';
        if (dependencies[dependencies.length() - 2] != '\0')
            dependencies += '\0';
    }

	SC_HANDLE hService = CreateServiceA(
		hSCM,
        m_config.name.c_str(),
        m_config.displayName.c_str(),
		SERVICE_ALL_ACCESS,
		SERVICE_WIN32_OWN_PROCESS,
		startType,
		SERVICE_ERROR_NORMAL,
		path,
		NULL,
		NULL,
        !dependencies.empty() ? dependencies.c_str() : NULL,
		!m_config.win_username.empty() ? m_config.win_username.c_str() : NULL,
		!m_config.win_password.empty() ? m_config.win_password.c_str() : NULL
	);

	if (hService == NULL)
	{
		std::cout << "service install failed, cannot create service, error=" << GetLastError() << std::endl;
		CloseServiceHandle(hSCM);
		return GetLastError();
	}

    if (m_config.win_startType == WIN_SERVICE_DELAYED_AUTO_START)
    {
		SERVICE_DELAYED_AUTO_START_INFO info = { TRUE };
		if (!ChangeServiceConfig2A(hService, SERVICE_CONFIG_DELAYED_AUTO_START_INFO, &info))
		{
			std::cout << "failed to enable delayed auto-start" << std::endl;
		}
    }

	if (!m_config.description.empty())
	{
		SERVICE_DESCRIPTIONA descInfo;
		descInfo.lpDescription = (LPTSTR)m_config.description.c_str();
		if (!ChangeServiceConfig2A(hService, SERVICE_CONFIG_DESCRIPTION, &descInfo))
		{
			std::cout << "failed to set services description" << std::endl;
		}
	}
	CloseServiceHandle(hService);
	CloseServiceHandle(hSCM);

	std::cout << "service successfully installed" << std::endl;

	return 0;
}

int WindowsService::uninstallService()
{
	SC_HANDLE hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (hSCM == NULL)
	{
		std::cout << "service uninstall failed, cannot open SCM, error=" << GetLastError() << std::endl;
		return GetLastError();
	}

	SC_HANDLE hService = OpenServiceA(hSCM, m_config.name.c_str(), DELETE);
	if (hService == NULL)
	{
		std::cout << "service install failed, cannot open service, error=" << GetLastError() << std::endl;
		CloseServiceHandle(hSCM);
		return GetLastError();
	}

	int res = 0;
	if (!DeleteService(hService))
	{
		std::cout << "service install failed, cannot delete service, error=" << GetLastError() << std::endl;
		res = GetLastError();
	}

	CloseServiceHandle(hService);
	CloseServiceHandle(hSCM);

    if (res == 0)
    {
        std::cout << "service successfully uninstalled" << std::endl;
    }

	return res;
}

int WindowsService::startService()
{
    int res = 0;

    SC_HANDLE hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (hSCM == NULL)
    {
        std::cout << "service start failed, cannot open SCM, error=" << GetLastError() << std::endl;
        return GetLastError();
    }

    SC_HANDLE hService = OpenServiceA(hSCM, m_config.name.c_str(), SERVICE_START);
    if (hService == NULL)
    {
        std::cout << "service start failed, cannot open service, error=" << GetLastError() << std::endl;
        CloseServiceHandle(hSCM);
        return GetLastError();
    }

    if (!StartService(hService, 0, NULL))
    {
        std::cout << "service start failed, cannot start service, error=" << GetLastError() << std::endl;
        res = GetLastError();
    }

    CloseServiceHandle(hService);
    CloseServiceHandle(hSCM);

    if (res == 0)
    {
        std::cout << "service successfully started" << std::endl;
    }

    return res;
}

int WindowsService::stopService()
{
    int res = 0;

    SC_HANDLE hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (hSCM == NULL)
    {
        std::cout << "service stop failed, cannot open SCM, error=" << GetLastError() << std::endl;
        return GetLastError();
    }

    SC_HANDLE hService = OpenServiceA(hSCM, m_config.name.c_str(), SERVICE_STOP);
    if (hService == NULL)
    {
        std::cout << "service stop failed, cannot open service, error=" << GetLastError() << std::endl;
        CloseServiceHandle(hSCM);
        return GetLastError();
    }

    SERVICE_STATUS status;
    if (!ControlService(hService, SERVICE_CONTROL_STOP, &status))
    {
        std::cout << "service stop failed, cannot stop service, error=" << GetLastError() << std::endl;
        res = GetLastError();
    }

    CloseServiceHandle(hService);
    CloseServiceHandle(hSCM);

    if (res == 0)
    {
        std::cout << "service successfully stopped" << std::endl;
    }

    return res;
}

int WindowsService::restartService()
{
    return -1;
}

int WindowsService::runAsService(int argc, char* argv[])
{
    log(SERVICE_LOG_INFO, "[Service] main: enter, pid=%d, tid=%d", GetCurrentProcessId(), GetCurrentThreadId());

    SERVICE_TABLE_ENTRYA serviceTables[] = {
        { (LPSTR)m_config.name.c_str(), (LPSERVICE_MAIN_FUNCTIONA)&WindowsService::_serviceMain },
        { NULL, NULL }
    };

    if (!StartServiceCtrlDispatcherA(serviceTables))
    {
        log(SERVICE_LOG_ERR, "[Service] main: StartServiceCtrlDispatcher failed, error=%d, pid=%d, tid=%d", GetLastError(), GetCurrentProcessId(), GetCurrentThreadId());
        return GetLastError();
    }

    log(SERVICE_LOG_INFO, "[Service] main: leave, pid=%d, tid=%d", GetCurrentProcessId(), GetCurrentThreadId());
    return 0;
}

void WINAPI WindowsService::_serviceMain(DWORD dwNumServiceArgs, LPSTR* lpServiceArgVectors)
{
	WindowsService::instance().serviceMain(dwNumServiceArgs, lpServiceArgVectors);
}

DWORD WINAPI WindowsService::_serviceCtrlHandler(DWORD dwControl, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext)
{
	return WindowsService::instance().serviceCtrlHandler(dwControl, dwEventType, lpEventData, lpContext);
}

void WindowsService::serviceMain(DWORD dwNumServiceArgs, LPSTR* lpServiceArgVectors)
{
	log(SERVICE_LOG_INFO, "[Service] serviceMain: enter, pid=%d, tid=%d", GetCurrentProcessId(), GetCurrentThreadId());

	m_status.dwCurrentState = SERVICE_START_PENDING;
	m_threadID = GetCurrentThreadId();

	m_hStatus = RegisterServiceCtrlHandlerExA(m_config.name.c_str(), &WindowsService::_serviceCtrlHandler, NULL);
	if (m_hStatus == NULL)
	{
		log(SERVICE_LOG_ERR, "[Service] serviceMain: RegisterServiceCtrlHandlerEx failed, error=%d, pid=%d, tid=%d", GetLastError(), GetCurrentProcessId(), GetCurrentThreadId());
		return;
	}

    m_serviceMain(static_cast<int>(dwNumServiceArgs), lpServiceArgVectors);

	//setStatus(SERVICE_START_PENDING);

	//if (doStart(dwNumServiceArgs, lpServiceArgVectors))
	//{
	//	setStatus(SERVICE_RUNNING);

	//	if (!runLoop())
	//	{
	//		m_status.dwWin32ExitCode = E_FAIL;
	//	}

	//	doStop();
	//}
	//else
	//{
	//	log("[Service] serviceMain: doStart failed, pid=%d, tid=%d", GetCurrentProcessId(), GetCurrentThreadId());
	//	m_status.dwWin32ExitCode = E_FAIL;
	//}

	//setStatus(SERVICE_STOPPED);

	log(SERVICE_LOG_INFO, "[Service] serviceMain: leave, pid=%d, tid=%d", GetCurrentProcessId(), GetCurrentThreadId());
}

int WindowsService::runServiceLoop()
{
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}

int WindowsService::setServiceState(ServiceState state)
{
    m_status.dwCurrentState = static_cast<DWORD>(state);
    if (!SetServiceStatus(m_hStatus, &m_status))
    {
        log(SERVICE_LOG_INFO, "[Service] SetServiceStatus failed, error=%d, pid=%d, tid=%d", GetLastError(), GetCurrentProcessId(), GetCurrentThreadId());
    }
    return 0;
}

DWORD WindowsService::serviceCtrlHandler(DWORD dwControl, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext)
{
	log(SERVICE_LOG_INFO, "[Service] serviceCtrlHandler: control=%d, pid=%d, tid=%d", dwControl, GetCurrentProcessId(), GetCurrentThreadId());

	switch (dwControl)
	{
	case SERVICE_CONTROL_STOP:
		doStop();
		break;
	default:
		return ERROR_CALL_NOT_IMPLEMENTED;
	}

	return NO_ERROR;
}

void WindowsService::doStop()
{
	setStatus(SERVICE_STOP_PENDING);
	PostThreadMessage(m_threadID, WM_QUIT, 0, 0);
}

void WindowsService::setStatus(DWORD dwState)
{
	m_status.dwCurrentState = dwState;
	if (!SetServiceStatus(m_hStatus, &m_status))
	{
		log(SERVICE_LOG_INFO, "[Service] SetServiceStatus failed, error=%d, pid=%d, tid=%d", GetLastError(), GetCurrentProcessId(), GetCurrentThreadId());
	}
}

// the default working directory of service is system32, we need to change it to the directory of exe
void WindowsService::changeWorkingDirectory()
{
	TCHAR path[MAX_PATH];
	if (GetModuleFileName(NULL, path, MAX_PATH))
	{
		int len = lstrlen(path);
		while (len > 0 && path[len] != '\\')
		{
			path[len] = '\0';
			--len;
		}

		SetCurrentDirectory(path);
	}

	char workingDir[MAX_PATH];
	GetCurrentDirectoryA(MAX_PATH, workingDir);
	log(SERVICE_LOG_INFO, "[Service] working directory: %s", workingDir);
}

void WindowsService::log(ServiceLogLevel level, const char* format, ...)
{
	const int len = 1024;
	char msg[len];
	va_list va;
	va_start(va, format);
	vsnprintf_s(msg, len, format, va);
	va_end(va);

	OutputDebugStringA(msg);
}

CVCAM_BASE_NAMESPACE_END


#endif//_WIN32
