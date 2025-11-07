
#ifndef OOP_ALERT_H
#define OOP_ALERT_H

#include <iostream>
#include <string>


class Alert {
private:
    std::string message;
    std::string roomName;
    int priority; // 1 = critic, 2 = avertisment, 3 = info

    const int alertID;
    static int nextID;

public:
    Alert(const std::string& msg, const std::string& room, int prio);

    ~Alert();

    Alert(const Alert& other);

    Alert& operator=(const Alert& other);

    [[nodiscard]] int getPriority() const;
    [[nodiscard]] std::string getMessage() const;
    [[nodiscard]] int getID() const;

    friend std::ostream& operator<<(std::ostream& os, const Alert& a);
};


#endif //OOP_ALERT_H