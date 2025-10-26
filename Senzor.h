//
// Created by bunea on 26.10.2025.
//

#ifndef OOP_SENZOR_H
#define OOP_SENZOR_H

#include <iostream>
#include <string>


class Senzor{
private:
    std::string tipSenzor;
    double valoareCurenta;
    std::string unitateMasura;

public:
    Senzor(const std::string& tip, double valoare, const std::string& unitate);

    Senzor();

    std::string getTip() const;
    double getValoare() const;
    std::string getUnitate() const;

    void actualizeazaValoare(double nouaValoare);

    friend std::ostream& operator<<(std::ostream& os, const Senzor& s);
};


#endif //OOP_SENZOR_H