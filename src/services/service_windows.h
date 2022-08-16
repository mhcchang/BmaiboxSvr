#ifndef __CVCAM_WINDOWS_SERVICE_H__
#define __CVCAM_WINDOWS_SERVICE_H__

#include "service_base.h"
#include <Windows.h>
#include <winsvc.h>

#include <string>

//CVCAM_NAMESPACE_BEGIN
CVCAM_BASE_NAMESPACE_BEGIN

class WindowsService : public ServiceImpl
{
private:
	static WindowsService* s_pInstance;
	static WindowsService& instance();

public:
	WindowsService();
	virtual ~WindowsService();

public:
	virtual int installService() override;
    virtual int uninstallService() override;
    virtual int startService() override;
    virtual int stopService() override;
    virtual int restartService() override;
    virtual int runAsService(int argc, char* argv[]) override;
    virtual int runServiceLoop() override;
    virtual int setServiceState(ServiceState state) override;
    virtual void log(ServiceLogLevel level, const char* format, ...) override;

protected:
	static void WINAPI _serviceMain(DWORD dwNumServiceArgs, LPSTR* lpServiceArgVectors);
	static DWORD WINAPI _serviceCtrlHandler(DWORD dwControl, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext);

protected:
	virtual void serviceMain(DWORD dwNumServiceArgs, LPSTR* lpServiceArgVectors);
	DWORD serviceCtrlHandler(DWORD dwControl, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext);
	virtual void doStop();

	void setStatus(DWORD dwState);

	void changeWorkingDirectory();

private:
	SERVICE_STATUS_HANDLE m_hStatus;
	SERVICE_STATUS m_status;
	DWORD m_threadID;
};

CVCAM_BASE_NAMESPACE_END
//CVCAM_NAMESPACE_END

#endif//__CVCAM_WINDOWS_SERVICE_H__
