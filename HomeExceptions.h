
#ifndef OOP_HOMEEXCEPTIONS_H
#define OOP_HOMEEXCEPTIONS_H
#include <exception>
#include <string>

class HomeExceptions : public std::exception {
protected:
    std::string message;
public:
    explicit HomeExceptions(std::string msg);

    [[nodiscard]] const char* what() const noexcept override;
};

//Erori legate de fisiere
class FileConfigException : public HomeExceptions {
public:
    explicit FileConfigException(const std::string& filename);
};

//Erori legate de datele senzorilor
class InvalidDataSensorException : public HomeExceptions {
public:
    explicit InvalidDataSensorException(const std::string& details);
};

//Erori legate de motorul de reguli
class RuleEngineException : public HomeExceptions {
public:
    explicit RuleEngineException(const std::string& ruleDetails);
};

//Duplicat
class DuplicateEntityException : public HomeExceptions {
public:
    explicit DuplicateEntityException(const std::string& entityName);
};

//Camera inexista
class RoomNotFoundException : public HomeExceptions {
public:
    explicit RoomNotFoundException(const std::string& roomName);
};

// Valori imposibile
class SensorOutOfRangeException : InvalidDataSensorException {
public:
    SensorOutOfRangeException(const std::string& sensorType, double value);
};
#endif //OOP_HOMEEXCEPTIONS_H