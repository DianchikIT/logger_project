#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <fstream>
#include <mutex>
#include <chrono>
#include <iomanip>
#include <ctime>

enum class LogLevel {
    INFO,
    WARNING,
    ERROR
};

class Logger {
public:
    static void init(const std::string& filename, LogLevel defaultLevel = LogLevel::INFO);
    static void setLogLevel(LogLevel level);
    static LogLevel getCurrentLevel();
    static void log(const std::string& message, LogLevel level = LogLevel::INFO);
    static std::string levelToString(LogLevel level);
    static void shutdown();

private:
    static std::ofstream logFile;
    static LogLevel currentLevel;
    static std::mutex logMutex;
    static std::string getCurrentTime();
};

#endif // LOGGER_H
