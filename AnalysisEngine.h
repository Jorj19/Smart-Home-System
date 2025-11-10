//
// Created by bunea on 03.11.2025.
//

#ifndef OOP_ANALYSISENGINE_H
#define OOP_ANALYSISENGINE_H

#include "HomeSystem.h"
#include "Alert.h"
#include "Rule.h"

#include <vector>
#include <string>

enum class ComfortCategory {
    NoData = -1,
    TooCold = -2,
    Ideal = 0,
    Acceptable = 1,
    TooHot = 2
};

struct SystemIndices {
    int roomsIdeal;
    int roomsAcceptable;
    int roomsHot;
    int roomsCold;
    int roomsNoData;
    std::string overallAcousticDiscomfort;
    std::string overallLuminousDiscomfort;
};

class AnalysisEngine {
private:
    const HomeSystem& system;
    std::vector<Rule> ruleList;

    void loadRulesFromFile(const std::string& filename);

    [[nodiscard]] static ComfortCategory calculateRoomComfortCategory(const Room& room);
    [[nodiscard]] static std::string getRoomAcousticDiscomfort(const Room& room);
    [[nodiscard]] static std::string getRoomLuminousDiscomfort(const Room& room);

public:
    explicit AnalysisEngine(const HomeSystem& sys, const std::string& rulesFilename);

    void addRule(const Rule& rule);

    [[nodiscard]] const std::vector<Rule>& getRuleList() const;

    [[nodiscard]] std::vector<Alert> generateAlerts() const;

    [[nodiscard]] std::string generateStatusReport() const;

    [[nodiscard]] int calculateHealthScore() const;

    [[nodiscard]] SystemIndices calculateSystemIndices() const;
};


#endif //OOP_ANALYSISENGINE_H