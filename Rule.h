
#ifndef OOP_RULE_H
#define OOP_RULE_H

#include "Sensor.h"
#include <string>

class Rule {
private:
    std::string ruleName;
    std::string targetSensorType;
    double threshold;
    std::string condition;
    std::string alertMessage;
    int alertPriority;
public:
    explicit Rule(std::string name, std::string sensorType, std::string cond, double thres, std::string message, int priority);

    [[nodiscard]] bool isTriggeredBy(const Sensor& sensor) const;

    [[nodiscard]] std::string getMessage() const;
    [[nodiscard]] int getPriority() const;

};


#endif //OOP_RULE_H