#ifndef SENSOR_H
#define SENSOR_H

#include <iostream>
#include <string>

class Sensor {
private:
    std::string sensorType;
    double currentValue;
    std::string measurementUnit;

    static int totalSensors;

    virtual void printDetails(std::ostream& os) const;

public:
    explicit Sensor(std::string type, double value, std::string unit);

    virtual ~Sensor();

    [[nodiscard]] virtual double calculateHazardLevel() const = 0;

    [[nodiscard]] virtual Sensor* clone() const = 0;

    void display(std::ostream& os) const;

    [[nodiscard]] const std::string& getType() const;
    [[nodiscard]] double getValue() const;
    [[nodiscard]] const std::string& getUnit() const;

    static int getTotalSensors();

    void updateValue(double newValue);

    friend std::ostream& operator<<(std::ostream& os, const Sensor& s);
};

//temperatura
class temperatureSensor : public Sensor {
public:
    explicit temperatureSensor(double value);
    [[nodiscard]] double calculateHazardLevel() const override;
    [[nodiscard]] Sensor* clone() const override;
};

//umiditate
class humiditySensor : public Sensor {
public:
    explicit humiditySensor(double value);
    [[nodiscard]] double calculateHazardLevel() const override;
    [[nodiscard]] Sensor* clone() const override;
};

//lumina
class lightSensor : public Sensor {
public:
    explicit lightSensor(double value);
    [[nodiscard]] double calculateHazardLevel() const override;
    [[nodiscard]] Sensor* clone() const override;
};

//fum
class smokeSensor : public Sensor {
public:
    explicit smokeSensor(double value);
    [[nodiscard]] double calculateHazardLevel() const override;
    [[nodiscard]] Sensor* clone() const override;
};

//gaze toxice (co, co2, tvoc)
class toxicGasSensor : public Sensor {
public:
    toxicGasSensor(std::string gasType ,double value, std::string unit);
    [[nodiscard]] double calculateHazardLevel() const override;
    [[nodiscard]] Sensor* clone() const override;
};

//particule (PM 2.5)
class particleSensor : public Sensor {
public:
    explicit particleSensor(double value);
    [[nodiscard]] double calculateHazardLevel() const override;
    [[nodiscard]] Sensor* clone() const override;
};


#endif // SENSOR_H