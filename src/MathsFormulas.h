#ifndef MATHS_FORMULAS_H
#define MATHS_FORMULAS_H

#include <cmath>
#include <limits>

// MathsFormulas contains mathematical formulas used to verify if simulation is mathematically correct
// Usage: MathsFormulas::exp_population_without_mortality(...)

class MathsFormulas {
public:
    //*--------------------------------------------------------------------------------------
    //* Unique populations of rabbits case
    //*--------------------------------------------------------------------------------------

    // Exponential population, without mortality and no need of eat
    static float exp_population_without_mortality(float population_initiale, float taux_croissance, float temps) {
        return population_initiale * std::exp(taux_croissance * temps);
    }

    // Exponential population, with mortality and no need of eat
    static float exp_population_with_mortality(float population_initiale, float taux_croissance, float taux_mortalite, float temps) {
        return population_initiale * std::exp((taux_croissance - taux_mortalite) * temps);
        // return (taux_croissance / population_initiale ) / 1 + (((taux_croissance / population_initiale) - population_initiale) / population_initiale) * exp(-taux_mortalite * temps);
    }

    //*--------------------------------------------------------------------------------------
    //* Herbivore and carnivore population case
    //*--------------------------------------------------------------------------------------

    struct Populations
    {
        double herbivores;
        double carnivores;
    };

    struct LotkaVolterraRates
    {
        double herbivoreGrowth;
        double predation;
        double carnivoreMortality;
        double carnivoreGrowthFromPrey;
    };

    // dH/dt = H * (a - b*C), dC/dt = C * (d*H - c).
    static Populations lotka_volterra(Populations population, const LotkaVolterraRates& rates, double elapsed)
    {
        
        constexpr int MAX_STEPS = 16384;    //? Limite le nombre de pas de temps pour éviter les calculs trop longs.
        const double invalid = std::numeric_limits<double>::quiet_NaN();    //? Valeur de retour pour signaler 
                                                                            //? une divergence ou un problème numérique.


        if (!std::isfinite(population.herbivores) || !std::isfinite(population.carnivores) || !std::isfinite(elapsed) || elapsed < 0.0)
            return {invalid, invalid};

        if (elapsed == 0.0) return population;

        //& Cas sans interaction, calculables directement quel que soit le temps ecoule.
        if (population.herbivores == 0.0)
            return {0.0, population.carnivores * std::exp(-rates.carnivoreMortality * elapsed)};
        if (population.carnivores == 0.0)
            return {population.herbivores * std::exp(rates.herbivoreGrowth * elapsed), 0.0};

        //& Borne le travail en cas de saut de temps anormal. En jeu, elapsed vaut un pas.
        if (elapsed > MAX_STEPS) return {invalid, invalid};
        const int steps = static_cast<int>(std::ceil(elapsed));
        const double dt = elapsed / steps;


        for (int i = 0; i < steps; ++i)
        {
            //& Demi-pas des herbivores, pas des carnivores, demi-pas des herbivores.
            //& Chaque sous-pas garde l'autre population fixe et conserve la positivite.
            const double midHerbivores = population.herbivores * std::exp((rates.herbivoreGrowth - rates.predation * population.carnivores) * dt / 2.0);
            population.carnivores *= std::exp((rates.carnivoreGrowthFromPrey * midHerbivores - rates.carnivoreMortality) * dt);
            population.herbivores = midHerbivores * std::exp((rates.herbivoreGrowth - rates.predation * population.carnivores) * dt / 2.0);
            
            //& Si une population devient non finie, on considere que le modele diverge.
            if (!std::isfinite(population.herbivores) || !std::isfinite(population.carnivores))
                return {invalid, invalid};
        }
        return population;
    }

    //*--------------------------------------------------------------------------------------
    //* Informatic case
    //*--------------------------------------------------------------------------------------

    // TODO - Add informatic formula

};


#endif // MATHS_FORMULAS_H
