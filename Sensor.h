#ifndef SENSOR_H
#define SENSOR_H

#include <iostream>
#include <string>

class Sensor {
private:
    std::string sensorType;
    double currentValue;
    std::string measurementUnit;

public:
    Sensor(const std::string& type, double value, const std::string& unit);

    Sensor();

    std::string getType() const;
    double getValue() const;
    std::string getUnit() const;

    void updateValue(double newValue);

    friend std::ostream& operator<<(std::ostream& os, const Sensor& s);
};

#endif // SENSOR_H