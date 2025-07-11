#pragma once

#include <string>
#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <filesystem>
#include <chrono>

namespace FelissCore {

// Memory Manager
class MemoryManager {
public:
    static void* Allocate(size_t size) {
        return ::operator new(size);
    }

    static void Deallocate(void* ptr) {
        ::operator delete(ptr);
    }
};

enum class LogLevel {
    Info,
    Warning,
    Error,
    Debug
};

class Logger {
public:
    static void Log(const std::string& message, LogLevel level = LogLevel::Info) {
        const char* prefix;
        switch (level) {
            case LogLevel::Info:    prefix = "[INFO] "; break;
            case LogLevel::Warning: prefix = "[WARN] "; break;
            case LogLevel::Error:   prefix = "[ERR!] "; break;
            case LogLevel::Debug:   prefix = "[DBG ] "; break;
        }
        std::cout << prefix << message << std::endl;
    }
};

class FileSystem {
public:
    static bool Exists(const std::string& path) {
        return std::filesystem::exists(path);
    }

    static std::string ReadTextFile(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            Logger::Log("Failed to open file: " + path, LogLevel::Error);
            return "";
        }

        std::string content((std::istreambuf_iterator<char>(file)),
                             std::istreambuf_iterator<char>());
        return content;
    }

    static bool WriteTextFile(const std::string& path, const std::string& data) {
        std::ofstream file(path);
        if (!file.is_open()) {
            Logger::Log("Failed to write file: " + path, LogLevel::Error);
            return false;
        }
        file << data;
        return true;
    }
};

// Cross-Platform
class Timer {
public:
    static double GetTimeSeconds() {
        using namespace std::chrono;
        auto now = high_resolution_clock::now();
        auto duration = now.time_since_epoch();
        return duration_cast<duration<double>>(duration).count();
    }
};

} 
