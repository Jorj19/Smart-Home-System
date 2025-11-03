#include "Sensor.h"
#include <utility>

Sensor::Sensor(std::string type, double value, std::string unit) : sensorType(std::move(type)), currentValue(value), measurementUnit(std::move(unit)) {
}

Sensor::Sensor() : sensorType("Unknown"), currentValue(0.0), measurementUnit("N/A") {

}

std::string Sensor::getType() const {
    return this->sensorType;
}

double Sensor::getValue() const {
    return this->currentValue;
}

std::string Sensor::getUnit() const {
    return this->measurementUnit;
}

// cppcheck-suppress unusedFunction
void Sensor::updateValue(double newValue) {
    this->currentValue = newValue;
}


std::ostream& operator<<(std::ostream& os, const Sensor& s) {
    os << "Sensor [Type: " << s.sensorType << ", Value: " << s.currentValue << " " << s.measurementUnit << "]";
    return os;
}