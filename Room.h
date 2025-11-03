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
    int sensorCount;

public:
    Room(const std::string& name);

    Room();

    void addSensor(const Sensor& s);

    double calculateSensorAverage(const std::string& sensorType) const;

    void displaySensorsOverThreshold(double threshold, const std::string& sensorType) const;

    std::string getName() const;

    const std::vector<Sensor>& getSensorList() const;

    int getSensorCount() const;

    friend std::ostream& operator<<(std::ostream& os, const Room& r);
};

#endif // ROOM_H