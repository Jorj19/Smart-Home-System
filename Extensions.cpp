//
// Created by bunea on 12.01.2026.
//

#include "Extensions.h"
#include "Sensor.h"

#include <algorithm>
#include <cctype>


std::shared_ptr<Sensor> sensorFactory::createSensor(int typeIndex, double value) {
    switch(typeIndex) {
    case 1: return std::make_shared<temperatureSensor>(value);
    case 2: return std::make_shared<humiditySensor>(value);
    case 3: return std::make_shared<lightSensor>(value);
    case 4: return std::make_shared<toxicGasSensor>("CO2", value, "ppm");
    case 5: return std::make_shared<particleSensor>(value);
    case 6: return std::make_shared<smokeSensor>(value);
    case 7: return std::make_shared<toxicGasSensor>("TVOC", value, "ppb");
    case 8: return std::make_shared<toxicGasSensor>("CO", value, "ppm");
    case 9: return std::make_shared<soundSensor>(value);
    default:
        return nullptr;
    }
}


std::shared_ptr<Sensor> sensorFactory::createSensorFromString(const std::string& rawType, double value) {
    std::string type = rawType;
    std::ranges::transform(type.begin(), type.end(), type.begin(),
                   [](unsigned char c){ return std::tolower(c); });

    int id = 0;

    // Mapare string -> ID
    if (type == "temperatura" || type == "temperature") id = 1;
    else if (type == "umiditate" || type == "humidity") id = 2;
    else if (type == "lumina" || type == "light") id = 3;
    else if (type == "co2") id = 4;
    else if (type == "pm2.5" || type == "pm25") id = 5;
    else if (type == "fum" || type == "smoke") id = 6;
    else if (type == "tvoc") id = 7;
    else if (type == "co") id = 8;
    else if (type == "sunet" || type == "sound") id = 9;

    if (id != 0) {
        return createSensor(id, value);
    }

    return nullptr;
}

// cppcheck-suppress unusedFunction
std::string sensorFactory::getSensorName(int typeIndex) {
    switch(typeIndex) {
    case 1: return "Temperatura";
    case 2: return "Umiditate";
    case 3: return "Lumina";
    case 4: return "CO2";
    case 5: return "PM2.5";
    case 6: return "Fum";
    case 7: return "TVOC";
    case 8: return "CO";
    case 9: return "Sunet";
    default: return "Unknown";
    }
}

