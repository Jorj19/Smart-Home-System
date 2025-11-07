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

class AnalysisEngine {
private:
    const HomeSystem& system;
    std::vector<Rule> ruleList;

public:
    explicit AnalysisEngine(const HomeSystem& sys);

    void addRule(const Rule& rule);

    [[nodiscard]] const std::vector<Rule>& getRuleList() const;

    [[nodiscard]] std::vector<Alert> generateAlerts() const;

    [[nodiscard]] std::string generateStatusReport() const;

    [[nodiscard]] int calculateHealthScore() const;
};


#endif //OOP_ANALYSISENGINE_H