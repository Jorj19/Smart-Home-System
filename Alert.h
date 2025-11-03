
#ifndef OOP_ALERT_H
#define OOP_ALERT_H

#include <iostream>
#include <string>


class Alert {
private:
    std::string message;
    std::string roomName;
    int priority; // 1 = critic, 2 = avertisment, 3 = info

public:
    Alert(const std::string& msg, const std::string& room, int prio);

    [[nodiscard]] int getPriority() const;
    [[nodiscard]] std::string getMessage() const;

    friend std::ostream& operator<<(std::ostream& os, const Alert& a);
};


#endif //OOP_ALERT_H