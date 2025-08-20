#include <iostream>
#include <string>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include "logger.h"


std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    size_t end = s.find_last_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    return s.substr(start, end - start + 1);
}

class LogQueue {
private:
    std::queue<std::pair<std::string, LogLevel>> messages;
    std::mutex mtx;
    std::condition_variable cv;
    std::atomic<bool> running{true};

public:
    void push(const std::string& message, LogLevel level) {
        std::lock_guard<std::mutex> lock(mtx);
        messages.emplace(message, level);
        cv.notify_one();
    }

    bool pop(std::string& message, LogLevel& level) {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this]() { return !messages.empty() || !running; });

        if (messages.empty() && !running) {
            return false;
        }

        message = messages.front().first;
        level = messages.front().second;
        messages.pop();
        return true;
    }

    void shutdown() {
        running = false;
        cv.notify_all();
    }
};

void logWorker(LogQueue& queue) {
    std::string message;
    LogLevel level;

    while (queue.pop(message, level)) {
        try {
            Logger::log(message, level);
        } catch (const std::exception& e) {
            std::cerr << "Ошибка записи в лог: " << e.what() << std::endl;
        }
    }
}

LogLevel parseLogLevel(const std::string& levelStr) {
    if (levelStr == "INFO") return LogLevel::INFO;
    if (levelStr == "WARNING") return LogLevel::WARNING;
    if (levelStr == "ERROR") return LogLevel::ERROR;
    throw std::invalid_argument("Неизвестный уровень: " + levelStr);
}

void printUsage() {
    std::cout << "Использование: logger_app <файл_лога> <уровень>" << std::endl;
    std::cout << "Доступные уровни: INFO, WARNING, ERROR" << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printUsage();
        return 1;
    }

    std::string logFile = argv[1];
    LogLevel defaultLevel;

    try {
        defaultLevel = parseLogLevel(argv[2]);
    } catch (const std::invalid_argument& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        printUsage();
        return 1;
    }

    try {
        Logger::init(logFile, defaultLevel);
    } catch (const std::exception& e) {
        std::cerr << "Ошибка инициализации логгера: " << e.what() << std::endl;
        return 1;
    }

    LogQueue logQueue;
    std::thread worker(logWorker, std::ref(logQueue));

    std::cout << "Логгер запущен. Введите 'exit' для выхода или 'level <УРОВЕНЬ>' для смены уровня." << std::endl;

    std::string input;
    while (true) {
        std::cout << "Введите сообщение и уровень (например, 'Привет INFO'): ";
        std::getline(std::cin, input);
      
        std::string trimmed = trim(input);
      
        if (trimmed.empty()) {
            continue;
        }

        if (trimmed == "exit") {
            break;
        }

        if (trimmed.rfind("level ", 0) == 0) {
            std::string newLevelStr = trim(trimmed.substr(6));
            if (newLevelStr.empty()) {
                std::cerr << "Ошибка: Не указан уровень после 'level'. Пример: 'level WARNING'" << std::endl;
                continue;
            }
            try {
                LogLevel newLevel = parseLogLevel(newLevelStr);
                Logger::setLogLevel(newLevel);
                std::cout << "Уровень логгирования изменён на: " << newLevelStr << std::endl;
            } catch (const std::invalid_argument& e) {
                std::cerr << "Ошибка: " << e.what() << std::endl;
            }
            continue;
        }

        try {
            parseLogLevel(trimmed);
            std::cerr << "Ошибка: Введено только название уровня '" << trimmed
                      << "', но нет текста сообщения. Используйте формат: 'сообщение " << trimmed << "'" << std::endl;
            continue;
        } catch (const std::invalid_argument&) {
        
        }

        size_t lastSpace = input.rfind(' ');
        std::string message;
        LogLevel level = Logger::getCurrentLevel();

        if (lastSpace != std::string::npos) {
            std::string levelStr = trim(input.substr(lastSpace + 1));
            try {
                level = parseLogLevel(levelStr);
                message = trim(input.substr(0, lastSpace));
                if (message.empty()) {
                    std::cerr << "Ошибка: Сообщение пустое." << std::endl;
                    continue;
                }
            } catch (const std::invalid_argument&) {
                message = trimmed;
            }
        } else {
            message = trimmed;
        }

        logQueue.push(message, level);
    }

    logQueue.shutdown();
    worker.join();
    Logger::shutdown();

    return 0;
}
