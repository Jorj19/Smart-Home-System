//
// Created by bunea on 03.11.2025.
//

#include "AnalysisEngine.h"

#include <sstream>


AnalysisEngine::AnalysisEngine(const HomeSystem& sys) : system(sys) {

}

std::vector<Alert> AnalysisEngine::generateAlerts() const {
    std::vector<Alert> foundAlerts;

    for (const Room& room : this->system.getRoomList()) {

        for (const Sensor& sensor : room.getSensorList()) {
            // Rule 1: Critical Temperature
            if (sensor.getType() == "Temperatura" && sensor.getValue() > 35.0) {
                foundAlerts.emplace_back("Fire Hazard! Critical Temperature!", room.getName(), 1);
            }

            // Rule 2: Poor Air Quality
            if (sensor.getType() == "CO2" && sensor.getValue() > 1000.0) {
                foundAlerts.emplace_back("Poor Air Quality. Ventilate.", room.getName(), 2);
            }

            // Rule 3: Light is om
            if (sensor.getType() == "Lumina" && sensor.getValue() > 100.0) {
                foundAlerts.emplace_back("Light is on. Possible waste.", room.getName(), 3);
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

    ss << "Total rooms monitored: " << rooms.size() << "\n";

    for (const Room& room : rooms) {
        ss << "Room: " << room.getName() << "\n";

        double avgTemp = room.calculateSensorAverage("Temperatura");
        double avgCO2 = room.calculateSensorAverage("CO2");
        ss << " - Avg Temp: " << avgTemp << " C\n";
        ss << " - Avg CO2:  " << avgCO2 << " ppm\n";
    }

    return ss.str();
}

int AnalysisEngine::calculateHealthScore() const {
    int totalScore = 100;

    std::vector<Alert> alerts = this->generateAlerts();

    for (const Alert& alert : alerts) {

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
