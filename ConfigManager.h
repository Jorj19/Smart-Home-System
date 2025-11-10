//
// Created by bunea on 10.11.2025.
//

#ifndef OOP_CONFIGMANAGER_H
#define OOP_CONFIGMANAGER_H
#include "HomeSystem.h"
#include <string>


class ConfigManager {
public:
    static HomeSystem loadSystemFromFile(const std::string& filename);

    static bool saveSystemToFile(const std::string& filename, const HomeSystem& system);
};


#endif //OOP_CONFIGMANAGER_H