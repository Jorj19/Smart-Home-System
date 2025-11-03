#include <iostream>
#include <vector>
#include "Sensor.h"
#include "Room.h"
#include "HomeSystem.h"
#include "Alert.h"
#include "AnalysisEngine.h"

int main() {
    //Creare Senzori
    std::cout << "Creating Sensors...\n";
    Sensor livingTemp("Temperatura", 22.5, "C");
    Sensor livingCO2("CO2", 450.0, "ppm");
    Sensor livingLight("Lumina", 300.0, "lux");

    Sensor bedroomTemp("Temperatura", 19.0, "C");
    Sensor bedroomCO2("CO2", 1100.0, "ppm"); // val mare
    Sensor kitchenTemp("Temperatura", 40.0, "C"); // val mare

    livingTemp.updateValue(22.6);

    //Creare Camera
    std::cout << "\nCreating 'Living Room' and adding sensors...\n";
    Room livingRoom("Living Room");
    livingRoom.addSensor(livingTemp);
    livingRoom.addSensor(livingCO2);
    livingRoom.addSensor(livingLight);

    std::cout << livingRoom;

    std::cout << "\n- Testing 'Room' non-trivial functions -\n";
    double avgTemp = livingRoom.calculateSensorAverage("Temperatura");
    std::cout << "Average temperature in Living Room: " << avgTemp << " C\n";

    // Test funcția de prag
    livingRoom.displaySensorsOverThreshold(400.0, "CO2");
    livingRoom.displaySensorsOverThreshold(500.0, "Lumina");

    std::cout << "\n- Testing Copy Constructor (Room) -\n";
    Room livingRoomCopy = livingRoom;
    livingRoomCopy.addSensor(Sensor("Test", 1.0, "T"));
    std::cout << "- ORIGINAL (should not have 'Test' sensor) -\n" << livingRoom;
    std::cout << "- COPY (should have 'Test' sensor) -\n" << livingRoomCopy;

    std::cout << "\n- Testing Operator= (Room) -\n";
    Room office("Office"); // Creăm o cameră nouă
    office = livingRoom; // Se apelează Operator= (auto-generat)
    std::cout << "- OFFICE (copied from Living Room) -\n" << office;

    //Creare HomeSystem
    std::cout << "\n Creating 'The Homey' System and adding rooms... \n";
    HomeSystem home("The Homey");
    home.addRoom(livingRoom);

    Room bedroom("Bedroom");
    bedroom.addSensor(bedroomTemp);
    bedroom.addSensor(bedroomCO2);

    home.addRoom(bedroom);

    Room kitchen("Kitchen");
    kitchen.addSensor(kitchenTemp);
    home.addRoom(kitchen);

    std::cout << "\n Displaying Full Home System \n";
    std::cout << home;

    home.findRoomsWithCriticalSensors("Temperatura", 30.0);

    std::cout << "\n Creating and Testing AnalysisEngine\n";

    AnalysisEngine homeBrain(home);

    // Test 1: Generare Raport Sumar
    std::cout << "\n Test Status Report \n";
    std::string report = homeBrain.generateStatusReport();
    std::cout << report;

    // Test 2: Calculare Scor
    std::cout << "\n Test Health Score \n";
    int score = homeBrain.calculateHealthScore();
    std::cout << "The health score of the home is: " << score << "/100\n";

    // Test 3: Generare Alerte
    std::cout << "\n Test Alert Generation \n";
    std::vector<Alert> currentAlerts = homeBrain.generateAlerts();


    if (currentAlerts.empty()) {
        std::cout << "No alerts detected. System is stable.\n";
    } else {
        std::cout << "!!! ALERTS DETECTED !!!\n";
        for (const Alert& a : currentAlerts) {
            std::cout << "  -> " << a << "\n";
        }
    }

    return 0;
}