#include <iostream>
#include <string>
#include <vector>
#include <limits>

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
                double value;
                std::string unit;
                std::cout << "Enter sensor type (e.g., Temperatura, CO2, Umiditate, Fum...): ";
                std::cin >> type;
                std::cout << "Enter sensor value (e.g., 22.5, 1 for ON, 0 for OFF): ";
                std::cin >> value;
                if (!std::cin) {
                    clearInputBuffer();
                    throw InvalidDataSensorException("Value must be a number");
                }
                std::cout << "Enter sensor unit (e.g., C, ppm, %, bool): ";
                std::cin >> unit;

                room->addSensor(Sensor(type, value, unit));
                std::cout << "Sensor added to " << roomName << ".\n";
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

    Room livingRoom("Living Room");
    livingRoom.addSensor(Sensor("Temperatura", 23.0, "C"));
    livingRoom.addSensor(Sensor("Umiditate", 50.0, "%")); // Ideal
    livingRoom.addSensor(Sensor("TVOC", 100.0, "ppb"));
    livingRoom.addSensor(Sensor("CO2", 500.0, "ppm"));
    livingRoom.addSensor(Sensor("PM2.5", 10.0, "ug/m3"));
    livingRoom.addSensor(Sensor("Lumina", 450.0, "lux")); // Good
    livingRoom.addSensor(Sensor("Sunet", 40.0, "dB")); // Low
    livingRoom.addSensor(Sensor("Fum", 0.0, "bool"));
    livingRoom.addSensor(Sensor("CO", 0.0, "ppm"));
    livingRoom.addSensor(Sensor("ContactFereastra", 0.0, "bool"));

    Room bedroom("Bedroom");
    bedroom.addSensor(Sensor("Temperatura", 18.5, "C")); // Alert
    bedroom.addSensor(Sensor("Umiditate", 55.0, "%"));
    bedroom.addSensor(Sensor("TVOC", 50.0, "ppb"));
    bedroom.addSensor(Sensor("CO2", 1100.0, "ppm")); // Alert
    bedroom.addSensor(Sensor("PM2.5", 5.0, "ug/m3"));
    bedroom.addSensor(Sensor("Lumina", 50.0, "lux")); // Too Dark
    bedroom.addSensor(Sensor("Sunet", 30.0, "dB"));
    bedroom.addSensor(Sensor("Fum", 0.0, "bool"));
    bedroom.addSensor(Sensor("CO", 0.0, "ppm"));
    bedroom.addSensor(Sensor("ContactFereastra", 1.0, "bool")); // Alert

    Room kitchen("Kitchen");
    kitchen.addSensor(Sensor("Temperatura", 40.0, "C")); // Alert
    kitchen.addSensor(Sensor("Umiditate", 75.0, "%")); // Alert
    kitchen.addSensor(Sensor("TVOC", 600.0, "ppb")); // Alert
    kitchen.addSensor(Sensor("CO2", 800.0, "ppm"));
    kitchen.addSensor(Sensor("PM2.5", 40.0, "ug/m3")); // Alert
    kitchen.addSensor(Sensor("Lumina", 600.0, "lux"));
    kitchen.addSensor(Sensor("Sunet", 65.0, "dB"));
    kitchen.addSensor(Sensor("Fum", 1.0, "bool")); // Alert
    kitchen.addSensor(Sensor("CO", 35.0, "ppm")); // Alert
    kitchen.addSensor(Sensor("ContactFereastra", 0.0, "bool"));

    Room garage("Garage");
    garage.addSensor(Sensor("ContactFereastra", 0.0, "bool"));

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