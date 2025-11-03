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
    int roomCount;

public:
    HomeSystem(const std::string& name);

    void addRoom(const Room& r);

    void findRoomsWithCriticalSensors(const std::string& sensorType, double threshold) const;

    const std::vector<Room>& getRoomList() const;
    int getRoomCount() const;
    std::string getSystemName() const;

    friend std::ostream& operator<<(std::ostream& os, const HomeSystem& hs);
};

#endif // HOMESYSTEM_H