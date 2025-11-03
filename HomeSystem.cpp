#include "HomeSystem.h"

HomeSystem::HomeSystem(const std::string& name) : systemName(name), roomCount(0) {

}

void HomeSystem::addRoom(const Room& r) {
    this->roomList.push_back(r);
    this->roomCount++;
}

void HomeSystem::findRoomsWithCriticalSensors(const std::string& sensorType, double threshold) const {
    std::cout << "\n Searching for Critical Sensors (" << sensorType << " > " << threshold << ") in system " << this->systemName << "\n";
    for (int i = 0; i < this->roomCount; i++) {
        this->roomList[i].displaySensorsOverThreshold(threshold, sensorType);
    }
}

const std::vector<Room>& HomeSystem::getRoomList() const {
    return this->roomList;
}

int HomeSystem::getRoomCount() const {
    return this->roomCount;
}

std::string HomeSystem::getSystemName() const {
    return this->systemName;
}

std::ostream& operator<<(std::ostream& os, const HomeSystem& hs) {
    os << "#\n";
    os << "Home System: " << hs.systemName << " (Total " << hs.roomCount << " rooms)\n";
    os << "#\n";

    if (hs.roomCount == 0) {
        os << " (System has no rooms)\n";
    } else {
        for (int i = 0; i < hs.roomCount; i++) {
            os << hs.roomList[i] << "\n";
        }
    }
    return os;
}