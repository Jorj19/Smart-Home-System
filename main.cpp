#include <iostream>


#include "Senzor.h"
#include "Camera.h"
#include "SistemHomey.h"


int main() {

    // --- 1. Creare Senzori ---
    std::cout << "--- Creare Senzori... ---\n";
    Senzor tempLiving("Temperatura", 22.5, "C");
    Senzor co2Living("CO2", 450.0, "ppm");
    Senzor luminaLiving("Lumina", 300.0, "lux");

    Senzor tempDormitor("Temperatura", 19.0, "C");
    Senzor co2Dormitor("CO2", 650.0, "ppm");
    Senzor tempBucatarie("Temperatura", 25.0, "C"); // Valoare ridicată

    // --- 2. Creare Camera ---
    std::cout << "\n--- Creare Camera 'Living' si adaugare senzori... ---\n";
    Camera living("Living");
    living.adaugSenzor(tempLiving);
    living.adaugSenzor(co2Living);
    living.adaugSenzor(luminaLiving);

    // --- 3. Testare 'operator<<' pentru Camera ---
    std::cout << living;


    std::cout << "\n--- Testare functii netriviale 'Camera' ---\n";
    double medieTemp = living.calculeazaMedieSenzori("Temperatura");
    std::cout << "Media temperaturii in Living: " << medieTemp << " C\n";

    living.afiseazaSenzoriPestePrag(400.0, "CO2");
    living.afiseazaSenzoriPestePrag(500.0, "Lumina");


    std::cout << "\n--- Testare Constructor de Copiere (Camera) ---\n";

    std::cout << "CHECKPOINT 1\n";
    Camera livingCopie = living;

    std::cout << "CHECKPOINT 2\n";
    livingCopie.adaugSenzor(Senzor("Test", 1.0, "T"));
    std::cout << "--- ORIGINALUL (nu trebuie sa aiba senzorul 'Test') ---\n" << living;
    std::cout << "--- COPIA (trebuie sa aiba senzorul 'Test') ---\n" << livingCopie;

    std::cout << "\n--- Testare Operator= (Camera) ---\n";
    Camera birou("Birou");
    birou = living;
    std::cout << "--- BIROU (copiat din Living) ---\n" << birou;

    // --- 6. Creare SistemHomey si adaugare camere ---
    std::cout << "\n--- Creare Sistem 'The Homey' si adaugare camere... ---\n";
    SistemHomey casa("The Homey");
    casa.adaugCamere(living);

    Camera dormitor("Dormitor");
    dormitor.adaugSenzor(tempDormitor);
    dormitor.adaugSenzor(co2Dormitor);
    dormitor.adaugSenzor(tempBucatarie);

    casa.adaugCamere(dormitor);


    std::cout << "\n--- Afisare Sistem Homey Complet ---\n";
    std::cout << casa;


    std::cout << "\n--- Testare functii netriviale 'SistemHomey' ---\n";


    casa.gasesteCameraSenzorCritic("Temperatura", 24.0);


    // std::cout << "Hello, world!\n";
    // std::array<int, 100> v{};
    // int nr;
    // std::cout << "Introduceți nr: ";
    // /////////////////////////////////////////////////////////////////////////
    // /// Observație: dacă aveți nevoie să citiți date de intrare de la tastatură,
    // /// dați exemple de date de intrare folosind fișierul tastatura.txt
    // /// Trebuie să aveți în fișierul tastatura.txt suficiente date de intrare
    // /// (în formatul impus de voi) astfel încât execuția programului să se încheie.
    // /// De asemenea, trebuie să adăugați în acest fișier date de intrare
    // /// pentru cât mai multe ramuri de execuție.
    // /// Dorim să facem acest lucru pentru a automatiza testarea codului, fără să
    // /// mai pierdem timp de fiecare dată să introducem de la zero aceleași date de intrare.
    // ///
    // /// Pe GitHub Actions (bife), fișierul tastatura.txt este folosit
    // /// pentru a simula date introduse de la tastatură.
    // /// Bifele verifică dacă programul are erori de compilare, erori de memorie și memory leaks.
    // ///
    // /// Dacă nu puneți în tastatura.txt suficiente date de intrare, îmi rezerv dreptul să vă
    // /// testez codul cu ce date de intrare am chef și să nu pun notă dacă găsesc vreun bug.
    // /// Impun această cerință ca să învățați să faceți un demo și să arătați părțile din
    // /// program care merg (și să le evitați pe cele care nu merg).
    // ///
    // /////////////////////////////////////////////////////////////////////////
    // std::cin >> nr;
    // /////////////////////////////////////////////////////////////////////////
    // for(int i = 0; i < nr; ++i) {
    //     std::cout << "v[" << i << "] = ";
    //     std::cin >> v[i];
    // }
    // std::cout << "\n\n";
    // std::cout << "Am citit de la tastatură " << nr << " elemente:\n";
    // for(int i = 0; i < nr; ++i) {
    //     std::cout << "- " << v[i] << "\n";
    // }
    ///////////////////////////////////////////////////////////////////////////
    /// Pentru date citite din fișier, NU folosiți tastatura.txt. Creați-vă voi
    /// alt fișier propriu cu ce alt nume doriți.
    /// Exemplu:
    /// std::ifstream fis("date.txt");
    /// for(int i = 0; i < nr2; ++i)
    ///     fis >> v2[i];
    ///
    ///////////////////////////////////////////////////////////////////////////
    return 0;
}
