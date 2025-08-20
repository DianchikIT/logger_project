#include "logger.h"
#include <iostream>
#include <cassert>
#include <sstream>
#include <fstream>

std::string readFile(const std::string& filename) {
    std::ifstream file(filename);
    std::stringstream buffer;
    buffer << file.rdbuf();  
    return buffer.str();
}

void clearFile(const std::string& filename) {
    std::ofstream file(filename, std::ios::trunc);
    file.close();
}

void test_levelToString() {
    assert(Logger::levelToString(LogLevel::INFO) == "INFO");
    assert(Logger::levelToString(LogLevel::WARNING) == "WARNING");
    assert(Logger::levelToString(LogLevel::ERROR) == "ERROR");
    std::cout << "test_levelToString — пройден\n";
}

void test_log_message() {
    const std::string testFile = "test_log.txt";
    clearFile(testFile);
    std::cout << "Запуск: test_log_message\n";
    Logger::init(testFile, LogLevel::INFO);
    Logger::log("Тестовое сообщение", LogLevel::INFO);
    Logger::shutdown();
    std::string content = readFile(testFile);
    assert(content.find("Тестовое сообщение") != std::string::npos && "Ошибка: сообщение не найдено в файле");
    assert(content.find("[INFO]") != std::string::npos && "Ошибка: уровень [INFO] не найден");
    std::cout << "test_log_message — пройден\n";
}

void test_log_filter() {
    const std::string testFile = "test_filter.txt";
    clearFile(testFile);
    std::cout << "Запуск: test_log_filter\n";
    Logger::init(testFile, LogLevel::ERROR);
    Logger::log("Сообщение INFO", LogLevel::INFO);
    Logger::log("Сообщение ERROR", LogLevel::ERROR);
    Logger::shutdown();
    std::string content = readFile(testFile);
    assert(content.find("Сообщение INFO") == std::string::npos && "Ошибка: сообщение INFO было записано, хотя не должно");
    assert(content.find("Сообщение ERROR") != std::string::npos && "Ошибка: сообщение ERROR не было записано");
    std::cout << "test_log_filter — пройден\n";
}

int main() {
    std::cout << "Запуск тестов библиотеки Logger\n";

    try {
        test_levelToString();
        test_log_message();
        test_log_filter();
        std::cout << "Все тесты успешно пройдены!\n";
    } catch (const std::exception& e) {
        std::cerr << "Произошла ошибка во время теста: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Неизвестная ошибка во время тестирования.\n";
        return 1;
    }

    return 0;
}
