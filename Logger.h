//
// Created by bunea on 22.01.2026.
//
#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <mutex>

class Logger {
private:
    Logger() = default;
    std::mutex mtx;

public:
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    static Logger& getInstance();

    void log(const std::string& message);
    void error(const std::string& message);
};

#endif