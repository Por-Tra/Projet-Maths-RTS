#ifndef MATHEMATIQUE_SIDE_H
#define MATHEMATIQUE_SIDE_H

#include <cmath>

//*--------------------------------------------------------------------------------------
//* Cas unique de population de lapin par exemple
//*--------------------------------------------------------------------------------------

// Formule 1
// Population exponentielle, sans mortalité et sans besoins de nourrir
float Formule1(float population_initiale, float taux_croissance, float temps) {
    return population_initiale * exp(taux_croissance * temps);
}

// Formule 2
// Population exponentielle, avec mortalité et sans besoins de nourrir
float Formule2(float population_initiale, float taux_croissance, float taux_mortalite, float temps) {
    return (taux_croissance / population_initiale ) / 1 + (((taux_croissance / population_initiale) - population_initiale) / population_initiale) * exp(-taux_mortalite * temps);
}


//*--------------------------------------------------------------------------------------
//* Cas de population herbivore et carnivore
//*--------------------------------------------------------------------------------------

// Formule 3
// formule de Lotka-Volterra pour la population d'herbivores
// La population H des proies (par exemple des herbivores)
// La population C des prédateurs (par exemple des carnivores)

float Formule3(float population_herbivore, float population_carnivore, float taux_croissance_herbivore, float taux_mortalite_carnivore, float temps) {
    return population_herbivore * exp(taux_croissance_herbivore * temps) - (population_herbivore * population_carnivore * taux_mortalite_carnivore * temps);
}


//*--------------------------------------------------------------------------------------
//* Cas informatique
//*--------------------------------------------------------------------------------------


#endif // MATHEMATIQUE_SIDE_H
