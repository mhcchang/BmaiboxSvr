#include "defs.h"

#include "service_base.h"

#include <string.h> // for _stricmp
#include <signal.h>
#include <thread> // for sleep
#include <stdio.h>
#include <stdarg.h>

#if defined(__WINDOWS_OS__)
#include "service_windows.h"
#ifndef vsnprintf
#define vsnprintf vsnprintf_s
#endif
#elif defined(__LINUX_OS__)
#include "service_linux.h"
#define _stricmp strcasecmp
#else
#error Unknown platform!
#endif


CVCAM_BASE_NAMESPACE_BEGIN

ServiceImpl::ServiceImpl()
{

}

ServiceImpl::~ServiceImpl()
{

}

void ServiceImpl::init(const ServiceConfig& config, std::function<int(int, char*[])> serviceMain)
{
    m_config = config;
    m_serviceMain = serviceMain;
}

int ServiceImpl::installService()
{ 
    return -1;
}

int ServiceImpl::uninstallService()
{ 
    return -1;
}

int ServiceImpl::startService()
{
    return -1;
}

int ServiceImpl::stopService()
{ 
    return -1;
}

int ServiceImpl::restartService()
{
    return -1;
}

int ServiceImpl::setServiceState(ServiceState state)
{
    return -1;
}

void ServiceImpl::log(ServiceLogLevel level, const char* format, ...)
{

}

///////////////////////////////////////////////////////////////////////////////

ServiceBase::ServiceBase() : m_isRunningAsService(false), m_serviceImpl(nullptr)
{
#if defined(_WIN32)
    m_serviceImpl = new WindowsService();
#elif defined(__linux__)
    m_serviceImpl = new LinuxDaemon();
#endif
}

ServiceBase::~ServiceBase()
{
    delete m_serviceImpl;
}

int ServiceBase::run(const ServiceConfig& config, int argc, char* argv[])
{
    m_serviceConfig = config;
    int res = normalizeServiceConfig();
    if (res != 0)
    {
        log(SERVICE_LOG_ERR, "[Service] invalid service config");
        return res;
    }

    using namespace std::placeholders;
    m_serviceImpl->init(m_serviceConfig, std::bind(&ServiceBase::serviceMain, this, _1, _2));

    if (argc >= 2 && (argv[1][0] == '-' || argv[1][0] == '/'))
    {
        char* cmd = &argv[1][1];

        if (_stricmp(cmd, "install") == 0)
        {
            return installService();
        }
        else if (_stricmp(cmd, "uninstall") == 0)
        {
            return uninstallService();
        }
        else if (_stricmp(cmd, "start") == 0)
        {
            return startService();
        }
        else if (_stricmp(cmd, "stop") == 0)
        {
            return stopService();
        }
        else if (_stricmp(cmd, "help") == 0)
        {
            return showHelp();
        }
        else if (_stricmp(cmd, "service") == 0 || _stricmp(cmd, "daemon") == 0)
        {
            m_isRunningAsService = true;
            return runAsService(argc, argv);
        }
    }

    m_isRunningAsService = false;
    return runInConsole(argc, argv);
}

int ServiceBase::normalizeServiceConfig()
{
    if (m_serviceConfig.name.empty())
        return -1;

    if (m_serviceConfig.displayName.empty())
        m_serviceConfig.displayName = m_serviceConfig.name;

    if (m_serviceConfig.startArgs.empty())
        m_serviceConfig.startArgs = "-service";
    else
        m_serviceConfig.startArgs = "-service " + m_serviceConfig.startArgs;

    return 0;
}

int ServiceBase::installService()
{
    return m_serviceImpl->installService();
}

int ServiceBase::uninstallService()
{
    return m_serviceImpl->uninstallService();
}

int ServiceBase::startService()
{
    return m_serviceImpl->startService();
}

int ServiceBase::stopService()
{
    return m_serviceImpl->stopService();
}

int ServiceBase::restartService()
{
    return m_serviceImpl->restartService();
}

int ServiceBase::showHelp()
{
    return 0;
}

int ServiceBase::runAsService(int argc, char* argv[])
{
    return m_serviceImpl->runAsService(argc, argv);
}

int ServiceBase::serviceMain(int argc, char* argv[])
{
    m_serviceImpl->setServiceState(SERVICE_STATE_START_PENDING);
    if (m_serviceConfig.debug)
    {
        log(SERVICE_LOG_DEBUG, "[Service] starting ...");
    }
    
    int res = onStartService(argc, argv);
    if (res == 0)
    {
        m_serviceImpl->setServiceState(SERVICE_STATE_RUNNING);
        if (m_serviceConfig.debug)
        {
            log(SERVICE_LOG_DEBUG, "[Service] running ...");
        }

        res = runServiceLoop();
        if (res != 0)
        {
            log(SERVICE_LOG_ERR, "[Service] run service loop failed, err=%d", res);
        }

        m_serviceImpl->setServiceState(SERVICE_STATE_STOP_PENDING);
        if (m_serviceConfig.debug)
        {
            log(SERVICE_LOG_DEBUG, "[Service] stopping ...");
        }

        onStopService();
    }
    else
    {
        log(SERVICE_LOG_ERR, "[Service] start failed, err=%d", res);
    }

    onCleanupService();

    m_serviceImpl->setServiceState(SERVICE_STATE_STOPPED);
    if (m_serviceConfig.debug)
    {
        log(SERVICE_LOG_DEBUG, "[Service] stopped");
    }

    return res;
}

int ServiceBase::runInConsole(int argc, char* argv[])
{
    int res = onStartService(argc, argv);
    if (res == 0)
    {
        signal(SIGINT, &ServiceBase::consoleSignalHandler);

        res = runConsoleLoop();
        if (res != 0)
        {
            log(SERVICE_LOG_ERR, "[Service] run console loop failed, err=%d", res);
        }

        onStopService();
    }
    else
    {
        log(SERVICE_LOG_ERR, "[Service] start failed, err=%d", res);
    }

    onCleanupService();

    return res;
}

int ServiceBase::onStartService(int argc, char* argv[])
{
    return 0;
}

void ServiceBase::onStopService()
{

}

void ServiceBase::onCleanupService()
{

}

volatile bool ServiceBase::s_keepConsoleRunning = true;
void ServiceBase::consoleSignalHandler(int sig)
{
    if (sig == SIGINT)
    {
        s_keepConsoleRunning = false;
    }
}

int ServiceBase::runConsoleLoop()
{
    while (s_keepConsoleRunning)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return 0;
}

int ServiceBase::runServiceLoop()
{
    return m_serviceImpl->runServiceLoop();
}

void ServiceBase::log(ServiceLogLevel level, const char* format, ...)
{
    const int len = 1024;
    char msg[len];
    va_list va;
    va_start(va, format);
    vsnprintf(msg, len, format, va);
    va_end(va);
    m_serviceImpl->log(level, "%s", msg);
}

CVCAM_BASE_NAMESPACE_END

