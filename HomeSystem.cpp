#include "HomeSystem.h"
#include "HomeExceptions.h"
#include <utility>
#include <algorithm>

HomeSystem::HomeSystem(std::string name) : systemName(std::move(name)) {

}

void HomeSystem::addRoom(const Room& r) {
    if (this->findRoomByName(r.getName()) != nullptr) {
        throw DuplicateEntityException("Room '" + r.getName() + "'");
    }

    this->roomList.push_back(r);
}

// cppcheck-suppress unusedFunction
void HomeSystem::removeRoom(const Room& r) {
    const std::string& nameToDelete = r.getName();

    auto [first, last] = std::ranges::remove_if(roomList,
        [&nameToDelete](const Room& currentRoom) {
            return currentRoom.getName() == nameToDelete;
        });

    if (first != roomList.end()) {
        roomList.erase(first, last);
    } else {
        throw RoomNotFoundException(nameToDelete);
    }
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

