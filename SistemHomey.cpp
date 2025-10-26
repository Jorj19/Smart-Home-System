//
// Created by bunea on 26.10.2025.
//

#include "SistemHomey.h"

#include <iostream>

SistemHomey::SistemHomey(const std::string& nume) : numeSistem(nume), nrCamere(0), listaCamere(nullptr) {}

SistemHomey::~SistemHomey() {
    delete[] this->listaCamere;
    this->listaCamere = nullptr;
}

SistemHomey::SistemHomey(const SistemHomey& altul) : numeSistem(altul.numeSistem), nrCamere(altul.nrCamere){
    this->listaCamere = new Camera[this->nrCamere];
    for (int i = 0; i < this->nrCamere; i++) {
        this->listaCamere[i] = altul.listaCamere[i];
    }
}

SistemHomey& SistemHomey::operator=(const SistemHomey& altul) {
    if (this != &altul) {
        delete[] this->listaCamere;
        this->numeSistem = altul.numeSistem;
        this->nrCamere = altul.nrCamere;
        this->listaCamere = new Camera[this->nrCamere];
        for (int i = 0; i < this->nrCamere; i++) {
            this->listaCamere[i] = altul.listaCamere[i];
        }
    }
    return *this;
}

void SistemHomey::adaugCamere(const Camera& c) {
    Camera* listaNoua = new Camera[this->nrCamere + 1];
    for (int i = 0; i < this->nrCamere; i++) {
        listaNoua[i] = this->listaCamere[i];
    }
    listaNoua[this->nrCamere] = c;
    this->nrCamere++;
    delete[] this->listaCamere;
    this->listaCamere = listaNoua;
}

void SistemHomey::gasesteCameraSenzorCritic(const std::string& tipSenzor, double prag) const {
    std::cout << "\n>>> Cautare Senzori Critici (" << tipSenzor << " > " << prag << ") in sistemul " << this->numeSistem << "\n";
    for(int i=0; i < this->nrCamere; i++) {
        this->listaCamere[i].afiseazaSenzoriPestePrag(prag, tipSenzor);
    }
}

std::ostream& operator<<(std::ostream& os, const SistemHomey& sh) {
    os << "#########################################\n";
    os << "Sistem Homey: " << sh.numeSistem << " (Total " << sh.nrCamere << " camere)\n";
    os << "#########################################\n";
    for (int i = 0; i < sh.nrCamere; i++) {
        os << sh.listaCamere[i] << "\n";
    }
    return os;
}