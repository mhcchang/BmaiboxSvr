#if __LINUX_OS__

#include "service_linux.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>
#include <signal.h>
#include <limits.h>
#include <stdarg.h>
#include <thread>
#include <sstream>
#include <fstream>
#include <experimental/filesystem>

namespace std {
	namespace fs = std::experimental::filesystem;
}



CVCAM_BASE_NAMESPACE_BEGIN


LinuxDaemon::LinuxDaemon()
{
    changeWorkingDirectory();
}

LinuxDaemon::~LinuxDaemon()
{

}

std::string LinuxDaemon::unitName()
{
    return m_config.name + ".service";
}

std::string LinuxDaemon::unitPath()
{
    return "/lib/systemd/system/" + unitName();
}

std::string LinuxDaemon::exePath()
{
    char path[PATH_MAX] = { 0 };
    ssize_t len = readlink("/proc/self/exe", path, PATH_MAX);
    if (len == -1)
        return "";
    return path;
}

void LinuxDaemon::changeWorkingDirectory()
{
    std::string filePath = exePath();
    if (filePath.empty())
    {
        log(SERVICE_LOG_ERR, "[Service] get exe path failed");
        return;
    }

    std::fs::path p(filePath);
    std::string dir = p.parent_path().string();

    int err = chdir(dir.c_str());
    if (err != 0)
    {
        log(SERVICE_LOG_ERR, "[Service] change working directory failed, err=%d", err);
    }

    log(SERVICE_LOG_INFO, "[Service] working directory: %s", dir.c_str());
}

int LinuxDaemon::systemCtl(const char* cmd, const char* params)
{
    if (params)
        return execlp("systemctl", "systemctl", cmd, params, NULL);
    else
        return execlp("systemctl", "systemctl", cmd, NULL);
}

int LinuxDaemon::installService()
{
    std::string configPath = unitPath();
    std::string execStart = exePath();

    if (execStart.empty())
    {
        log(SERVICE_LOG_ERR, "[Service] get exe path failed");
        return -1;
    }

    std::stringstream ss;
    ss << "[Unit]" << std::endl;
    ss << "Description=" << m_config.description << std::endl;
    //ss << "ConditionFileIsExecutable=" << m_config.description << std::endl;
    ss << "[Service]" << std::endl;
    ss << "ExecStart=" << execStart << std::endl;
    ss << "Restart=always" << std::endl;
    ss << "RestartSec=60" << std::endl;
    ss << "[Install]" << std::endl;
    ss << "WantedBy=multi-user.target" << std::endl;
    std::string systemdScript = ss.str();

    std::ofstream ofs(configPath, std::ios_base::out|std::ios_base::trunc);
    if (!ofs.is_open())
    {
        log(SERVICE_LOG_ERR, "[Service] open unit file failed");
        return -1;
    }

    ofs << systemdScript;
    ofs.close();

    std::string cmd = "exec";
    systemCtl("enable", unitName().c_str());
    systemCtl("daemon-reload");

    return 0;
}

int LinuxDaemon::uninstallService()
{
    if (access(unitPath().c_str(), F_OK) != 0)
        return 0;

    unlink(unitPath().c_str());

    return systemCtl("disable", unitName().c_str());
}

int LinuxDaemon::startService()
{
    return systemCtl("start", unitName().c_str());
}

int LinuxDaemon::stopService()
{
    return systemCtl("stop", unitName().c_str());
}

int LinuxDaemon::restartService()
{
    return systemCtl("restart", unitName().c_str());
}

int LinuxDaemon::runAsService(int argc, char* argv[])
{
    daemonize();

    signal(SIGINT, &LinuxDaemon::signalHandler);
    signal(SIGHUP, &LinuxDaemon::signalHandler);

    s_keepServiceRunning = true;
    m_serviceMain(argc, argv);

    return 0;
}

int LinuxDaemon::daemonize()
{
    pid_t pid = fork();
    if (pid < 0)
        return -1;

    if (pid > 0)
        exit(0);

    if (setsid() < 0)
        return -1;

    umask(0);

    //if (chdir("/") < 0)
    //{
    //    exit(EXIT_FAILURE);
    //}

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    stdin = fopen("/dev/null", "r");
    stdout = fopen("/dev/null", "w+");
    stderr = fopen("/dev/null", "w+");

    return 0;
}

volatile bool LinuxDaemon::s_keepServiceRunning = true;

void LinuxDaemon::signalHandler(int sig)
{
    if (sig == SIGINT)
    {
        s_keepServiceRunning = false;
    }
    else if (sig == SIGHUP)
    {
        // reload config
    }
}

int LinuxDaemon::runServiceLoop()
{
    while (s_keepServiceRunning)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return 0;
}

void LinuxDaemon::log(ServiceLogLevel level, const char* format, ...)
{
    const int len = 1024;
    char msg[len];
    va_list va;
    va_start(va, format);
    vsnprintf(msg, len, format, va);
    va_end(va);
    syslog(level, "%s", msg);
}

CVCAM_BASE_NAMESPACE_END


#endif // __linux__
