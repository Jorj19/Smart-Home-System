#include <iostream>
#include <string>
#include <vector>
#include <limits>
#include <memory>
#include <algorithm>

#include "Sensor.h"
#include "Room.h"
#include "HomeSystem.h"
#include "Alert.h"
#include "AnalysisEngine.h"
#include "ConfigManager.h"
#include "HomeExceptions.h"

void runDemoSystem();
void runInteractiveSystem();
void clearInputBuffer();

std::string toLower(std::string str) {
    std::ranges::transform(str.begin(), str.end(), str.begin(),
                   [](unsigned char c){ return std::tolower(c); });
    return str;
}

int main() {
    try {
        std::cout << "\n===== THE HOMEY - Main Menu =====\n";
        std::cout << "1. Create New System \n";
        std::cout << "2. Run Demo System\n";
        std::cout << "3. Load System from File \n";
        std::cout << "0. Exit\n";
        std::cout << "Select option: ";

        int option;
        std::cin >> option;

        if (!std::cin) {
            std::cout << "Invalid input. Please enter a number.\n";
            clearInputBuffer();
        }

        switch (option) {
        case 1:
            runInteractiveSystem();
            break;
        case 2:
            runDemoSystem();
            break;
        case 3: {
            std::string filename;
            std::cout << "Enter configuration file to load (e.g., config.txt or my_home.txt): ";
            clearInputBuffer();
            std::getline(std::cin, filename);

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


void runInteractiveSystem() {
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

        int option;
        std::cin >> option;
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
                std::string type;
                double val;
                std::cout << "Enter sensor type (e.g., Temperatura, CO2, Umiditate, Fum...): ";
                std::cin >> type;
                std::cout << "Enter sensor value (e.g., 22.5, 1 for ON, 0 for OFF): ";
                std::cin >> val;
                if (!std::cin) {
                    throw InvalidDataSensorException("Value must be a number");
                }
                clearInputBuffer();

                std::shared_ptr<Sensor> newSensor = nullptr;
                std::string sensorType = toLower(type);

                if (sensorType == "temperatura") {
                    newSensor = std::make_shared<temperatureSensor>(val);
                }
                else if (sensorType == "umiditate") {
                    newSensor = std::make_shared<humiditySensor>(val);
                }
                else if (sensorType == "lumina") {
                    newSensor = std::make_shared<lightSensor>(val);
                }
                else if (sensorType == "fum") {
                    newSensor = std::make_shared<smokeSensor>(val);
                }
                else if (sensorType == "pm2.5" || type == "pm25") {
                    newSensor = std::make_shared<particleSensor>(val);
                }
                else if (sensorType == "co" || type == "co2" || type == "tvoc") {
                    std::string unit;
                    std::cout << "Enter Unit (ppm, ppb): ";
                    std::getline(std::cin, unit);

                    // Pentru gaze, normalizăm numele să fie Uppercase (CO2, nu co2)
                    std::string normalizedType = type;
                    if(sensorType == "co") normalizedType = "CO";
                    if(sensorType == "co2") normalizedType = "CO2";
                    if(sensorType == "tvoc") normalizedType = "TVOC";

                    newSensor = std::make_shared<toxicGasSensor>(normalizedType, val, unit);
                }
                else {
                    std::cout << "[WARNING] Unknown sensor type. Sensor NOT added.\n";
                }

                if (newSensor) {
                    room->addSensor(newSensor);
                    std::cout << "[SUCCESS] " << type << " sensor added to " << roomName << ".\n";
                }
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
                break;
            }
            case 5: {
                std::string filename;
                std::cout << "Enter filename to save as (e.g., my_home.txt): ";
                clearInputBuffer();
                std::getline(std::cin, filename);

                // Salvăm DOAR sistemul (fără motorul de reguli)
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

void runDemoSystem() {
    std::cout << "\n--- Running Demo System (Full Sensor Suite) ---\n";

    HomeSystem home("The Homey (Demo)");


    // LIVING ROOM
    Room livingRoom("Living Room");
    // Folosim make_shared pentru a crea pointeri smart catre clasele derivate
    livingRoom.addSensor(std::make_shared<temperatureSensor>(23.0));
    livingRoom.addSensor(std::make_shared<humiditySensor>(50.0));
    livingRoom.addSensor(std::make_shared<toxicGasSensor>("TVOC", 100.0, "ppb"));
    livingRoom.addSensor(std::make_shared<toxicGasSensor>("CO2", 500.0, "ppm"));
    livingRoom.addSensor(std::make_shared<particleSensor>(10.0));
    livingRoom.addSensor(std::make_shared<lightSensor>(450.0));
    livingRoom.addSensor(std::make_shared<smokeSensor>(0.0));
    livingRoom.addSensor(std::make_shared<toxicGasSensor>("CO", 0.0, "ppm"));

    // BEDROOM
    Room bedroom("Bedroom");
    bedroom.addSensor(std::make_shared<temperatureSensor>(18.5)); // Putina racoare (Notice)
    bedroom.addSensor(std::make_shared<humiditySensor>(55.0));
    bedroom.addSensor(std::make_shared<toxicGasSensor>("TVOC", 50.0, "ppb"));
    bedroom.addSensor(std::make_shared<toxicGasSensor>("CO2", 1100.0, "ppm")); // Aer inchis (Notice)
    bedroom.addSensor(std::make_shared<particleSensor>(5.0));
    bedroom.addSensor(std::make_shared<lightSensor>(50.0)); // Intuneric
    bedroom.addSensor(std::make_shared<smokeSensor>(0.0));
    bedroom.addSensor(std::make_shared<toxicGasSensor>("CO", 0.0, "ppm"));

    // KITCHEN (Multe alerte)
    Room kitchen("Kitchen");
    kitchen.addSensor(std::make_shared<temperatureSensor>(65.0)); // CRITIC (Foc)
    kitchen.addSensor(std::make_shared<humiditySensor>(75.0));   // Warning
    kitchen.addSensor(std::make_shared<toxicGasSensor>("TVOC", 600.0, "ppb"));
    kitchen.addSensor(std::make_shared<toxicGasSensor>("CO2", 800.0, "ppm"));
    kitchen.addSensor(std::make_shared<particleSensor>(40.0));   // Notice
    kitchen.addSensor(std::make_shared<lightSensor>(600.0));
    kitchen.addSensor(std::make_shared<smokeSensor>(1.0));       // CRITIC (Fum)
    kitchen.addSensor(std::make_shared<toxicGasSensor>("CO", 35.0, "ppm")); // Warning

    Room garage("Garage");
    // Garage gol

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