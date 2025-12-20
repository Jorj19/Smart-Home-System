
#include "Room.h"
#include <iostream>
#include <utility>
#include <cstddef>
Room::Room() : roomName("N/A") {}

Room::Room(std::string name) : roomName(std::move(name)) {}

Room::Room(const Room& other) : roomName(other.roomName) {
    for (const auto& s : other.sensorList) {
        if (s) {
            this->sensorList.push_back(std::shared_ptr<Sensor>(s->clone()));
        }
    }
}

Room& Room::operator=(Room other) {
    swap(*this, other);
    return *this;
}

void swap(Room& a, Room& b) noexcept {
    using std::swap;
    swap(a.roomName, b.roomName);
    swap(a.sensorList, b.sensorList);
}

void Room::addSensor(const std::shared_ptr<Sensor>& s) {
    if(s) {
        this->sensorList.push_back(s);
    }
}

// cppcheck-suppress unusedFunction
double Room::calculateSensorAverage(const std::string& sensorType) const {
    double sum = 0.0;
    std::size_t count = 0u;

    for (const auto& s : this->sensorList) {
        if (s->getType() == sensorType) {
            sum += s->getValue();
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

    for (const auto& s : this->sensorList) {
        if (s->getType() == sensorType && s->getValue() > threshold) {
            std::cout << " -> " << s << "\n";
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

const std::vector<std::shared_ptr<Sensor>>& Room::getSensorList() const {
    return this->sensorList;
}

std::ostream& operator<<(std::ostream& os, const Room& r) {
    os << " - Room: " << r.roomName << " (" << r.sensorList.size() << " sensors)\n";

    if (r.sensorList.empty()) {
        os << " (Room is empty)\n";
    } else {
        for (const auto& s : r.sensorList) {
            os  << " -> " << *s << "\n";
        }
    }
    return os;
}