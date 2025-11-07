
#include "Room.h"
#include <iostream>
#include <utility>
#include <cstddef>
Room::Room() : roomName("N/A") {

}

Room::Room(std::string name) : roomName(std::move(name)) {

}

void Room::addSensor(const Sensor& s) {
    this->sensorList.push_back(s);
}

double Room::calculateSensorAverage(const std::string& sensorType) const {
    double sum = 0.0;
    std::size_t count = 0u;

    for (const Sensor& sensor : this->sensorList) {
        if (sensor.getType() == sensorType) {
            sum += sensor.getValue();
            ++count;
        }
    }

    if (count == 0u) {
        return 0.0;
    }
    return sum / static_cast<double>(count);
}

void Room::displaySensorsOverThreshold(double threshold, const std::string& sensorType) const {
    std::cout << "Sensors [Type: " << sensorType << "] over threshold [" << threshold << "] in " << this->roomName << ":\n";
    bool found = false;

    for (const Sensor& sensor : this->sensorList) {
        if (sensor.getType() == sensorType && sensor.getValue() > threshold) {
            std::cout << " -> " << sensor << "\n";
            found = true;
        }
    }
    if (!found) {
        std::cout << " (none)\n";
    }
}


std::string Room::getName() const {
    return this->roomName;
}

const std::vector<Sensor>& Room::getSensorList() const {
    return this->sensorList;
}

std::ostream& operator<<(std::ostream& os, const Room& r) {
    os << " - Room: " << r.roomName << " (" << r.sensorList.size() << " sensors)\n";

    if (r.sensorList.empty()) {
        os << " (Room is empty)\n";
    } else {
        for (const Sensor& sensor : r.sensorList) {
            os  << " -> " << sensor << "\n";
        }
    }
    return os;
}