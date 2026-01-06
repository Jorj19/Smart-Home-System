//
// Created by bunea on 10.11.2025.
//

#include "ConfigManager.h"
#include "HomeExceptions.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <memory>
#include <algorithm>
#include <cctype>

static std::string toLower(std::string str) {
    std::ranges::transform(str.begin(), str.end(), str.begin(),
        [](unsigned char c) {return std::tolower(c); });

    return str;
}

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
                if (Room* room = loadedSystem.findRoomByName(parts[1]); room){
                    const std::string& sensorType = parts[2];
                    double sensorValue = std::stod(parts[3]);
                    std::string sensorUnits = parts[4];

                    std::string sensorTypeLower = toLower(sensorType);

                    std::shared_ptr<Sensor> newSensor = nullptr;

                    if (sensorTypeLower == "temperatura") {
                        newSensor = std::make_shared<temperatureSensor>(sensorValue);
                    }
                    else if (sensorTypeLower == "umiditate") {
                        newSensor = std::make_shared<humiditySensor>(sensorValue);
                    }
                    else if (sensorTypeLower == "fum") {
                        newSensor = std::make_shared<smokeSensor>(sensorValue);
                    }
                    else if (sensorTypeLower == "lumina") {
                        newSensor = std::make_shared<lightSensor>(sensorValue);
                    }
                    else if (sensorTypeLower == "pm2.5" || sensorTypeLower == "pm25") {
                        newSensor = std::make_shared<particleSensor>(sensorValue);
                    }
                    else if (sensorTypeLower == "co") {
                        newSensor = std::make_shared<toxicGasSensor>("CO", sensorValue, sensorUnits);
                    }
                    else if (sensorTypeLower == "co2") {
                        newSensor = std::make_shared<toxicGasSensor>("CO2", sensorValue, sensorUnits);
                    }
                    else if (sensorTypeLower == "tvoc") {
                        newSensor = std::make_shared<toxicGasSensor>("TVOC", sensorValue, sensorUnits);
                    }
                    else {
                        std::cout << "Warning: Unknown sensor type '" << sensorType << "'\n";
                    }

                    if (newSensor) {
                     room->addSensor(newSensor);
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
