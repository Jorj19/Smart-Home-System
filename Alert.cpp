
#include "Alert.h"
#include <utility>

int Alert::nextID = 1;

Alert::Alert(const std::string& msg, const std::string& room, int prio) : message(msg), roomName(room), priority(prio), alertID(nextID) {
    nextID++;
}

Alert::~Alert() {
    //std::cout << "Alert dest\n";
}

Alert::Alert(const Alert& other) : message(other.message), roomName(other.roomName), priority(other.priority), alertID(nextID) {
    nextID++;
}

Alert& Alert::operator=(const Alert& other) {
    if (this != &other) {
        this->message = other.message;
        this->roomName = other.roomName;
        this->priority = other.priority;
    }

    return *this;
}


// cppcheck-suppress unusedFunction
int Alert::getPriority() const {
    return this->priority;
}

// cppcheck-suppress unusedFunction
std::string Alert::getMessage() const {
    return this->message;
}

// cppcheck-suppress unusedFunction
int Alert::getID() const {
    return this->alertID;
}

std::ostream& operator<<(std::ostream& os, const Alert& a) {
    os << "[ID: " << a.alertID << ", Priority " << a.priority << "] Room: " << a.roomName
       << " -> Message: " << a.message;
    return os;
}
