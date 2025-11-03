#include "Room.h"
#include <iostream>

Room::Room() : roomName("N/A"), sensorCount(0) {

}

Room::Room(const std::string& name) : roomName(name), sensorCount(0) {

}

void Room::addSensor(const Sensor& s) {
    this->sensorList.push_back(s);
    this->sensorCount++;
}

double Room::calculateSensorAverage(const std::string& sensorType) const {
    double sum = 0.0;
    int count = 0;

    for (int i = 0; i < this->sensorCount; i++) {
        if (this->sensorList[i].getType() == sensorType) {
            sum += this->sensorList[i].getValue();
            count++;
        }
    }

    if (count == 0) {
        return 0.0;
    }
    return sum / count;
}

void Room::displaySensorsOverThreshold(double threshold, const std::string& sensorType) const {
    std::cout << "Sensors [Type: " << sensorType << "] over threshold [" << threshold << "] in " << this->roomName << ":\n";
    bool found = false;

    for (int i = 0; i < this->sensorCount; i++) {
        if (this->sensorList[i].getType() == sensorType && this->sensorList[i].getValue() > threshold) {
            std::cout << " -> " << this->sensorList[i] << "\n";
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

int Room::getSensorCount() const {
    return this->sensorCount;
}

std::ostream& operator<<(std::ostream& os, const Room& r) {
    os << " - Room: " << r.roomName << " (" << r.sensorCount << " sensors)\n";

    if (r.sensorCount == 0) {
        os << " (Room is empty)\n";
    } else {
        for (int i = 0; i < r.sensorCount; i++) {
            os << "  " << i + 1 << ". " << r.sensorList[i] << "\n";
        }
    }
    return os;
}