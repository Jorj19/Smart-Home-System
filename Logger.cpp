//
// Created by bunea on 22.01.2026.
//

#include "Logger.h"
#include <iostream>

// cppcheck-suppress unusedFunction
Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

// cppcheck-suppress unusedFunction
void Logger::log(const std::string& message) {
    std::lock_guard<std::mutex> lock(mtx);
    std::cout << "[LOG] " << message << std::endl;
}

// cppcheck-suppress unusedFunction
void Logger::error(const std::string& message) {
    std::lock_guard<std::mutex> lock(mtx);
    std::cerr << "[ERROR] " << message << std::endl;
}