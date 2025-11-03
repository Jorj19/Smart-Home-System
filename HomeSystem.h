#ifndef HOMESYSTEM_H
#define HOMESYSTEM_H

#include "Room.h"
#include <string>
#include <iostream>
#include <vector>

class HomeSystem {
private:
    std::string systemName;
    std::vector<Room> roomList;

public:
    explicit HomeSystem(std::string name);

    void addRoom(const Room& r);

    void findRoomsWithCriticalSensors(const std::string& sensorType, double threshold) const;

    [[nodiscard]] const std::vector<Room>& getRoomList() const;
    [[nodiscard]] std::string getSystemName() const;

    friend std::ostream& operator<<(std::ostream& os, const HomeSystem& hs);
};

#endif // HOMESYSTEM_H