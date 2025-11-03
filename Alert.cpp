
#include "Alert.h"

Alert::Alert(const std::string& msg, const std::string& room, int prio) : message(msg), roomName(room), priority(prio) {

}

int Alert::getPriority() const {
    return this->priority;
}

std::string Alert::getMessage() const {
    return this->message;
}

std::ostream& operator<<(std::ostream& os, const Alert& a) {
    os << "[Priority " << a.priority << "] Room: " << a.roomName
       << " -> Message: " << a.message;
    return os;
}
