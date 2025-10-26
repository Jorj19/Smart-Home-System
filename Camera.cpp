//
// Created by bunea on 26.10.2025.
//

#include "Camera.h"
#include <iostream>

Camera::Camera() : numeCamera("N/A"), nrSenzori(0), listaSenzori(nullptr) {
}


//constructor de initializare
Camera::Camera(const std::string& nume) : numeCamera(nume), nrSenzori(0), listaSenzori(nullptr){

}

//destructor
Camera::~Camera(){
    delete[] this->listaSenzori;
    this->listaSenzori = nullptr;
}

//Constructor de copiere
Camera::Camera(const Camera& alta) : numeCamera(alta.numeCamera), nrSenzori(alta.nrSenzori){
    this->listaSenzori = new Senzor[this->nrSenzori];

    for (int i = 0; i < this->nrSenzori; i++){
        this->listaSenzori[i] = alta.listaSenzori[i];
    }
}


Camera& Camera::operator=(const Camera& alta) {
    if (this != &alta)
    {
        delete[] this->listaSenzori;

        this->numeCamera = alta.numeCamera;
        this->nrSenzori = alta.nrSenzori;

        this->listaSenzori = new Senzor[this->nrSenzori];

        for (int i = 0; i < this->nrSenzori; i++) {
            this->listaSenzori[i] = alta.listaSenzori[i];
        }

    }
    return *this;
}


void Camera::adaugSenzor(const Senzor& s) {
    Senzor* listaNoua = new Senzor[this->nrSenzori + 1];

    for (int i = 0; i < this->nrSenzori; i++) {
        listaNoua[i] = this->listaSenzori[i];
    }

    listaNoua[this->nrSenzori] = s;

    this->nrSenzori++;

    delete[] this->listaSenzori;

    this->listaSenzori = listaNoua;
}

double Camera::calculeazaMedieSenzori(const std::string& tipSenzor) const {
    double suma = 0.0;
    int count = 0;

    for (int i = 0; i < this->nrSenzori; i++) {
        if (this->listaSenzori[i].getTip() == tipSenzor) {
            suma += listaSenzori[i].getValoare();
            count++;
        }
    }

    if (count == 0) {
        return 0.0;
    }

    return suma/count;
}


void Camera::afiseazaSenzoriPestePrag(double prag, const std::string& tipSenzor) const {
    std::cout << "  Senzori tip [" << tipSenzor << "] peste pragul [" << prag << "] in " << this->numeCamera << ":\n";
    bool found = false;

    for (int i = 0; i < this->nrSenzori; i++) {
        if (this->listaSenzori[i].getTip() == tipSenzor && this->listaSenzori[i].getValoare() == prag) {
            found = true;
            std::cout << " -> " << this->listaSenzori[i];
        }
    }
    if (!found) {
        std::cout << " (niciunul)\n ";
    }
}


std::string Camera::getNume() const {
    return this->numeCamera;
}

std::ostream& operator<<(std::ostream& os, const Camera& c) {
    os << "Camera: " << c.getNume() << "( " << c.nrSenzori << " senzori ) \n";
    if (c.nrSenzori == 0) {
        os << "(Camera este goala)";
    }
    else {
        for (int i = 0; i < c.nrSenzori; i++) {
            os << " " << i+1 << ". " << c.listaSenzori[i] << std::endl;
        }
    }

    return os;
}
