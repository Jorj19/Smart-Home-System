//
// Created by bunea on 22.01.2026.
//

#include "Logger.h"
#include <iostream>

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

void Logger::log(const std::string& message) {
    std::lock_guard<std::mutex> lock(mtx);
    std::cout << "[LOG] " << message << std::endl;
}

void Logger::error(const std::string& message) {
    std::lock_guard<std::mutex> lock(mtx);
    std::cerr << "[ERROR] " << message << std::endl;
}