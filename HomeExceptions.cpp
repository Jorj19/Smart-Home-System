//
// Created by bunea on 15.12.2025.
//

#include "HomeExceptions.h"

#include "Room.h"

HomeExceptions::HomeExceptions(std::string msg) : message(std::move(msg)){}

const char* HomeExceptions::what() const noexcept {
    return this->message.c_str();
}

FileConfigException::FileConfigException(const std::string& filename)
    : HomeExceptions("File Error: Could not open or read configuration file [" + filename + "]"){}

InvalidDataSensorException::InvalidDataSensorException(const std::string& details)
    : HomeExceptions("Sensor data error: " + details){}

RuleEngineException::RuleEngineException(const std::string& ruleDetails)
    : HomeExceptions("Rule engine error: Malformed rule definition - " + ruleDetails){}

DuplicateEntityException::DuplicateEntityException(const std::string& entityName)
    : HomeExceptions("Duplicate entity error: Entity " + entityName + " already exists in the system " ){}

RoomNotFoundException::RoomNotFoundException(const std::string& roomName)
    : HomeExceptions("Room '" + roomName + "' not found in the system "){}

SensorOutOfRangeException::SensorOutOfRangeException(const std::string& sensorType, double value)
    : InvalidDataSensorException("Value: " + std::to_string(value) + " is out of physical range for " + sensorType) {}
