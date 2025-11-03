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
    explicit Sensor(std::string type, double value, std::string unit);

    Sensor();

    [[nodiscard]] std::string getType() const;
    [[nodiscard]] double getValue() const;
    [[nodiscard]] std::string getUnit() const;

    void updateValue(double newValue);

    friend std::ostream& operator<<(std::ostream& os, const Sensor& s);
};

#endif // SENSOR_H