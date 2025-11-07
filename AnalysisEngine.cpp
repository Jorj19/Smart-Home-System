//
// Created by bunea on 03.11.2025.
//

#include "AnalysisEngine.h"

#include <sstream>


AnalysisEngine::AnalysisEngine(const HomeSystem& sys) : system(sys) {

}

void AnalysisEngine::addRule(const Rule& rule) {
    this->ruleList.push_back(rule);
}

const std::vector<Rule>& AnalysisEngine::getRuleList() const {
    return this->ruleList;
}


std::vector<Alert> AnalysisEngine::generateAlerts() const {
    std::vector<Alert> foundAlerts;

    for (const Room& room : this->system.getRoomList()) {

        for (const Sensor& sensor : room.getSensorList()) {

            for (const Rule& rule : this->ruleList) {
                if (rule.isTriggeredBy(sensor)) {
                    foundAlerts.emplace_back(rule.getMessage(), room.getName(), rule.getPriority());
                }
            }
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
