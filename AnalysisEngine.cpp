//
// Created by bunea on 03.11.2025.
//

#include "AnalysisEngine.h"

#include <sstream>


AnalysisEngine::AnalysisEngine(const HomeSystem& sys) : system(sys) {

}

std::vector<Alert> AnalysisEngine::generateAlerts() const {
    std::vector<Alert> foundAlerts;

    const std::vector<Room>& rooms = this->system.getRoomList();
    int roomCount = this->system.getRoomCount();

    for (int i = 0; i < roomCount; i++) {
        const Room& room = rooms[i];

        const std::vector<Sensor>& sensors = room.getSensorList();
        int sensorCount = room.getSensorCount();

        for (int j = 0; j < sensorCount; j++) {
            const Sensor& sensor = sensors[j];
            // Rule 1: Critical Temperature
            if (sensor.getType() == "Temperatura" && sensor.getValue() > 35.0) {
                foundAlerts.push_back(Alert("Fire Hazard! Critical Temperature!", room.getName(), 1));
            }

            // Rule 2: Poor Air Quality
            if (sensor.getType() == "CO2" && sensor.getValue() > 1000.0) {
                foundAlerts.push_back(Alert("Poor Air Quality. Ventilate.", room.getName(), 2));
            }

            // Rule 3: Light is om
            if (sensor.getType() == "Lumina" && sensor.getValue() > 100.0) {
                foundAlerts.push_back(Alert("Light is on. Possible waste.", room.getName(), 3));
            }

            // de adaugat mai multe
        }
    }
    return foundAlerts;
}


std::string AnalysisEngine::generateStatusReport() const {
    std::stringstream ss;
    ss << "= SYSTEM STATUS REPORT (" << this->system.getSystemName() << ") =\n";

    const std::vector<Room>& rooms = this->system.getRoomList();
    int roomCount = this->system.getRoomCount();

    ss << "Total rooms monitored: " << roomCount << "\n";

    for (int i = 0; i < roomCount; i++) {
        ss << "Room: " << rooms[i].getName() << "\n";

        double avgTemp = rooms[i].calculateSensorAverage("Temperatura");
        double avgCO2 = rooms[i].calculateSensorAverage("CO2");
        ss << " - Avg Temp: " << avgTemp << " C\n";
        ss << " - Avg CO2:  " << avgCO2 << " ppm\n";
    }

    return ss.str();
}

int AnalysisEngine::calculateHealthScore() const {
    int totalScore = 100;

    std::vector<Alert> alerts = this->generateAlerts();

    for (int i = 0; i < alerts.size(); i++) {
        const Alert& alert = alerts[i];

        if (alert.getPriority() == 1) {
            totalScore -= 20;
        }

        if (alert.getPriority() == 2) {
            totalScore -= 5;
        }
    }

    if (totalScore < 0) {
        totalScore = 0;
    }

    return totalScore;
}
