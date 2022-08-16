#ifndef __MHC_LOGGER_H__
#define __MHC_LOGGER_H__

#include <string>
#include <map>
#include <mutex>

#include <spdlog/spdlog.h>

// logger level
#define MHC_LEVEL_TRACE       0
#define MHC_LEVEL_DEBUG       1
#define MHC_LEVEL_INFO        2
#define MHC_LEVEL_WARN        3
#define MHC_LEVEL_ERROR       4
#define MHC_LEVEL_CRITICAL    5
#define MHC_LEVEL_OFF         6

// logger sinks
#define MHC_SINK_NULL             0
#define MHC_SINK_STDOUT           1
#define MHC_SINK_STDOUT_COLOR     2
#define MHC_SINK_STDERR           3
#define MHC_SINK_MSVC             4
#define MHC_SINK_SYSLOG           5
#define MHC_SINK_FILE             6
#define MHC_SINK_DAILY_FILE       7
#define MHC_SINK_ROTATING_FILE    8


#ifndef MHC_ACTIVE_LEVEL
#define MHC_ACTIVE_LEVEL  MHC_LEVEL_TRACE
#endif

class Logger
{
public:
    static Logger& instance();

    typedef std::map<std::string, std::string> Params;

public:
    Logger();
    ~Logger();

    bool initDirectly(const std::string& name, int sinkType, int level, const Params& params = Params());
    bool initFromConfig(const std::string& name, const std::string& configPath, const Params& params = Params());
    bool init(const std::string& name, int sinkType = MHC_SINK_STDOUT, int level = MHC_LEVEL_WARN, const Params& params = Params());

protected:
    static std::mutex s_instanceMutex;
    std::recursive_mutex m_mutex;
    bool m_inited;
    int m_sinkType;
    int m_level;
    struct Context;
    Context* m_context;
};

#define MHC_LOGGER_INIT(name, ...) Logger::instance().init(name, ##__VA_ARGS__)

#define MHC_LOGGER_CALL(logger, level, ...) \
if (logger->should_log(level)) \
    logger->log(spdlog::source_loc{}, level, __VA_ARGS__)
    //logger->log(spdlog::source_loc{SPDLOG_FILE_BASENAME(__FILE__), __LINE__, SPDLOG_FUNCTION}, level, __VA_ARGS__)

#if MHC_ACTIVE_LEVEL <= MHC_LEVEL_TRACE
#define MHC_LOGGER_TRACE(logger, ...) MHC_LOGGER_CALL(logger, spdlog::level::trace, __VA_ARGS__)
#define MHC_TRACE(...) MHC_LOGGER_TRACE(spdlog::default_logger_raw(), __VA_ARGS__)
#else
#define MHC_LOGGER_TRACE(logger, ...) (void)0
#define MHC_TRACE(...) (void)0
#endif

#if MHC_ACTIVE_LEVEL <= MHC_LEVEL_DEBUG
#define MHC_LOGGER_DEBUG(logger, ...) MHC_LOGGER_CALL(logger, spdlog::level::debug, __VA_ARGS__)
#define MHC_DEBUG(...) MHC_LOGGER_DEBUG(spdlog::default_logger_raw(), __VA_ARGS__)
#else
#define MHC_LOGGER_DEBUG(logger, ...) (void)0
#define MHC_DEBUG(...) (void)0
#endif

#if MHC_ACTIVE_LEVEL <= MHC_LEVEL_INFO
#define MHC_LOGGER_INFO(logger, ...) MHC_LOGGER_CALL(logger, spdlog::level::info, __VA_ARGS__)
#define MHC_INFO(...) MHC_LOGGER_INFO(spdlog::default_logger_raw(), __VA_ARGS__)
#else
#define MHC_LOGGER_INFO(logger, ...) (void)0
#define MHC_INFO(...) (void)0
#endif

#if MHC_ACTIVE_LEVEL <= MHC_LEVEL_WARN
#define MHC_LOGGER_WARN(logger, ...) MHC_LOGGER_CALL(logger, spdlog::level::warn, __VA_ARGS__)
#define MHC_WARN(...) MHC_LOGGER_WARN(spdlog::default_logger_raw(), __VA_ARGS__)
#else
#define MHC_LOGGER_WARN(logger, ...) (void)0
#define MHC_WARN(...) (void)0
#endif

#if MHC_ACTIVE_LEVEL <= MHC_LEVEL_ERROR
#define MHC_LOGGER_ERROR(logger, ...) MHC_LOGGER_CALL(logger, spdlog::level::err, __VA_ARGS__)
#define MHC_ERROR(...) MHC_LOGGER_ERROR(spdlog::default_logger_raw(), __VA_ARGS__)
#else
#define MHC_LOGGER_ERROR(logger, ...) (void)0
#define MHC_ERROR(...) (void)0
#endif

#if MHC_ACTIVE_LEVEL <= MHC_LEVEL_CRITICAL
#define MHC_LOGGER_CRITICAL(logger, ...) MHC_LOGGER_CALL(logger, spdlog::level::critical, __VA_ARGS__)
#define MHC_CRITICAL(...) MHC_LOGGER_CRITICAL(spdlog::default_logger_raw(), __VA_ARGS__)
#else
#define MHC_LOGGER_CRITICAL(logger, ...) (void)0
#define MHC_CRITICAL(...) (void)0
#endif

#define MHC_FLUSH_LOG()   spdlog::default_logger_raw()->flush()


#endif//__MHC_LOGGER_H__
