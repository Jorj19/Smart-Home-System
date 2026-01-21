//
// Created by bunea on 12.01.2026.
//

#ifndef OOP_EXTENSIONS_H
#define OOP_EXTENSIONS_H

#include <memory>
#include <vector>
#include <string>
#include "Sensor.h"

// cppcheck-suppress unusedFunction
template <typename T>
// cppcheck-suppress unusedFunction
T clampValue(T value, T min, T max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}


template <typename T>

class HistoryBuffer {
private:
    std::vector<T> buffer;
    size_t maxSize;

public:
    explicit HistoryBuffer(size_t size = 10) : maxSize(size) {}

    // cppcheck-suppress unusedFunction
    void add(T item) {
        if (buffer.size() >= maxSize) {
            buffer.erase(buffer.begin());
        }
        buffer.push_back(item);
    }

    // cppcheck-suppress unusedFunction
    void clear() { buffer.clear(); }

    // cppcheck-suppress unusedFunction
    const std::vector<T>& getData() const { return buffer; }
};

class sensorFactory {
public:
    static std::shared_ptr<Sensor> createSensor(int typeIndex, double value);
    static std::shared_ptr<Sensor> createSensorFromString(const std::string& rawType, double value);
    static std::string getSensorName(int typeIndex);

};

#endif //OOP_EXTENSIONS_H