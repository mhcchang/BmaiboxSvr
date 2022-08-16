#ifndef __CVCAM_LINUX_DAEMON_H__
#define __CVCAM_LINUX_DAEMON_H__

#include "service_base.h"


CVCAM_BASE_NAMESPACE_BEGIN

class LinuxDaemon : public ServiceImpl
{
public:
    LinuxDaemon();
    ~LinuxDaemon();

    virtual int installService() override;
    virtual int uninstallService() override;
    virtual int startService() override;
    virtual int stopService() override;
    virtual int restartService() override;
    virtual int runAsService(int argc, char* argv[]) override;
    virtual int runServiceLoop() override;
    virtual void log(ServiceLogLevel level, const char* format, ...) override;

protected:
    std::string unitName();
    std::string unitPath();
    std::string exePath();
    int systemCtl(const char* cmd, const char* params = nullptr);
    void changeWorkingDirectory();

private:
    int daemonize();
    static void signalHandler(int sig);

private:
    static volatile bool s_keepServiceRunning;
};

CVCAM_BASE_NAMESPACE_END


#endif//__CVCAM_LINUX_DAEMON_H__
