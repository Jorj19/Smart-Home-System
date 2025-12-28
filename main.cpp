#include <iostream>
#include <string>
#include <vector>
#include <limits>
#include <memory>
#include <algorithm>
#include <random>

#include "Sensor.h"
#include "Room.h"
#include "HomeSystem.h"
#include "Alert.h"
#include "AnalysisEngine.h"
#include "ConfigManager.h"
#include "HomeExceptions.h"
#include <httplib.h>

void runDemoSystem(const std::string& ip);
void runInteractiveSystem(const std::string& ip);
void clearInputBuffer();

std::string toLower(std::string str) {
    std::ranges::transform(str.begin(), str.end(), str.begin(),
                   [](unsigned char c){ return std::tolower(c); });
    return str;
}


double extractNestedValue(const std::string& json, const std::string& sensorName) {
    std::string keySearch = "\"" + sensorName + "\"";
    size_t keyPos = json.find(keySearch);
    if (keyPos == std::string::npos) return -999.0;

    size_t valueLabelPos = json.find("\"valoare\"", keyPos);
    if (valueLabelPos == std::string::npos || valueLabelPos - keyPos > 100) return -999.0;

    size_t colonPos = json.find(':', valueLabelPos);
    size_t endPos = json.find_first_of(",}", colonPos);

    std::string valStr = json.substr(colonPos + 1, endPos - colonPos - 1);
    try {
        return std::stod(valStr);
    } catch (...) {
        return -999.0;
    }
}

double applySimulationOffset(double baseValue, const std::string& type) {
    if (type == "Fum") {
        return baseValue;
    }

    double offset = 0.0;

    static std::random_device rd;
    static std::mt19937 gen(rd());

    std::uniform_real_distribution<double> dis(-1.0, 1.0);

    double randomFactor = dis(gen);

    if (type == "Temperatura") {
        // +/- 3 grade
        offset = randomFactor * 3.0;
    }
    else if (type == "Umiditate") {
        // +/- 10%
        offset = randomFactor * 10.0;
    }
    else if (type == "Lumina") {
        // +/- 150 lux
        offset = randomFactor * 150.0;
    }
    else if (type == "CO2" || type == "TVOC") {
        // +/- 50 unitati
        offset = randomFactor * 50.0;
    }
    else if (type == "Sunet") {
        // +/- 10 db
        offset = randomFactor * 12.0;
    }
    else {
        offset = randomFactor * 1.0;
    }

    double finalValue = offset + baseValue;

    finalValue = std::round(finalValue * 10.0) / 10.0;

    if (finalValue < 0.0) return 0.0;
    if (type == "Umiditate" && finalValue > 100.0) return 100.0;

    return finalValue;
}


double getLiveValueFromPi(const std::string& sensorType, const std::string& ip) {
    std::string typeNormalizat = toLower(sensorType);

    std::string jsonKey;

    if (typeNormalizat == "temperatura")
        jsonKey = "temperature";
    else if (typeNormalizat == "umiditate")
        jsonKey = "humidity";
    else if (typeNormalizat == "lumina")
        jsonKey = "lux";
    else if (typeNormalizat == "co2")
        jsonKey = "mq135_co2";
    else if (typeNormalizat == "pm2.5")
        jsonKey = "pm2_5";
    else if (typeNormalizat == "fum")
        jsonKey = "fum";
    else if (typeNormalizat == "tvoc")
        jsonKey = "tvoc";
    else if (typeNormalizat == "co")
        jsonKey = "co";
    else if (typeNormalizat == "sunet")
        jsonKey = "sunet";
    else {
        std::cout << "[WARNING] Tip senzor necunoscut: " << sensorType << "\n";
        return 0.0;
    }

    //conectare la server
    httplib::Client cli(ip, 5000);
    cli.set_connection_timeout(0, 500000);

    auto res = cli.Get("/sensors");

    if (res && res->status == 200) {
        double val = extractNestedValue(res->body, jsonKey);
        if (val != -999.0) {
            std::cout << "[SERVER] " << sensorType << " -> Valoare: " << val << "\n";
            return val;
        }
    }
    else {
        std::cerr << "[!] Connection failed to " << ip << ". Is the server script running?\n";
    }

    return 0.0;
}

int main() {
    try {
        std::string ip;

        std::cout << "========= THE HOMEY - CONFIGURATION =========\n";
        std::cout << "Where should I get sensor data from?\n";
        std::cout << "1. FAKE SERVER (Simulation on this PC)\n";
        std::cout << "2. REAL SERVER (Raspberry Pi)\n";
        std::cout << "Select option: ";

        int serverChoice = 1;
        if (!(std::cin >> serverChoice)) {
            return 0;
        }

        if (serverChoice == 1) {
            ip = "127.0.0.1"; // localhost

            std::cout << "\n[INFO] Selected FAKE SERVER.\n";
            std::cout << "PLEASE MAKE SURE YOU RUN THE PYTHON SCRIPT:\n";
            std::cout << "Run this command in a separate terminal:\n";
            std::cout << "  python ServerModules/fake_server.py\n";

        } else if (serverChoice == 2) {
            std::cout << "\nEnter Raspberry Pi IP Address (e.g., 192.168.100.112): ";
            clearInputBuffer();
            std::getline(std::cin, ip);
            if(ip.empty()) ip = "192.168.100.112";

            std::cout << "[INFO] Selected REAL SERVER at " << ip << "\n";
        } else {
            std::cout << "Invalid selection. Defaulting to Localhost Simulation.\n";
            ip = "127.0.0.1";
        }

        std::cout << "\n===== THE HOMEY - Main Menu =====\n";
        std::cout << "1. Create New System \n";
        std::cout << "2. Run Demo System\n";
        std::cout << "3. Load System from File \n";
        std::cout << "0. Exit\n";
        std::cout << "Select option: ";

        int option = 0;
        if (!(std::cin >> option)) {
            return 0;
        }

        switch (option) {
        case 1:
            runInteractiveSystem(ip);
            break;
        case 2:
            runDemoSystem(ip);
            break;
        case 3: {
            std::string filename;
            std::cout << "Enter configuration file to load (e.g., config.txt or my_home.txt): ";
            clearInputBuffer();
            std::getline(std::cin, filename);

            if(filename.empty()) filename = "config.txt";

            HomeSystem loadedSystem = ConfigManager::loadSystemFromFile(filename);

            AnalysisEngine loadedEngine(loadedSystem, "rules.txt");


            std::cout << "\n--- Running analysis on loaded system... ---\n";
            std::cout << loadedSystem;
            std::cout << loadedEngine.generateStatusReport();
            std::cout << "System Health Score: " << loadedEngine.calculateHealthScore() << "/100\n";
            std::vector<Alert> alerts = loadedEngine.generateAlerts();
            if (alerts.empty()) {
                std::cout << "No alerts detected. System is stable.\n";
            } else {
                std::cout << "\n!!! ALERTS DETECTED !!!\n";
                for (const Alert& a : alerts) {
                    std::cout << "  -> " << a << "\n";
                }
            }
            break;
        }
        case 0:
            std::cout << "Goodbye!\n";
            return 0;
        default:
            std::cout << "Invalid option!\n";
            break;
        }
    }
    catch (const HomeExceptions& e) {
        std::cerr << "\n[!] HOME SYSTEM ERROR: " << e.what() << "\n";
        std::cerr << "The application will now close safely.\n";
    }
    catch (const std::exception& e) {
        std::cerr << "\n[!] STANDARD ERROR: " << e.what() << "\n";
    }
}

void runInteractiveSystem(const std::string& ip) {
    std::string systemName;
    std::cout << "Enter new system name: ";
    clearInputBuffer();
    std::getline(std::cin, systemName);

    HomeSystem userSystem(systemName);
    AnalysisEngine userEngine(userSystem, "rules.txt");

    while(true) {
        std::cout << "\n--- System: " << systemName << " ---\n";
        std::cout << "1. Add Room\n";
        std::cout << "2. Add Sensor (to a Room)\n";
        std::cout << "3. Run Analysis\n";
        std::cout << "4. Display Full System\n";
        std::cout << "5. Save system to file\n";
        std::cout << "0. Back to Main Menu\n";
        std::cout << "Select option: ";

        int option = 0;
        if (!(std::cin >> option)) {
            break;
        }

        if (std::cin.eof()) {
            break;
        }

        if (!std::cin) {
            std::cout << "Invalid input. Please enter a number.\n";
            clearInputBuffer();
            continue;
        }

        if (option == 0) break;

        try {
            switch(option) {
            case 1: {
                std::string roomName;
                std::cout << "Enter room name: ";
                clearInputBuffer();
                std::getline(std::cin, roomName);
                userSystem.addRoom(Room(roomName));
                std::cout << "Room '" << roomName << "' added.\n";
                break;
            }
            case 2: {
                std::string roomName;
                std::cout << "Enter room name to add sensor to: ";
                clearInputBuffer();
                std::getline(std::cin, roomName);

                Room* room = userSystem.findRoomByName(roomName);
                if (room == nullptr) {
                    throw RoomNotFoundException(roomName);
                }

                std::cout << "Select sensor type:\n";
                std::cout << "1. Temperatura\n2. Umiditate\n3. Lumina\n4. CO2\n5. PM2.5\n6. Fum\n7. TVOC\n8. CO\n9. ADD ALL SENSORS (Full Package)\n";
                std::cout << "Choice: ";
                int sOpt;
                std::cin >> sOpt;

                std::vector<int> sensorsToAdd;

                if (sOpt == 9) {
                    sensorsToAdd = {1, 2, 3, 4, 5, 6, 7, 8};
                    std::cout << "[INFO] Selected ALL sensors package.\n";
                } else if (sOpt >= 1 && sOpt <= 8) {
                    sensorsToAdd.push_back(sOpt);
                } else {
                    std::cout << "Invalid selection.\n";
                    break;
                }

                std::cout << "\nData Source:\n1. Manual Input\n2. Live from Server (Pi/Sim)\nChoice: ";
                int sourceOpt;
                std::cin >> sourceOpt;

                bool applyVariation = false;

                if (sourceOpt == 2) {
                    std::cout << "Is this the MAIN room (with the Raspberry Pi)? (1-Yes, 0-No/Other Room): ";
                    int isMainRoom;
                    std::cin >> isMainRoom;

                    if (isMainRoom == 0) {
                        applyVariation = true;
                        std::cout << "[INFO] Values will be simulated based on Real Pi data (Variations applied).\n";
                    }
                }

                for (int sensorIndex : sensorsToAdd) {

                    std::string type;
                    switch(sensorIndex) {
                        case 1: type = "Temperatura"; break;
                        case 2: type = "Umiditate"; break;
                        case 3: type = "Lumina"; break;
                        case 4: type = "CO2"; break;
                        case 5: type = "PM2.5"; break;
                        case 6: type = "Fum"; break;
                        case 7: type = "TVOC"; break;
                        case 8: type = "CO"; break;
                        case 9: type = "Sunet"; break;
                        default:
                            type = "Unknown";
                            std::cerr << "[Warning] Unknown sensor index provided.\n";
                            break;
                    }

                    double val = 0.0;

                    if (sourceOpt == 2) {
                        double realVal = getLiveValueFromPi(type, ip);

                        if (applyVariation) {
                            val = applySimulationOffset(realVal, type);
                            std::cout << "   -> Real: " << realVal << " | Simulated for this room: " << val << "\n";
                        } else {
                            val = realVal;
                        }
                    } else {
                        std::cout << "Enter value for " << type << ": ";
                        std::cin >> val;
                    }

                    bool found = false;

                    const auto& existingSensors = room->getSensorList();
                    for (const auto& sensor : existingSensors) {
                        if (sensor->getType() == type) {
                            sensor->updateValue(val);

                            std::cout << "[UPDATE] Senzorul " << type << " a fost actualizat la valoarea: " << val << "\n";
                            found = true;
                            break;
                        }
                    }

                    if (!found) {
                        Sensor* newSensor = nullptr;
                        try {
                            switch(sensorIndex) {
                                case 1: newSensor = new temperatureSensor(val); break;
                                case 2: newSensor = new humiditySensor(val); break;
                                case 3: newSensor = new lightSensor(val); break;
                                case 4: newSensor = new toxicGasSensor("CO2", val, "ppm"); break;
                                case 5: newSensor = new particleSensor(val); break;
                                case 6: newSensor = new smokeSensor(val); break;
                                case 7: newSensor = new toxicGasSensor("TVOC", val, "ppb"); break;
                                case 8: newSensor = new toxicGasSensor("CO", val, "ppm"); break;
                                case 9: newSensor = new soundSensor(val); break;
                                default:
                                    std::cout << "[ERROR] Cannot create sensor: Invalid index.\n";
                                    newSensor = nullptr;
                                    break;
                            }
                        } catch (...) {
                            std::cout << "[ERROR] Error creating " << type << "\n";
                            continue;
                        }

                        if (newSensor != nullptr) {
                            std::shared_ptr<Sensor> sharedSensor(newSensor);
                            room->addSensor(sharedSensor);
                            std::cout << "[SUCCESS] Added " << type << ": " << val << " " << newSensor->getUnit() << "\n";
                        }
                    }
                }

                std::cout << "\n[DONE] Finished adding sensors to " << roomName << ".\n";
                break;
            }
            case 3: {
                std::cout << "\n--- Running Analysis ---\n";

                std::string report = userEngine.generateStatusReport();
                std::cout << report;

                std::cout << "System Health Score: " << userEngine.calculateHealthScore() << "/100\n";

                std::vector<Alert> alerts = userEngine.generateAlerts();
                if (alerts.empty()) {
                    std::cout << "No alerts detected. System is stable.\n";
                } else {
                    std::cout << "\n!!! ALERTS DETECTED !!!\n";
                    for (const Alert& a : alerts) {
                        std::cout << "  -> " << a << "\n";
                    }
                }
                break;
            }
            case 4: {
                std::cout << "\n--- Displaying Current System ---\n";
                std::cout << userSystem;
                std::cout << "Total active sensors in simulation: " << Sensor::getTotalSensors() << "\n";
                break;
            }
            case 5: {
                std::string filename;
                std::cout << "Enter filename to save as (e.g., my_home.txt): ";
                clearInputBuffer();
                std::getline(std::cin, filename);

                ConfigManager::saveSystemToFile(filename, userSystem);
                break;
            }
            default: {
                std::cout << "Invalid option!\n";
                break;
            }
            }
        }
        catch (const HomeExceptions& e) {
            std::cout << "\n[!] OPERATION FAILED: " << e.what() << "\n";
        }
        catch (const std::exception& e) {
            std::cout << "\n[!] ERROR: " << e.what() << "\n";
        }
    }
}

void runDemoSystem(const std::string& ip) {
    std::cout << "\n--- Connecting to Environment at " << ip << " ---\n";

    // 2. datele de baza
    double baseTemp = getLiveValueFromPi("Temperatura", ip);
    double baseHum = getLiveValueFromPi("Umiditate", ip);
    double baseLux = getLiveValueFromPi("Lumina", ip);
    double baseCO2 = getLiveValueFromPi("CO2", ip);
    double basePM = getLiveValueFromPi("PM2.5", ip);
    double baseFum = getLiveValueFromPi("Fum", ip);
    double baseTVOC = getLiveValueFromPi("TVOC", ip);
    double baseCO = getLiveValueFromPi("CO", ip);
    double baseSunet = getLiveValueFromPi("Sunet", ip);

    std::cout << "[INIT] Data fetched. Building rooms...\n";

    HomeSystem home("The Homey (Live Demo)");

    // aici avem datele reale (in living se afla raspberry pi-ul)
    Room livingRoom("Living Room");
    livingRoom.addSensor(std::make_shared<temperatureSensor>(baseTemp));
    livingRoom.addSensor(std::make_shared<humiditySensor>(baseHum));
    livingRoom.addSensor(std::make_shared<toxicGasSensor>("TVOC", baseTVOC, "ppb"));
    livingRoom.addSensor(std::make_shared<toxicGasSensor>("CO2", baseCO2, "ppm"));
    livingRoom.addSensor(std::make_shared<particleSensor>(basePM));
    livingRoom.addSensor(std::make_shared<lightSensor>(baseLux));
    livingRoom.addSensor(std::make_shared<smokeSensor>(baseFum));
    livingRoom.addSensor(std::make_shared<toxicGasSensor>("CO", baseCO, "ppm"));
    livingRoom.addSensor(std::make_shared<soundSensor>(baseSunet));


    Room bedroom("Bedroom");
    bedroom.addSensor(std::make_shared<temperatureSensor>(applySimulationOffset(baseTemp, "Temperatura")));
    bedroom.addSensor(std::make_shared<humiditySensor>(applySimulationOffset(baseHum, "Umiditate")));
    bedroom.addSensor(std::make_shared<toxicGasSensor>("TVOC", applySimulationOffset(baseTVOC, "TVOC"), "ppb"));
    bedroom.addSensor(std::make_shared<toxicGasSensor>("CO2", applySimulationOffset(baseCO2, "CO2"), "ppm"));
    bedroom.addSensor(std::make_shared<particleSensor>(applySimulationOffset(basePM, "PM2.5")));
    bedroom.addSensor(std::make_shared<lightSensor>(applySimulationOffset(baseLux, "Lumina")));
    bedroom.addSensor(std::make_shared<smokeSensor>(baseFum)); // Fumul de obicei e la fel (0)
    bedroom.addSensor(std::make_shared<toxicGasSensor>("CO", baseCO, "ppm"));
    bedroom.addSensor(std::make_shared<soundSensor>(applySimulationOffset(baseSunet, "Sunet") - 10.0));


    Room kitchen("Kitchen");
    // aici adaugam un offset mai mare - in bucatarie e mai cald
    double kitchenTemp = applySimulationOffset(baseTemp, "Temperatura") + 2.0;

    kitchen.addSensor(std::make_shared<temperatureSensor>(kitchenTemp));
    kitchen.addSensor(std::make_shared<humiditySensor>(applySimulationOffset(baseHum, "Umiditate")));
    kitchen.addSensor(std::make_shared<toxicGasSensor>("TVOC", applySimulationOffset(baseTVOC, "TVOC"), "ppb"));
    kitchen.addSensor(std::make_shared<toxicGasSensor>("CO2", applySimulationOffset(baseCO2, "CO2"), "ppm"));
    kitchen.addSensor(std::make_shared<particleSensor>(applySimulationOffset(basePM, "PM2.5")));
    kitchen.addSensor(std::make_shared<lightSensor>(applySimulationOffset(baseLux, "Lumina")));
    kitchen.addSensor(std::make_shared<smokeSensor>(baseFum));
    kitchen.addSensor(std::make_shared<toxicGasSensor>("CO", baseCO, "ppm"));
    kitchen.addSensor(std::make_shared<soundSensor>(applySimulationOffset(baseSunet, "Sunet") + 15.0));

    Room garage("Garage");
    // gol

    home.addRoom(livingRoom);
    home.addRoom(bedroom);
    home.addRoom(kitchen);
    home.addRoom(garage);

    std::cout << home;


    AnalysisEngine homeBrain(home, "rules.txt");

    std::cout << "\n--- Test Status Report ---\n";
    std::string report = homeBrain.generateStatusReport();
    std::cout << report;

    std::cout << "\n--- Test Health Score ---\n";
    int score = homeBrain.calculateHealthScore();
    std::cout << "The health score of the home is: " << score << "/100\n";

    std::cout << "\n--- Test Alert Generation ---\n";
    std::vector<Alert> currentAlerts = homeBrain.generateAlerts();

    if (currentAlerts.empty()) {
        std::cout << "No alerts detected. System is stable.\n";
    } else {
        std::cout << "!!! ALERTS DETECTED !!!\n";
        for (const Alert& a : currentAlerts) {
            std::cout << "  -> " << a << "\n";
        }
    }
}



void clearInputBuffer() {
    if (std::cin.fail()) {
        std::cin.clear();
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}