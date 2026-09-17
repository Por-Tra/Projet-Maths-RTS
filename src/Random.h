#ifndef RANDOM_H
#define RANDOM_H

#include <algorithm>
#include <random>

//* Un SEUL moteur aleatoire pour toute la simulation.
//* Avant : `std::random_device rd; std::mt19937 gen(rd());` etait recree a chaque
//* appel de chooseDirection() / reproduce(). Or construire un mt19937 initialise
//* 2.5 Ko d'etat interne, et random_device peut taper dans le systeme (/dev/urandom).
//* Fait 60 fois par seconde pour chaque entite, c'est un des plus gros couts du code.
namespace Random
{
    inline std::mt19937& engine()
    {
        static std::mt19937 gen{std::random_device{}()};
        return gen;
    }

    // Entier uniforme dans [min, max] (bornes incluses)
    inline int inRange(int min, int max)
    {
        std::uniform_int_distribution<int> dist(min, max);
        return dist(engine());
    }

    // Reel uniforme dans [0, 1)
    inline float unit()
    {
        std::uniform_real_distribution<float> dist(0.f, 1.f);
        return dist(engine());
    }

    // Vrai avec une probabilite donnee (0.0 -> jamais, 1.0 -> toujours)
    inline bool chance(float probability)
    {
        return unit() < probability;
    }

    template <typename Container>
    void shuffle(Container& container)
    {
        std::shuffle(std::begin(container), std::end(container), engine());
    }
}

#endif // RANDOM_H
