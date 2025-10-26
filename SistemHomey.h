//
// Created by bunea on 26.10.2025.
//

#ifndef OOP_SISTEMHOMEY_H
#define OOP_SISTEMHOMEY_H

#include "Camera.h"
#include <iostream>
#include <string>

class SistemHomey {
private:
    std::string numeSistem;
    int nrCamere;
    Camera* listaCamere;

public:
    SistemHomey(const std::string& nume);

    ~SistemHomey();

    SistemHomey(const SistemHomey& altul);

    SistemHomey& operator=(const SistemHomey& altul);

    void adaugCamere(const Camera& c);

    void gasesteCameraSenzorCritic(const std::string& tipSenzor, double prag) const;

    friend std::ostream& operator<<(std::ostream& os, const SistemHomey& sh);
};


#endif //OOP_SISTEMHOMEY_H