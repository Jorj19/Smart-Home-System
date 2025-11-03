//
// Created by bunea on 03.11.2025.
//

#ifndef OOP_ANALYSISENGINE_H
#define OOP_ANALYSISENGINE_H

#include "HomeSystem.h"
#include "Alert.h"

#include <vector>
#include <string>

class AnalysisEngine {
private:
    const HomeSystem& system;

public:
    AnalysisEngine(const HomeSystem& sys);

    std::vector<Alert> generateAlerts() const;

    std::string generateStatusReport() const;

    int calculateHealthScore() const;
};


#endif //OOP_ANALYSISENGINE_H