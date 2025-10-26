#include "Senzor.h"

Senzor::Senzor(const std::string& tip, double valoare, const std::string& unitate) : tipSenzor(tip), valoareCurenta(valoare), unitateMasura(unitate){
    //std::cout << "Constructor Senzor cu parametrii apelat" << std::endl;

}

Senzor::Senzor(): tipSenzor("Necunoscut"), valoareCurenta(0.0), unitateMasura("N/A"){

}

std::string Senzor::getTip() const{
    return this->tipSenzor;
}

double Senzor::getValoare() const{
    return this->valoareCurenta;
}

std::string Senzor::getUnitate() const{
    return this->unitateMasura;
}

void Senzor::actualizeazaValoare(double nouaValoare){
    this->valoareCurenta = nouaValoare;
}

std::ostream& operator<<(std::ostream& os, const Senzor& s){
    os << "Senzor [Tip: " << s.tipSenzor << ",Valoare: " << s.valoareCurenta << " " << s.unitateMasura << "]";
    return os;
}