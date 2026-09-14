#ifndef MATHS_FORMULAS_H
#define MATHS_FORMULAS_H

#include <cmath>

// MathsFormulas contains mathematical formulas used to verify if simulation is mathematically right
// Usage: MathsFormulas::exp_population_without_mortality(...)

class MathsFormulas {
public:
    //*--------------------------------------------------------------------------------------
    //* Unique populations of rabbits case
    //*--------------------------------------------------------------------------------------

    // Exponential population, without mortality and no need of eat
    static float exp_population_without_mortality(float population_initiale, float taux_croissance, float temps) {
        return population_initiale * exp(taux_croissance * temps);
    }

    // Exponential population, with mortality and no need of eat
    static float exp_population_with_mortality(float population_initiale, float taux_croissance, float taux_mortalite, float temps) {
        return (taux_croissance / population_initiale ) / 1 + (((taux_croissance / population_initiale) - population_initiale) / population_initiale) * exp(-taux_mortalite * temps);
    }

    //*--------------------------------------------------------------------------------------
    //* Herbivore and carnivore population case
    //*--------------------------------------------------------------------------------------

    // Lotka-Volterra equation for the herbivore population
    // The H population represents the prey (e.g. herbivores)
    // The C population represents the predators (e.g. carnivores)

    static float lotka_volterra(float population_herbivore, float population_carnivore, float taux_croissance_herbivore, float taux_mortalite_carnivore, float temps) {
        return population_herbivore * exp(taux_croissance_herbivore * temps) - (population_herbivore * population_carnivore * taux_mortalite_carnivore * temps);
    }

    //*--------------------------------------------------------------------------------------
    //* Informatic case
    //*--------------------------------------------------------------------------------------

    // TODO - Add informatic formula

};


#endif // MATHS_FORMULAS_H
