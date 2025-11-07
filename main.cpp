#include <iostream>
#include <vector>
#include "Sensor.h"
#include "Room.h"
#include "HomeSystem.h"
#include "Alert.h"
#include "AnalysisEngine.h"
#include <limits>
#include "Rule.h"

void runDemoSystem();
void runInteractiveSystem();
void clearInputBuffer();


int main() {
    while (true) {
        std::cout << "\n===== THE HOMEY - Main Menu =====\n";
        std::cout << "1. Create New System\n";
        std::cout << "2. Run Demo System\n";
        std::cout << "0. Exit\n";
        std::cout << "Select option: ";

        int option;
        std::cin >> option;

        if (!std::cin) {
            std::cout << "Invalid input. Please enter a number.\n";
            clearInputBuffer();
            continue;
        }

        switch (option) {
            case 1:
                runInteractiveSystem();
                break;
            case 2:
                runDemoSystem();
                break;
            case 0:
                std::cout << "Goodbye!\n";
                return 0;
            default:
                std::cout << "Invalid option!\n";
                break;
        }
    }
}

void runInteractiveSystem() {
    std::string systemName;
    std::cout << "Enter new system name: ";
    clearInputBuffer();
    std::getline(std::cin, systemName);

    HomeSystem userSystem(systemName);
    AnalysisEngine userEngine(userSystem);

    userEngine.addRule(Rule("Critical Temperature", "Temperatura", ">", 35.0, "Fire Hazard! Critical Temperature!", 1));
    userEngine.addRule(Rule("Low CO2", "CO2", ">", 1000.0, "Poor Air Quality. Ventilate.", 2));
    userEngine.addRule(Rule("Lumina Inutila", "Lumina", ">", 100.0, "Light is on. Possible waste.", 3));
    userEngine.addRule(Rule("High Humidity", "Umiditate", ">", 70.0, "High humidity detected! Risk of mold.", 2));
    userEngine.addRule(Rule("Smoke Detected", "Fum", "==", 1.0, "CRITICAL: SMOKE DETECTED! EVACUATE!", 1));
    userEngine.addRule(Rule("CO Detected", "CO", ">", 30.0, "CRITICAL: High CO levels detected! Ventilate!", 1));
    userEngine.addRule(Rule("High Particulates", "PM2.5", ">", 35.0, "Poor Air Quality (PM2.5). Run purifier.", 2));
    userEngine.addRule(Rule("Loud Noise", "Sunet", ">", 80.0, "Loud noise detected. Check for anomaly.", 2));
    userEngine.addRule(Rule("Water Leak", "Apa", "==", 1.0, "CRITICAL: Water leak detected!", 1));
    userEngine.addRule(Rule("Window Open", "ContactFereastra", "==", 1.0, "Window open. Check security/efficiency.", 3));

    while(true) {
        std::cout << "\n--- System: " << systemName << "\n";
        std::cout << "1. Add Room\n";
        std::cout << "2. Add Sensor (to a Room)\n";
        std::cout << "3. Run Analysis\n";
        std::cout << "4. Display Full System\n";
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
                    std::cout << "Error: Room '" << roomName << "' not found.\n";
                } else {
                    std::string type;
                    double value;
                    std::string unit;
                    std::cout << "Enter sensor type (e.g., Temperatura, CO2): ";
                    std::cin >> type;
                    std::cout << "Enter sensor value (e.g., 22.5): ";
                    std::cin >> value;
                    std::cout << "Enter sensor unit (e.g., C, ppm): ";
                    std::cin >> unit;

                    room->addSensor(Sensor(type, value, unit));
                    std::cout << "Sensor added to " << roomName << ".\n";
                }
                break;
            }
            case 3: {
                std::cout << "\n--- Running Analysis ---\n";
                std::vector<Alert> alerts = userEngine.generateAlerts();
                if (alerts.empty()) {
                    std::cout << "No alerts detected. System is stable.\n";
                } else {
                    std::cout << "!!! ALERTS DETECTED !!!\n";
                    for (const Alert& a : alerts) {
                        std::cout << "  -> " << a << "\n";
                    }
                }
                std::cout << "System Health Score: " << userEngine.calculateHealthScore() << "/100\n";
                break;
            }
            case 4: {
                std::cout << "\n--- Displaying Current System ---\n";
                std::cout << userSystem;
                break;
            }
            default:
                std::cout << "Invalid option!\n";
                break;
        }
    }
}
void runDemoSystem() {
    std::cout << "\n--- Running Demo System ---\n";


    HomeSystem home("The Homey (Demo)");

    Room livingRoom("Living Room");
    livingRoom.addSensor(Sensor("Temperatura", 22.5, "C"));
    livingRoom.addSensor(Sensor("CO2", 450.0, "ppm"));
    livingRoom.addSensor(Sensor("Lumina", 300.0, "lux"));
    livingRoom.addSensor(Sensor("Sunet", 40.0, "dB"));

    Room bedroom("Bedroom");
    bedroom.addSensor(Sensor("Temperatura", 19.0, "C"));
    bedroom.addSensor(Sensor("CO2", 1100.0, "ppm"));
    bedroom.addSensor(Sensor("ContactFereastra", 1.0, "bool"));

    Room kitchen("Kitchen");
    kitchen.addSensor(Sensor("Temperatura", 40.0, "C"));
    kitchen.addSensor(Sensor("Fum", 1.0, "bool"));
    kitchen.addSensor(Sensor("CO", 35.0, "ppm"));
    kitchen.addSensor(Sensor("PM2.5", 50.0, "ug/m3"));

    Room bathroom("Bathroom");
    bathroom.addSensor(Sensor("Umiditate", 75.0, "%"));
    bathroom.addSensor(Sensor("Apa", 1.0, "bool"));

    home.addRoom(livingRoom);
    home.addRoom(bedroom);
    home.addRoom(kitchen);
    home.addRoom(bathroom);

    std::cout << home;

    AnalysisEngine homeBrain(home);

    homeBrain.addRule(Rule("Critical Temperature", "Temperatura", ">", 35.0, "Fire Hazard! Critical Temperature!", 1));
    homeBrain.addRule(Rule("Low CO2", "CO2", ">", 1000.0, "Poor Air Quality. Ventilate.", 2));
    homeBrain.addRule(Rule("Lumina Inutila", "Lumina", ">", 100.0, "Light is on. Possible waste.", 3));
    homeBrain.addRule(Rule("High Humidity", "Umiditate", ">", 70.0, "High humidity detected! Risk of mold.", 2));
    homeBrain.addRule(Rule("Smoke Detected", "Fum", "==", 1.0, "CRITICAL: SMOKE DETECTED! EVACUATE!", 1));
    homeBrain.addRule(Rule("CO Detected", "CO", ">", 30.0, "CRITICAL: High CO levels detected! Ventilate!", 1));
    homeBrain.addRule(Rule("High Particulates", "PM2.5", ">", 35.0, "Poor Air Quality (PM2.5). Run purifier.", 2));
    homeBrain.addRule(Rule("Loud Noise", "Sunet", ">", 80.0, "Loud noise detected. Check for anomaly.", 2));
    homeBrain.addRule(Rule("Water Leak", "Apa", "==", 1.0, "CRITICAL: Water leak detected!", 1));
    homeBrain.addRule(Rule("Window Open", "ContactFereastra", "==", 1.0, "Window open. Check security/efficiency.", 3));


    std::cout << "\n--- Test Status Report ---\n";
    std::string report = homeBrain.generateStatusReport();
    std::cout << report;

    std::cout << "\n--- Test Health Score ---\n";
    int score = homeBrain.calculateHealthScore();
    std::cout << "The health score of the home is: " << score << "/100\n";

    std::cout << "\n--- Test Alert Generation (Rule-Based) ---\n";
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
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    if (std::cin.fail()) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
}