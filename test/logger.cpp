#include "logger.h"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/msvc_sink.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/null_sink.h>

#if defined(__linux__) || defined(__GNUC__)
#include <spdlog/sinks/syslog_sink.h>
#endif

//#include <rapidjson/document.h>
//#include <rapidjson/istreamwrapper.h>
//#include <rapidjson/utils.h>

#include <fstream>
#include <experimental/filesystem>

namespace std {
    namespace fs = std::experimental::filesystem;
}


struct Logger::Context
{
    std::shared_ptr<spdlog::logger> logger;
};

std::mutex Logger::s_instanceMutex;

Logger& Logger::instance()
{
    std::lock_guard<std::mutex> lock(s_instanceMutex);
    static Logger s_instance;
    return s_instance;
}

Logger::Logger() : m_inited(false)
{
    m_context = new Logger::Context;
}

Logger::~Logger()
{
    delete m_context;
}

bool Logger::initDirectly(const std::string& name, int sinkType, int level, const Params& params)
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);

    if (m_inited)
        return false;

    std::string fileName = "logs/" + name + ".log";

    if (sinkType == MHC_SINK_NULL)
    {
        std::shared_ptr<spdlog::sinks::sink> sink = std::make_shared<spdlog::sinks::null_sink_mt>();
        m_context->logger = std::make_shared<spdlog::logger>(name, sink);
    }
    else if (sinkType == MHC_SINK_STDOUT)
    {
        std::shared_ptr<spdlog::sinks::sink> sink = std::make_shared<spdlog::sinks::stdout_sink_mt>();
        m_context->logger = std::make_shared<spdlog::logger>(name, sink);
    }
    else if (sinkType == MHC_SINK_STDOUT_COLOR)
    {
        std::shared_ptr<spdlog::sinks::sink> sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        m_context->logger = std::make_shared<spdlog::logger>(name, sink);
    }
    else if (sinkType == MHC_SINK_STDERR)
    {
        std::shared_ptr<spdlog::sinks::sink> sink = std::make_shared<spdlog::sinks::stderr_sink_mt>();
        m_context->logger = std::make_shared<spdlog::logger>(name, sink);
    }
#if defined(_WIN32) || defined(_WIN64)
    else if (sinkType == MHC_SINK_MSVC)
    {
        std::shared_ptr<spdlog::sinks::sink> sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
        m_context->logger = std::make_shared<spdlog::logger>(name, sink);
    }
#endif
#if defined(linux) || defined(__GNUC__)
    else if (sinkType == MHC_SINK_SYSLOG)
    {
        std::shared_ptr<spdlog::sinks::sink> sink = std::make_shared<spdlog::sinks::syslog_sink_mt>();
        m_context->logger = std::make_shared<spdlog::logger>(name, sink);
    }
#endif
    else
    {
        try
        {
            if (!std::fs::exists("./logs"))
            {
                std::fs::create_directories("./logs");
            }

            if (sinkType == MHC_SINK_FILE)
            {
                std::shared_ptr<spdlog::sinks::sink> sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(fileName);
                m_context->logger = std::make_shared<spdlog::logger>(name, sink);
            }
            else if (sinkType == MHC_SINK_DAILY_FILE)
            {
                std::shared_ptr<spdlog::sinks::sink> sink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(fileName, 23, 59);
                m_context->logger = std::make_shared<spdlog::logger>(name, sink);
            }
            else if (sinkType == MHC_SINK_ROTATING_FILE)
            {
                std::size_t fileSize = params.find("fileSize") != params.end() ?  atoi(params.at("fileSize").c_str()) : 0;
                if (fileSize == 0)
                    fileSize = 100 * 1024 * 1024;

                std::size_t fileCount = params.find("fileCount") != params.end() ? atoi(params.at("fileCount").c_str()) : 0;
                if (fileCount == 0)
                    fileCount = 10;

                std::shared_ptr<spdlog::sinks::sink> sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(fileName, fileSize, fileCount);
                m_context->logger = std::make_shared<spdlog::logger>(name, sink);
            }

            std::size_t flushInterval = params.find("flushInterval") != params.end() ? atoi(params.at("flushInterval").c_str()) : 0;
            if (flushInterval == 0)
                flushInterval = 60;

            spdlog::flush_every(std::chrono::seconds(flushInterval));
        }
        catch (const std::exception& e)
        {
            e;
        }
        catch (...)
        {

        }
    }
    
    if (!m_context->logger)
    {
        std::shared_ptr<spdlog::sinks::sink> sink = std::make_shared<spdlog::sinks::null_sink_mt>();
        m_context->logger = std::make_shared<spdlog::logger>(name, sink);
    }

    m_context->logger->set_level(static_cast<spdlog::level::level_enum>(level));
    spdlog::set_default_logger(m_context->logger);

    m_inited = true;

    return true;
}

bool Logger::initFromConfig(const std::string& name, const std::string& configPath, const Params& params)
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);

    if (m_inited)
        return false;

 /*   std::ifstream ifs(configPath);
    rapidjson::IStreamWrapper isw(ifs);

    rapidjson::Document doc;
    doc.ParseStream(isw);

    if (doc.HasParseError())
        return false;

    if (!doc.HasMember(name.c_str()))
        return false;

    if (!doc[name.c_str()].HasMember("log"))
        return false;

    rapidjson::Value log = doc[name.c_str()]["log"].GetObject();

    int sinkType = MHC_SINK_STDOUT;
    if (log.HasMember("sink"))
    {
        std::string sink = log["sink"].GetString();
        if (sink == "null")
            sinkType = MHC_SINK_NULL;
        else if (sink == "stdout")
            sinkType = MHC_SINK_STDOUT;
        else if (sink == "stdout_color")
            sinkType = MHC_SINK_STDOUT_COLOR;
        else if (sink == "stderr")
            sinkType = MHC_SINK_STDERR;
        else if (sink == "msvc")
            sinkType = MHC_SINK_MSVC;
        else if (sink == "syslog")
            sinkType = MHC_SINK_SYSLOG;
        else if (sink == "file")
            sinkType = MHC_SINK_FILE;
        else if (sink == "daily_file")
            sinkType = MHC_SINK_DAILY_FILE;
        else if (sink == "rotating_file")
            sinkType = MHC_SINK_ROTATING_FILE;
    }

    int level = MHC_LEVEL_INFO;
    if (log.HasMember("level"))
    {
        if (log["level"].IsInt())
        {
            level = log["level"].GetInt();
        }
        else
        {
            level = (int)spdlog::level::from_str(log["level"].GetString());
        }
    }

    Params param2 = params;

    if (log.HasMember("fileSize"))
    {
        int fileSize = rapidjson::ToInt(log["fileSize"]); // in MB
        param2["fileSize"] = std::to_string(fileSize * 1024 * 1024);
    }

    if (log.HasMember("fileCount"))
    {
        int fileCount = rapidjson::ToInt(log["fileCount"]);
        param2["fileCount"] = std::to_string(fileCount);
    }

    if (log.HasMember("flushInterval"))
    {
        int flushInterval = rapidjson::ToInt(log["flushInterval"]); // in seconds
        param2["flushInterval"] = std::to_string(flushInterval);
    }
	
    return initDirectly(name, sinkType, level, param2);
	*/
	return true;
}

bool Logger::init(const std::string& name, int sinkType, int level, const Params& params)
{
    //if (initFromConfig(name, "etc/" + name + ".json", params))
    //    return true;

    return initDirectly(name, sinkType, level, params);
}
