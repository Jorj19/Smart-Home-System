//
// Created by bunea on 22.01.2026.
//
#ifndef LOGGER_H
#define LOGGER_H

#include <string>

class Logger {
private:
    Logger() {} // Constructor privat

public:
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    static Logger& getInstance(); // Metoda singleton

    void log(const std::string& message);
    void error(const std::string& message);
};

#endif