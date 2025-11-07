//
// Created by bunea on 07.11.2025.
//

#include "Rule.h"

Rule::Rule(std::string name, std::string sensorType, std::string cond, double thres, std::string message, int priority)
    : ruleName(std::move(name)), targetSensorType(std::move(sensorType)), threshold(thres), condition(std::move(cond)), alertMessage(std::move(message)), alertPriority(priority) {}

std::string Rule::getMessage() const {
    return this->alertMessage;
}

int Rule::getPriority() const {
    return this->alertPriority;
}

bool Rule::isTriggeredBy(const Sensor& sensor) const {
    if (sensor.getType() != this->targetSensorType) {
        return false;
    }

    if (this->condition == ">") {
        return sensor.getValue() > this->threshold;
    } else if (this->condition == "<") {
        return sensor.getValue() < this->threshold;
    } else if (this->condition == "==") {
        return sensor.getValue() == this->threshold;
    }

    return false;
}

