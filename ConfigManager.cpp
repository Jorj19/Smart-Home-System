//
// Created by bunea on 10.11.2025.
//

#include "ConfigManager.h"
#include "HomeExceptions.h"
#include "Extensions.h"
#include <fstream>
#include <sstream>
#include <memory>
#include <algorithm>
#include <cctype>


// cppcheck-suppress unusedFunction
HomeSystem ConfigManager::loadSystemFromFile(const std::string& filename) {
    std::cout << "Loading system from " << filename << std::endl;

    std::ifstream file(filename);
    if (!file.is_open()) {
       throw FileConfigException(filename);
    }

    HomeSystem loadedSystem("Loaded System");
    std::string line;
    int lineCount = 0;

    while (std::getline(file, line)) {
        lineCount++;
        if (line.empty() || line[0] == '#') {
            continue;
        }

        std::stringstream ss(line);
        std::string segment;
        std::vector<std::string> parts;
        while (std::getline(ss, segment, ',')) {
            parts.push_back(segment);
        }

        if (parts.empty()) {
            continue;
        }


        try {
            if (const std::string& type = parts[0]; type == "ROOM" && parts.size() == 2) {
                loadedSystem.addRoom(Room(parts[1]));
                std::cout << "Loaded Room: " << parts[1] << "\n";

            } else if (type == "SENSOR" && parts.size() == 5) {
                if (Room* room = loadedSystem.findRoomByName(parts[1])) {
                    std::string sensorTypeStr = parts[2];
                    double sensorValue = std::stod(parts[3]);

                    if (auto newSensor = sensorFactory::createSensorFromString(sensorTypeStr, sensorValue)) {
                        room->addSensor(newSensor);
                    } else {
                        std::cout << "Warning (L" << lineCount << "): Unknown sensor type '" << sensorTypeStr << "'\n";
                    }
                } else {
                    throw RoomNotFoundException(parts[1]);
                }

            } else {
                std::cout << "Warning (L" << lineCount << "): Unknown or malformed line: " << line << "\n";
            }
        } catch (const std::invalid_argument& e) {
            throw InvalidDataSensorException("Line " + std::to_string(lineCount) + ": Numeric conversion failed." + e.what());
        }
    }

    file.close();
    std::cout << "--- System loaded successfully. ---\n";
    return loadedSystem;
}


// cppcheck-suppress unusedFunction
bool ConfigManager::saveSystemToFile(const std::string& filename, const HomeSystem& system) {
    std::ofstream outFile(filename);
    if (!outFile.is_open()) {
        throw FileConfigException(filename);
    }

    outFile << "# --- System Rooms ---\n";
    for (const Room& room : system.getRoomList()) {
        outFile << "ROOM," << room.getName() << "\n";
    }

    outFile << "\n# --- System Sensors --- \n";
    for (const Room& room : system.getRoomList()) {
        for (const auto& sensor : room.getSensorList()) {
            outFile << "SENSOR," << room.getName() << ","
                    << sensor->getType() << ","
                    << sensor->getValue() << ","
                    << sensor->getUnit() << "\n";
        }
    }

    outFile.close();
    std::cout << "System layout successfully saved to " << filename << "\n";
    return true;
}
