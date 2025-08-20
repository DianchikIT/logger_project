#include "logger.h"
#include <stdexcept>
#include <sstream>

std::ofstream Logger::logFile;
LogLevel Logger::currentLevel = LogLevel::INFO;
std::mutex Logger::logMutex;

void Logger::init(const std::string& filename, LogLevel defaultLevel) {
    std::lock_guard<std::mutex> lock(logMutex);
    if (logFile.is_open()) {
        throw std::runtime_error("Логгер уже инициализирован!");
    }
    currentLevel = defaultLevel;
    logFile.open(filename, std::ios::out | std::ios::app);
    if (!logFile.is_open()) {
        throw std::runtime_error("Не удалось открыть файл лога: " + filename);
    }
}

void Logger::setLogLevel(LogLevel level) {
    std::lock_guard<std::mutex> lock(logMutex);
    currentLevel = level;
}

LogLevel Logger::getCurrentLevel() {
    std::lock_guard<std::mutex> lock(logMutex);
    return currentLevel;
}

void Logger::log(const std::string& message, LogLevel level) {
    std::lock_guard<std::mutex> lock(logMutex);
    
    if (level < currentLevel || !logFile.is_open()) {
        return;
    }
    
    logFile << "[" << getCurrentTime() << "] "
            << "[" << levelToString(level) << "] "
            << message << std::endl;
}

void Logger::shutdown() {
    std::lock_guard<std::mutex> lock(logMutex);
    if (logFile.is_open()) {
        logFile.close();
    }
}

std::string Logger::getCurrentTime() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X");
    return ss.str();
}

std::string Logger::levelToString(LogLevel level) {
    switch(level) {
        case LogLevel::INFO: return "INFO";
        case LogLevel::WARNING: return "WARNING";
        case LogLevel::ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}
