//
// Created by bunea on 12.01.2026.
//

#ifndef OOP_EXTENSIONS_H
#define OOP_EXTENSIONS_H

#include <memory>
#include "Sensor.h"
class Extensions {
};


class sensorFactory {
public:
    static std::shared_ptr<Sensor> createSensor(int typeIndex, double value);
    static std::shared_ptr<Sensor> createSensorFromString(const std::string& rawType, double value);
    std::shared_ptr<Sensor> getSensorName(int typeIndex);
};

#endif //OOP_EXTENSIONS_H