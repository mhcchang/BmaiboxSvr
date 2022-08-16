#ifndef __CVCAM_SERVICE_BASE_H__
#define __CVCAM_SERVICE_BASE_H__

#include <string>
#include <functional>

#ifndef CVCAM_NAMESPACE_BEGIN
#define CVCAM_NAMESPACE_BEGIN namespace cvcam {
#define CVCAM_NAMESPACE_END }
#endif

#ifndef CVCAM_BASE_NAMESPACE_BEGIN
#define CVCAM_BASE_NAMESPACE_BEGIN namespace base {
#define CVCAM_BASE_NAMESPACE_END }
#endif

//CVCAM_NAMESPACE_BEGIN
CVCAM_BASE_NAMESPACE_BEGIN

enum ServiceState
{
    SERVICE_STATE_STOPPED          = 0x00000001,
    SERVICE_STATE_START_PENDING    = 0x00000002,
    SERVICE_STATE_STOP_PENDING     = 0x00000003,
    SERVICE_STATE_RUNNING          = 0x00000004,
};

enum WinServiceStartType
{
    WIN_SERVICE_BOOT_START      = 0,
    WIN_SERVICE_SYSTEM_START    = 1,
    WIN_SERVICE_AUTO_START      = 2,
    WIN_SERVICE_DEMAND_START    = 3,
    WIN_SERVICE_DISABLED        = 4,
    WIN_SERVICE_DELAYED_AUTO_START  = 10,
};

enum ServiceLogLevel : int
{
    SERVICE_LOG_EMERG       = 0,
    SERVICE_LOG_ALERT       = 1,
    SERVICE_LOG_CRIT        = 2,
    SERVICE_LOG_ERR         = 3,
    SERVICE_LOG_WARNING     = 4,
    SERVICE_LOG_NOTICE      = 5,
    SERVICE_LOG_INFO        = 6,
    SERVICE_LOG_DEBUG       = 7,
};

struct ServiceConfig
{
    std::string name; // required
    std::string displayName;
    std::string description;

    std::string startArgs; // 

    // windows-specific config
    WinServiceStartType win_startType = WIN_SERVICE_DELAYED_AUTO_START;
    std::string win_dependencies; // use \0 as the seperator of multiple dependencies
    std::string win_username;
    std::string win_password;

    // linux-specific config

    // other common config
    bool debug = false;
};

class ServiceImpl
{
public:
    ServiceImpl();
    virtual ~ServiceImpl();
    virtual void init(const ServiceConfig& config, std::function<int(int, char*[])> serviceMain);
    virtual int installService();
    virtual int uninstallService();
    virtual int startService();
    virtual int stopService();
    virtual int restartService();
    virtual int runAsService(int argc, char* argv[]) = 0;
    virtual int runServiceLoop() = 0;
    virtual int setServiceState(ServiceState state);
    virtual void log(ServiceLogLevel level, const char* format, ...);

protected:
    ServiceConfig m_config;
    std::function<int(int, char*[])> m_serviceMain;
};

class ServiceBase
{
public:
    ServiceBase();
    virtual ~ServiceBase();

    virtual int run(const ServiceConfig& config, int argc, char* argv[]);

protected:
    const ServiceConfig& serviceConfig() const { return m_serviceConfig; }

    virtual int normalizeServiceConfig();

    virtual int installService();
    virtual int uninstallService();
    virtual int startService();
    virtual int stopService();
    virtual int restartService();
    virtual int showHelp();

    bool isRunningAsService() const { return m_isRunningAsService; }
    bool isRunningInConsole() const { return !m_isRunningAsService; }

    virtual int runAsService(int argc, char* argv[]);
    virtual int runInConsole(int argc, char* argv[]);

    virtual int serviceMain(int argc, char* argv[]);
    virtual int onStartService(int argc, char* argv[]);
    virtual void onStopService();
    virtual void onCleanupService();
    virtual int runConsoleLoop();
    virtual int runServiceLoop();

    virtual void log(ServiceLogLevel level, const char* format, ...);

private:
    static void consoleSignalHandler(int sig);

private:
    bool m_isRunningAsService;
    ServiceConfig m_serviceConfig;
    ServiceImpl* m_serviceImpl;
    static volatile bool s_keepConsoleRunning;
};

CVCAM_BASE_NAMESPACE_END
//CVCAM_NAMESPACE_END

#endif//__CVCAM_SERVICE_BASE_H__
