#include "HomeSystem.h"
#include <utility>

HomeSystem::HomeSystem(std::string name) : systemName(std::move(name)) {

}

void HomeSystem::addRoom(const Room& r) {
    if (this->findRoomByName(r.getName()) != nullptr) {
        std::cout << "Error: A room with the name '" << r.getName() << "' already exists. Not adding.\n";
        return;
    }

    this->roomList.push_back(r);
}

Room* HomeSystem::findRoomByName(const std::string& name) {
    for (Room& room : this->roomList) {
        if (room.getName() == name) {
            return &room;
        }
    }
    return nullptr;
}

// cppcheck-suppress unusedFunction
void HomeSystem::findRoomsWithCriticalSensors(const std::string& sensorType, double threshold) const {
    std::cout << "\n Searching for Critical Sensors (" << sensorType << " > " << threshold << ") in system " << this->systemName << "\n";
    for (const Room& room : this->roomList) {
        room.displaySensorsOverThreshold(threshold, sensorType);
    }
}

const std::vector<Room>& HomeSystem::getRoomList() const {
    return this->roomList;
}

// cppcheck-suppress unusedFunction
std::string HomeSystem::getSystemName() const {
    return this->systemName;
}

std::ostream& operator<<(std::ostream& os, const HomeSystem& hs) {
    os << "#\n";
    os << "Home System: " << hs.systemName << " (Total " << hs.roomList.size() << " rooms)\n";
    os << "#\n";

    if (hs.roomList.empty()) {
        os << " (System has no rooms)\n";
    } else {
        for (const Room& room : hs.roomList) {
            os << room << "\n";
        }
    }
    return os;
}