//
// Created by bunea on 26.10.2025.
//

#ifndef OOP_CAMERA_H
#define OOP_CAMERA_H


#include "Senzor.h"
#include <iostream>
#include <string>


class Camera{
private:
    std::string numeCamera;
    int nrSenzori;
    Senzor* listaSenzori;

public:
    Camera();

    Camera(const std::string& nume);

    ~Camera();

    Camera(const Camera& alta);

    Camera& operator=(const Camera& alta);

    void adaugSenzor(const Senzor& s);

    //Calculeaza media valorilor unui senzor
    double calculeazaMedieSenzori(const std::string& tipSenzor) const;

    //Afiseaza senzorii care depasesc o valoarea critica
    void afiseazaSenzoriPestePrag(double prag, const std::string& tipSenzor) const;

    std::string getNume() const;

    friend std::ostream& operator<<(std::ostream& os, const Camera& c);
};


#endif //OOP_CAMERA_H