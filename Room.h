#ifndef ROOM_H
#define ROOM_H

#include "Sensor.h"
#include <iostream>
#include <string>
#include <vector>

class Room {
private:
    std::string roomName;
    std::vector<Sensor> sensorList;

public:
    explicit Room(std::string name);

    Room();

    void addSensor(const Sensor& s);

    [[nodiscard]] double calculateSensorAverage(const std::string& sensorType) const;

    void displaySensorsOverThreshold(double threshold, const std::string& sensorType) const;

    [[nodiscard]] std::string getName() const;

    [[nodiscard]] const std::vector<Sensor>& getSensorList() const;

    friend std::ostream& operator<<(std::ostream& os, const Room& r);
};

#endif // ROOM_H