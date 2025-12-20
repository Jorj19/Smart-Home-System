#ifndef ROOM_H
#define ROOM_H

#include "Sensor.h"
#include <iostream>
#include <string>
#include <vector>
#include <memory>

class Room {
private:
    std::string roomName;
    std::vector<std::shared_ptr<Sensor>> sensorList;

public:
    explicit Room(std::string name);

    Room();

    ~Room() = default;

    Room(const Room& other);

    Room& operator=(Room other);

    friend void swap(Room& first, Room& second) noexcept;

    void addSensor(const std::shared_ptr<Sensor>& s);

    [[nodiscard]] double calculateSensorAverage(const std::string& sensorType) const;

    void displaySensorsOverThreshold(double threshold, const std::string& sensorType) const;

    [[nodiscard]] std::string getName() const;

    [[nodiscard]] const std::vector<std::shared_ptr<Sensor>>& getSensorList() const;

    friend std::ostream& operator<<(std::ostream& os, const Room& r);
};

#endif // ROOM_H