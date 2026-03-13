#pragma once
#include "feliss/Types.h"
#include <sstream>
#include <mutex>
#include <fstream>
#include <functional>
#include <chrono>
#include <vector>
#include <atomic>

namespace Feliss {

struct LogEntry {
    LogLevel    level  = LogLevel::Info;
    std::string tag;
    std::string message;
    std::chrono::system_clock::time_point timestamp;
    uint64_t    threadId = 0;
};

using LogCallback = std::function<void(const LogEntry&)>;

class Logger {
public:
    Logger();
    ~Logger();
    Logger(const Logger&)            = delete;
    Logger& operator=(const Logger&) = delete;

    void setMinLevel(LogLevel lvl);
    void setOutputFile(const std::string& path);
    void setColorEnabled(bool v);
    void addCallback(LogCallback cb);
    void clearCallbacks();

    void log(LogLevel lvl, std::string_view tag, std::string_view msg);

    void trace  (std::string_view tag, std::string_view msg) { log(LogLevel::Trace,   tag, msg); }
    void debug  (std::string_view tag, std::string_view msg) { log(LogLevel::Debug,   tag, msg); }
    void info   (std::string_view tag, std::string_view msg) { log(LogLevel::Info,    tag, msg); }
    void warning(std::string_view tag, std::string_view msg) { log(LogLevel::Warning, tag, msg); }
    void error  (std::string_view tag, std::string_view msg) { log(LogLevel::Error,   tag, msg); }
    void fatal  (std::string_view tag, std::string_view msg) { log(LogLevel::Fatal,   tag, msg); }

    const std::vector<LogEntry>& entries() const;
    void clearEntries();
    void setMaxEntries(size_t n);

    static Logger& get();

private:
    std::string formatEntry(const LogEntry& e) const;
    const char* levelLabel(LogLevel l) const;
    const char* levelColor(LogLevel l) const;
    void        writeConsole(const LogEntry& e);
    void        writeFile(const LogEntry& e);

    std::atomic<uint32_t>     m_minLevel { static_cast<uint32_t>(LogLevel::Debug) };
    bool                      m_colorEnabled { true };
    std::ofstream             m_file;
    mutable std::mutex        m_mutex;
    std::vector<LogCallback>  m_callbacks;
    std::vector<LogEntry>     m_entries;
    size_t                    m_maxEntries { 4096 };
};

} // namespace Feliss

// ---- Macros ----
#define FLS_LOG(lvl, tag, msg)   ::Feliss::Logger::get().log(lvl, tag, msg)
#define FLS_TRACE(tag, msg)      ::Feliss::Logger::get().trace(tag, msg)
#define FLS_DEBUG(tag, msg)      ::Feliss::Logger::get().debug(tag, msg)
#define FLS_INFO(tag, msg)       ::Feliss::Logger::get().info(tag, msg)
#define FLS_WARN(tag, msg)       ::Feliss::Logger::get().warning(tag, msg)
#define FLS_ERROR(tag, msg)      ::Feliss::Logger::get().error(tag, msg)
#define FLS_FATAL(tag, msg)      ::Feliss::Logger::get().fatal(tag, msg)

// Stream-style formatted macros
#define FLS_INFOF(tag, stream_expr) do { \
    std::ostringstream _fls_ss; _fls_ss << stream_expr; \
    ::Feliss::Logger::get().info(tag, _fls_ss.str()); } while(0)
#define FLS_WARNF(tag, stream_expr) do { \
    std::ostringstream _fls_ss; _fls_ss << stream_expr; \
    ::Feliss::Logger::get().warning(tag, _fls_ss.str()); } while(0)
#define FLS_ERRORF(tag, stream_expr) do { \
    std::ostringstream _fls_ss; _fls_ss << stream_expr; \
    ::Feliss::Logger::get().error(tag, _fls_ss.str()); } while(0)

#define FLS_ASSERT(cond, msg) do { \
    if (!(cond)) { \
        ::Feliss::Logger::get().fatal("Assert", \
            std::string("ASSERT FAILED: " #cond " | ") + (msg)); \
        std::abort(); \
    } } while(0)
