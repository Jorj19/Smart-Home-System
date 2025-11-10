//
// Created by bunea on 03.11.2025.
//

#include "AnalysisEngine.h"

#include <sstream>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cctype>



AnalysisEngine::AnalysisEngine(const HomeSystem& sys, const std::string& rulesFilename) : system(sys) {
    this->loadRulesFromFile(rulesFilename);
}

void AnalysisEngine::addRule(const Rule& rule) {
    this->ruleList.push_back(rule);
}

// cppcheck-suppress unusedFunction
const std::vector<Rule>& AnalysisEngine::getRuleList() const {
    return this->ruleList;
}


void AnalysisEngine::loadRulesFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cout << "CRITICAL ERROR: Could not load rules file: " << filename << "\n";
        return;
    }

    std::cout << "Loading default rules from " << filename << "...\n";
    std::string line;
    int lineCount = 0;

    while (std::getline(file, line)) {
        lineCount++;
        std::stringstream ss(line);
        std::string segment;
        std::vector<std::string> parts;
        while (std::getline(ss, segment, ',')) {
            parts.push_back(segment);
        }
        if (parts.empty() || parts[0].empty() || parts[0][0] == '#') continue;


        try {
            if (const std::string& type = parts[0]; type == "RULE" && parts.size() == 7) {
                Rule newRule(parts[1], parts[2], parts[3], std::stod(parts[4]), parts[5], std::stoi(parts[6]));
                this->addRule(newRule);
            }
        } catch (const std::invalid_argument& e) {
            std::cout << "Error (Rules L" << lineCount << "): Bad data format in line: " << line << "'. Reason: " << e.what() << "\n";
        }
    }
    file.close();
    std::cout << "Successfully loaded " << this->ruleList.size() << " rules.\n";
}


ComfortCategory AnalysisEngine::calculateRoomComfortCategory(const Room& room) {
    // -1 = Fără Date
    // 0  = Ideal
    // 1  = Acceptabil (Ușor cald/rece)
    // 2  = Prea Cald
    // -2 = Prea Rece
    double temp = -99.0;
    double humidity = -99.0;

    for (const Sensor& sensor : room.getSensorList()) {
        if (sensor.getType() == "Temperatura") {
            temp = sensor.getValue();
        }
        if (sensor.getType() == "Umiditate") {
            humidity = sensor.getValue();
        }
    }

    if (temp == -99 || humidity == -99) {
        return ComfortCategory::NoData; // No data
    }

    if (temp > 20 && temp < 24 && humidity > 40 && humidity < 60) {
        return ComfortCategory::Ideal; //Ideal comfort
    }

    if (temp > 26 || humidity > 70) {
        return ComfortCategory::TooHot; // Too hot
    }

    if (temp < 19) {
        return ComfortCategory::TooCold; // too cold
    }

    return ComfortCategory::Acceptable;
}

std::string AnalysisEngine::getRoomAcousticDiscomfort(const Room& room) {
    for (const Sensor& sensor : room.getSensorList()) {
        if (sensor.getType() == "Sunet") {
            double db = sensor.getValue();
            if (db > 70) return "High";
            if (db > 45) return "Moderate";
            return "Low";
        }
    }
    return "N/A";
}

std::string AnalysisEngine::getRoomLuminousDiscomfort(const Room& room) {
    for (const Sensor& sensor : room.getSensorList()) {
        if (sensor.getType() == "Lumina") {
            double lux = sensor.getValue();
            if (lux > 2000) return "Glare";
            if (lux < 100) return "Too Dark";
            return "Good";
        }
    }
    return "N/A";
}


std::vector<Alert> AnalysisEngine::generateAlerts() const {
    std::vector<Alert> foundAlerts;

    for (const Room& room : this->system.getRoomList()) {

        for (const Sensor& sensor : room.getSensorList()) {

            for (const Rule& rule : this->ruleList) {
                if (rule.isTriggeredBy(sensor)) {
                    foundAlerts.push_back(Alert(rule.getMessage(), room.getName(), rule.getPriority()));
                }
            }
        }
    }
    return foundAlerts;
}

std::string AnalysisEngine::generateStatusReport() const {
    std::stringstream ss;
    ss << "===== SYSTEM STATUS REPORT (" << this->system.getSystemName() << ") =====\n";

    const std::vector<Room>& rooms = this->system.getRoomList();
    ss << "Total rooms monitored: " << rooms.size() << "\n";
    ss << "-------------------------------\n";

    for (const Room& room : rooms) {
        ss << "Room: " << room.getName() << "\n";

        std::string comfortText;
        switch (ComfortCategory comfortCode = calculateRoomComfortCategory(room); comfortCode) {
        case ComfortCategory::Ideal: comfortText = "Ideal"; break;
        case ComfortCategory::Acceptable: comfortText = "Acceptable"; break;
        case ComfortCategory::TooHot: comfortText = "Too Hot/Humid"; break;
        case ComfortCategory::TooCold: comfortText = "Too Cold"; break;
        case ComfortCategory::NoData: comfortText = "N/A (Missing Data)"; break;
        }

        ss << "  - Thermal Comfort:     " << comfortText << "\n";
        ss << "  - Acoustic Discomfort:   " << getRoomAcousticDiscomfort(room) << "\n";
        ss << "  - Luminous Discomfort:   " << getRoomLuminousDiscomfort(room) << "\n";
    }

    SystemIndices indices = this->calculateSystemIndices();
    ss << "-------------------------------\n";
    ss << "OVERALL SYSTEM INDICES:\n";
    ss << "  - Ideal Rooms: " << indices.roomsIdeal << "\n";
    ss << "  - Acceptable Rooms: " << indices.roomsAcceptable << "\n";
    ss << "  - Hot Rooms: " << indices.roomsHot << "\n";
    ss << "  - Cold Rooms: " << indices.roomsCold << "\n";
    ss << "  - (No Data Rooms): " << indices.roomsNoData << "\n";
    ss << "  - Acoustic Discomfort: " << indices.overallAcousticDiscomfort << "\n";
    ss << "  - Luminous Discomfort: " << indices.overallLuminousDiscomfort << "\n";
    ss << "=================================\n";

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

SystemIndices AnalysisEngine::calculateSystemIndices() const {
    SystemIndices indices = {0, 0, 0, 0, 0, "N/A", "N/A"};
    const std::vector<Room>& rooms = this->system.getRoomList();
    if (rooms.empty()) {
        return indices;
    }

    int noiseCount = 0;
    int highNoise = 0;
    int lightCount = 0;
    int badLight = 0;

    for (const Room& room : rooms) {
        switch (ComfortCategory comfortCode = calculateRoomComfortCategory(room); comfortCode) {
        case ComfortCategory::Ideal: indices.roomsIdeal++; break;
        case ComfortCategory::Acceptable: indices.roomsAcceptable++; break;
        case ComfortCategory::TooHot: indices.roomsHot++; break;
        case ComfortCategory::TooCold: indices.roomsCold++; break;
        case ComfortCategory::NoData: indices.roomsNoData++; break;
        }

        std::string acoustic = getRoomAcousticDiscomfort(room);
        if (acoustic != "N/A") {
            noiseCount++;
            if (acoustic == "High" || acoustic == "Moderate") {
                highNoise++;
            }
        }

        std::string luminous = getRoomLuminousDiscomfort(room);
        if (luminous != "N/A") {
            lightCount++;
            if (luminous == "Glare" || luminous == "Too Dark") {
                badLight++;
            }
        }
    }

    if (noiseCount > 0) {
        indices.overallAcousticDiscomfort = std::to_string(highNoise) + "/" + std::to_string(noiseCount) + " rooms have discomfort";
    }
    if (lightCount > 0) {
        indices.overallLuminousDiscomfort = std::to_string(badLight) + "/" + std::to_string(lightCount) + " rooms have discomfort";
    }

    return indices;

}
