#include "Logger.h"
#include <iostream>
#include <iomanip>
#include <ctime>
#include <thread>

namespace Feliss {

Logger& Logger::get() {
    static Logger s_instance;
    return s_instance;
}

Logger::Logger()  = default;
Logger::~Logger() {
    if (m_file.is_open()) m_file.close();
}

void Logger::setMinLevel(LogLevel lvl) {
    m_minLevel.store(static_cast<uint32_t>(lvl));
}
void Logger::setOutputFile(const std::string& path) {
    std::lock_guard<std::mutex> lk(m_mutex);
    if (m_file.is_open()) m_file.close();
    m_file.open(path, std::ios::app);
}
void Logger::setColorEnabled(bool v) { m_colorEnabled = v; }
void Logger::addCallback(LogCallback cb) {
    std::lock_guard<std::mutex> lk(m_mutex);
    m_callbacks.push_back(std::move(cb));
}
void Logger::clearCallbacks() {
    std::lock_guard<std::mutex> lk(m_mutex);
    m_callbacks.clear();
}

void Logger::log(LogLevel lvl, std::string_view tag, std::string_view msg) {
    if (static_cast<uint32_t>(lvl) < m_minLevel.load()) return;

    LogEntry e;
    e.level     = lvl;
    e.tag       = std::string(tag);
    e.message   = std::string(msg);
    e.timestamp = std::chrono::system_clock::now();
    e.threadId  = std::hash<std::thread::id>{}(std::this_thread::get_id());

    std::lock_guard<std::mutex> lk(m_mutex);
    if (m_entries.size() >= m_maxEntries) m_entries.erase(m_entries.begin());
    m_entries.push_back(e);
    writeConsole(e);
    if (m_file.is_open()) writeFile(e);
    for (auto& cb : m_callbacks) cb(e);
}

void Logger::writeConsole(const LogEntry& e) {
    if (m_colorEnabled) std::cout << levelColor(e.level);
    std::cout << formatEntry(e);
    if (m_colorEnabled) std::cout << "\033[0m";
    std::cout << '\n';
}
void Logger::writeFile(const LogEntry& e) {
    m_file << formatEntry(e) << '\n';
    m_file.flush();
}

std::string Logger::formatEntry(const LogEntry& e) const {
    auto t  = std::chrono::system_clock::to_time_t(e.timestamp);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                  e.timestamp.time_since_epoch()) % 1000;
    std::tm tm_buf{};
#ifdef _WIN32
    localtime_s(&tm_buf, &t);
#else
    localtime_r(&t, &tm_buf);
#endif
    char timeBuf[32];
    std::strftime(timeBuf, sizeof(timeBuf), "%H:%M:%S", &tm_buf);

    std::ostringstream ss;
    ss << '[' << timeBuf << '.'
       << std::setfill('0') << std::setw(3) << ms.count() << ']'
       << " [" << levelLabel(e.level) << ']'
       << " [" << e.tag << "] "
       << e.message;
    return ss.str();
}

const char* Logger::levelLabel(LogLevel l) const {
    switch(l) {
        case LogLevel::Trace:   return "TRACE";
        case LogLevel::Debug:   return "DEBUG";
        case LogLevel::Info:    return "INFO ";
        case LogLevel::Warning: return "WARN ";
        case LogLevel::Error:   return "ERROR";
        case LogLevel::Fatal:   return "FATAL";
        default:                return "?????";
    }
}
const char* Logger::levelColor(LogLevel l) const {
    switch(l) {
        case LogLevel::Trace:   return "\033[90m";
        case LogLevel::Debug:   return "\033[36m";
        case LogLevel::Info:    return "\033[32m";
        case LogLevel::Warning: return "\033[33m";
        case LogLevel::Error:   return "\033[31m";
        case LogLevel::Fatal:   return "\033[35m";
        default:                return "\033[0m";
    }
}
const std::vector<LogEntry>& Logger::entries() const { return m_entries; }
void Logger::clearEntries() {
    std::lock_guard<std::mutex> lk(m_mutex);
    m_entries.clear();
}
void Logger::setMaxEntries(size_t n) {
    std::lock_guard<std::mutex> lk(m_mutex);
    m_maxEntries = n;
}

} // namespace Feliss
