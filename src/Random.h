#ifndef RANDOM_H
#define RANDOM_H

#include <algorithm>
#include <random>


//** @brief class for generate random numbers, with a single random engine for the whole project


namespace Random
{
    inline std::mt19937& engine()
    {
        static std::mt19937 gen{std::random_device{}()};
        return gen;
    }

    inline int inRange(int min, int max)
    {
        std::uniform_int_distribution<int> dist(min, max);
        return dist(engine());
    }

    inline float unit()
    {
        std::uniform_real_distribution<float> dist(0.f, 1.f);
        return dist(engine());
    }

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
