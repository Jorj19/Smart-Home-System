#include "AnalysisEngine.h"
#include "HomeExceptions.h"
#include <sstream>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cctype>
#include <cmath>

AnalysisEngine::AnalysisEngine(const HomeSystem& sys, const std::string& rulesFilename)
    : system(sys) {
    try {
        loadRulesFromFile(rulesFilename);
    } catch (const FileConfigException& e) {
        std::cout << "[INFO] " << e.what() << " (" << rulesFilename
                  << "). Running without custom rules.\n";
    }
}

void AnalysisEngine::addRule(const Rule& rule) {
    ruleList.push_back(rule);
}

const std::vector<Rule>& AnalysisEngine::getRuleList() const {
    return ruleList;
}

void AnalysisEngine::loadRulesFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw FileConfigException(filename);
    }

    std::cout << "Loading default rules from " << filename << "...\n";
    std::string line;
    int lineCount = 0;

    while (std::getline(file, line)) {
        lineCount++;

        if (line.empty()) continue;
        if (line[0] == '#') continue;

        std::stringstream ss(line);
        std::string segment;
        std::vector<std::string> parts;

        while (std::getline(ss, segment, ',')) {
            parts.push_back(segment);
        }

        if (parts.size() < 7) continue;
        if (parts[0] != "RULE") continue;

        try {
            Rule newRule(parts[1], parts[2], parts[3], std::stod(parts[4]), parts[5], std::stoi(parts[6]));
            addRule(newRule);
        } catch (const std::exception& e) {
            throw RuleEngineException("Line " + std::to_string(lineCount) + ": " + e.what());
        }
    }

    std::cout << "Successfully loaded " << ruleList.size() << " rules.\n";
}

ComfortCategory AnalysisEngine::calculateRoomComfortCategory(const Room& room) {
    double temp = -99.0;
    double humidity = -99.0;

    for (const auto& sensor : room.getSensorList()) {
        if (!sensor) continue;
        if (sensor->getType() == "Temperatura") temp = sensor->getValue();
        if (sensor->getType() == "Umiditate") humidity = sensor->getValue();
    }

    if (temp == -99.0 || humidity == -99.0) {
        return ComfortCategory::NoData;
    }

    if (temp > 20 && temp < 24 && humidity > 40 && humidity < 60) {
        return ComfortCategory::Ideal;
    }

    if (temp > 26 || humidity > 70) {
        return ComfortCategory::TooHot;
    }

    if (temp < 19) {
        return ComfortCategory::TooCold;
    }

    return ComfortCategory::Acceptable;
}

std::string AnalysisEngine::getRoomAcousticDiscomfort(const Room& room) {
    for (const auto& sensor : room.getSensorList()) {
        if (!sensor) continue;
        if (sensor->getType() == "Sunet") {
            double db = sensor->getValue();
            if (db > 70) return "High";
            if (db > 45) return "Moderate";
            return "Low";
        }
    }
    return "N/A";
}

std::string AnalysisEngine::getRoomLuminousDiscomfort(const Room& room) {
    for (const auto& sensor : room.getSensorList()) {
        if (!sensor) continue;
        if (sensor->getType() == "Lumina") {
            double lux = sensor->getValue();
            if (lux > 2000) return "Glare";
            if (lux < 50) return "Too Dark";
            return "Good";
        }
    }
    return "N/A";
}

double AnalysisEngine::calculateAirQualityScore(const Room& room) const {
    double score = 100.0;

    for (const auto& sensor : room.getSensorList()) {
        if (!sensor) continue;

        const std::string& t = sensor->getType();
        const bool airRelevant = (t == "CO2" || t == "TVOC" || t == "PM2.5" || t == "CO");

        if (!airRelevant) continue;

        for (const Rule& rule : ruleList) {
            if (rule.isTriggeredBy(*sensor)) {
                if (rule.getPriority() == 1) score -= 30;
                else if (rule.getPriority() == 2) score -= 15;
                else score -= 7;
            }
        }

        double hazard = sensor->calculateHazardLevel();
        if (hazard >= 100.0) score -= 25;
        else if (hazard >= 60.0) score -= 10;
    }

    return std::max(0.0, score);
}

double AnalysisEngine::calculateComfortScore(const Room& room) const {
    double score = 100.0;

    for (const auto& sensor : room.getSensorList()) {
        if (!sensor) continue;

        const std::string& t = sensor->getType();
        const bool comfortRelevant = (t == "Temperatura" || t == "Umiditate" || t == "Lumina" || t == "Sunet");

        if (!comfortRelevant) continue;

        for (const Rule& rule : ruleList) {
            if (rule.isTriggeredBy(*sensor)) {
                if (rule.getPriority() == 1) score -= 20;
                else if (rule.getPriority() == 2) score -= 10;
                else score -= 5;
            }
        }

        double hazard = sensor->calculateHazardLevel();
        if (hazard >= 100.0) score -= 15;
        else if (hazard >= 60.0) score -= 7;
    }

    return std::max(0.0, score);
}

double AnalysisEngine::calculateSafetyScore(const Room& room) const {
    double score = 100.0;

    for (const auto& sensor : room.getSensorList()) {
        if (!sensor) continue;

        const std::string& t = sensor->getType();
        const bool safetyRelevant = (t == "Fum" || t == "fum" || t == "CO");

        if (!safetyRelevant) continue;

        for (const Rule& rule : ruleList) {
            if (rule.isTriggeredBy(*sensor)) {
                if (rule.getPriority() == 1) score -= 50;
                else if (rule.getPriority() == 2) score -= 25;
                else score -= 10;
            }
        }

        double hazard = sensor->calculateHazardLevel();
        if (hazard >= 100.0) score -= 40;
        else if (hazard >= 60.0) score -= 20;
    }

    return std::max(0.0, score);
}

std::vector<Alert> AnalysisEngine::generateAlerts() const {
    std::vector<Alert> foundAlerts;

    for (const Room& room : system.getRoomList()) {
        for (const auto& sensor : room.getSensorList()) {
            if (!sensor) continue;

            for (const Rule& rule : ruleList) {
                if (rule.isTriggeredBy(*sensor)) {
                    foundAlerts.emplace_back(rule.getMessage(), room.getName(), rule.getPriority());
                }
            }

            double hazard = sensor->calculateHazardLevel();
            if (hazard >= 100.0) {
                foundAlerts.emplace_back("CRITICAL HAZARD: " + sensor->getType(), room.getName(), 1);
            } else if (hazard >= 60.0) {
                foundAlerts.emplace_back("WARNING: " + sensor->getType() + " high levels", room.getName(), 2);
            }
        }
    }

    return foundAlerts;
}

std::string AnalysisEngine::generateStatusReport() const {
    std::stringstream ss;
    ss << "===== SYSTEM STATUS REPORT (" << system.getSystemName() << ") =====\n";

    const auto& rooms = system.getRoomList();
    ss << "Total rooms monitored: " << rooms.size() << "\n";
    ss << "-------------------------------\n";

    for (const Room& room : rooms) {
        ss << "Room: " << room.getName() << "\n";

        std::string comfortText;
        switch (calculateRoomComfortCategory(room)) {
            case ComfortCategory::Ideal:      comfortText = "Ideal"; break;
            case ComfortCategory::Acceptable: comfortText = "Acceptable"; break;
            case ComfortCategory::TooHot:     comfortText = "Too Hot/Humid"; break;
            case ComfortCategory::TooCold:    comfortText = "Too Cold"; break;
            case ComfortCategory::NoData:     comfortText = "N/A (Missing Data)"; break;
        }

        if (room.getSensorList().empty()) {
            ss << "  - No sensors available in this room\n";
            ss << "  - Air Quality Score:     N/A\n";
            ss << "  - Comfort Score:         N/A\n";
            ss << "  - Safety Score:          N/A\n";
            continue;
        }

        ss << "  - Thermal Comfort:       " << comfortText << "\n";
        ss << "  - Acoustic Discomfort:   " << getRoomAcousticDiscomfort(room) << "\n";
        ss << "  - Luminous Discomfort:   " << getRoomLuminousDiscomfort(room) << "\n";
        ss << "  - Air Quality Score:     " << calculateAirQualityScore(room) << "/100\n";
        ss << "  - Comfort Score:         " << calculateComfortScore(room) << "/100\n";
        ss << "  - Safety Score:          " << calculateSafetyScore(room) << "/100\n";
        ss << "-------------------------------\n";
    }

    SystemIndices indices = calculateSystemIndices();
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
    int score = 100;

    for (const Alert& alert : generateAlerts()) {
        if (alert.getPriority() == 1) score -= 25;
        else if (alert.getPriority() == 2) score -= 10;
        else score -= 3;
    }

    return std::max(0, score);
}

SystemIndices AnalysisEngine::calculateSystemIndices() const {
    SystemIndices indices{0, 0, 0, 0, 0, "N/A", "N/A"};

    const auto& rooms = system.getRoomList();
    if (rooms.empty()) return indices;

    int noiseCount = 0, highNoise = 0;
    int lightCount = 0, badLight = 0;

    for (const Room& room : rooms) {
        switch (calculateRoomComfortCategory(room)) {
            case ComfortCategory::Ideal:indices.roomsIdeal++; break;
            case ComfortCategory::Acceptable:indices.roomsAcceptable++; break;
            case ComfortCategory::TooHot:indices.roomsHot++; break;
            case ComfortCategory::TooCold:indices.roomsCold++; break;
            case ComfortCategory::NoData:indices.roomsNoData++; break;
        }

        std::string acoustic = getRoomAcousticDiscomfort(room);
        if (acoustic != "N/A") {
            noiseCount++;
            if (acoustic == "High" || acoustic == "Moderate") highNoise++;
        }

        std::string luminous = getRoomLuminousDiscomfort(room);
        if (luminous != "N/A") {
            lightCount++;
            if (luminous == "Glare" || luminous == "Too Dark") badLight++;
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
