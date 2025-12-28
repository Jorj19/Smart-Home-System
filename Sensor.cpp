#include "Sensor.h"
#include <utility>

#include "HomeExceptions.h"

int Sensor::totalSensors = 0;

Sensor::Sensor(std::string type, double value, std::string unit) : sensorType(std::move(type)), currentValue(value), measurementUnit(std::move(unit)) {

    if (this->sensorType.empty()) {
        throw InvalidDataSensorException("Sensor type cannot be empty");
    }

    totalSensors++;
}

Sensor::~Sensor() {
    totalSensors--;
}

void Sensor::display(std::ostream& os) const {
    this->printDetails(os);
}

void Sensor::printDetails(std::ostream& os) const {
    os << "[" << sensorType << "] Val: " << currentValue << " " << measurementUnit;

    double hazard = this->calculateHazardLevel();
    if (hazard >= 100.0) os << "(!!! CRITICAL !!!)";
    else if (hazard >= 40.0) os << " (! WARNING !)";
    else if (hazard >= 25.0) os << " (! INFO !)";
}

const std::string& Sensor::getType() const {
    return sensorType;
}

// cppcheck-suppress unusedFunction
double Sensor::getValue() const {
    return currentValue;
}

// cppcheck-suppress unusedFunction
const std::string& Sensor::getUnit() const {
    return measurementUnit;
}

int Sensor::getTotalSensors() {
    return totalSensors;
}

// cppcheck-suppress unusedFunction
void Sensor::updateValue(double newValue) {
    this->currentValue = newValue;
}

std::ostream& operator<<(std::ostream& os, const Sensor& s) {
    s.display(os);
    return os;
}


//temperatura
temperatureSensor::temperatureSensor(double value) : Sensor("Temperatura", value, "C") {}

double temperatureSensor::calculateHazardLevel() const {
    double val = this->getValue();

    //critical
    if (val > 60.0) return 100.0;
    if (val < 0.0) return 100.0;

    //warning
    if (val > 35.0) return 60;
    if (val < 10.0) return 60;

    //info
    if (val > 26.0) return 30.0;
    if (val < 18.0) return 30.0;

    return 0.0;
}

Sensor* temperatureSensor::clone() const {
    return new temperatureSensor(*this);
}

//umiditate
humiditySensor::humiditySensor(double value) : Sensor("Umiditate", value, "%") {}

double humiditySensor::calculateHazardLevel() const {
    double val = this->getValue();
    //warning
    if (val >= 80.0) return 60.0;

    //info
    if (val > 60.0) return 30.0;
    if (val < 30.0) return 30.0;

    return 0.0;
}

Sensor* humiditySensor::clone() const {
    return new humiditySensor(*this);
}

//lumina
lightSensor::lightSensor(double value) : Sensor("Lumina", value, "lux") {}

double lightSensor::calculateHazardLevel() const {
    double val = this->getValue();

    //warning
    if (val >= 5000.0) return 50.0;

    //info
    if (val < 100.0) return 25.0;
    if (val > 2000.0) return 25.0;

    return 0.0;
}

Sensor* lightSensor::clone() const {
    return new lightSensor(*this);
}

//fum
smokeSensor::smokeSensor(double value) : Sensor("fum", value, "bool") {}

double smokeSensor::calculateHazardLevel() const {

    if (this->getValue() > 0.5) return 100.0;
    return 0.0;
}

Sensor* smokeSensor::clone() const {
    return new smokeSensor(*this);
}


//gaze toxice
toxicGasSensor::toxicGasSensor(std::string gasType, double value, std::string unit)
    : Sensor(std::move(gasType), value, std::move(unit)) {}

double toxicGasSensor::calculateHazardLevel() const {
    double val = this->getValue();

    //CO
    if (this->getType() == "CO") {
        if (val > 50.0) return 100.0; //Critic
        if (val > 30.0) return 80.0; //Warning
        if (val > 9.0) return 40.0; //info
    }

    if (this->getType() == "CO2") {
        if (val > 5000.0) return 80.0; //Warning
        if (val > 2000.0) return 50.0; //Warning
        if (val > 1000.0) return 25.0; //Info
    }

    if (this->getType() == "TVOC") {
        if (val > 2000.0) return 60.0; // Warning
        if (val > 500.0)  return 30.0; // Info
    }

    return 0.0;
}

Sensor* toxicGasSensor::clone() const {
    return new toxicGasSensor(*this);
}

//particule
particleSensor::particleSensor(double value) : Sensor("PM2.5", value, "ug/m3") {}

double particleSensor::calculateHazardLevel() const {
    double val = this->getValue();

    if (val > 150.0) return 90.0; // Warning
    if (val > 75.0)  return 50.0; // Warning
    if (val > 35.0)  return 25.0; // Info

    return 0.0;
}

Sensor* particleSensor::clone() const {
    return new particleSensor(*this);
}

//sunet
soundSensor::soundSensor(double value) : Sensor("Sunet", value, "dB") {}

double soundSensor::calculateHazardLevel() const {
    double val = this->getValue();
    if (val >= 120.0) return 100.0;
    if (val >= 85.0) return 60.0;
    if (val >= 70.0) return 30.0;

    return 0.0;
}

Sensor* soundSensor::clone() const {
    return new soundSensor(*this);
}