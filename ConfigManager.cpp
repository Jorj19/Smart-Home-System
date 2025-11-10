//
// Created by bunea on 10.11.2025.
//

#include "ConfigManager.h"
#include <fstream>
#include <sstream>
#include <iostream>

HomeSystem ConfigManager::loadSystemFromFile(const std::string& filename) {
    std::cout << "Loading system from " << filename << std::endl;

    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cout << "Error: Could not open configuration file " << filename << ".\n";
        return HomeSystem("Empty System (load Failed)");
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

        std::string type = parts[0];

        try {
            if (type == "ROOM" && parts.size() == 2) {
                loadedSystem.addRoom(Room(parts[1]));
                std::cout << "Loaded Room: " << parts[1] << "\n";

            } else if (type == "SENSOR" && parts.size() == 5) {
                if (Room* room = loadedSystem.findRoomByName(parts[1]); room) {
                    room->addSensor(Sensor(parts[2], std::stod(parts[3]), parts[4]));
                } else {
                    std::cout << "Warning (L" << lineCount << "): Room '" << parts[1] << "' not found for sensor.\n";
                }

            } else {
                std::cout << "Warning (L" << lineCount << "): Unknown or malformed line: " << line << "\n";
            }
        } catch (const std::invalid_argument& e) {
            std::cout << "Error (L" << lineCount << "): Bad data format in line: " << line << "'. Reason: " << e.what() << "\n";
        }
    }

    file.close();
    std::cout << "--- System loaded successfully. ---\n";
    return loadedSystem;
}



bool ConfigManager::saveSystemToFile(const std::string& filename, const HomeSystem& system) {
    std::ofstream outFile(filename);
    if (!outFile.is_open()) {
        std::cout << "Error: Could not open configuration file " << filename << ".\n";
        return false;
    }

    outFile << "# --- System Rooms ---\n";
    for (const Room& room : system.getRoomList()) {
        outFile << "ROOM," << room.getName() << "\n";
    }

    outFile << "\n# --- System Sensors --- \n";
    for (const Room& room : system.getRoomList()) {
        for (const Sensor& sensor : room.getSensorList()) {
            outFile << "SENSOR," << room.getName() << ","
                    << sensor.getType() << ","
                    << sensor.getValue() << ","
                    << sensor.getUnit() << "\n";
        }
    }

    outFile.close();
    std::cout << "System layout successfully saved to " << filename << "\n";
    return true;
}
