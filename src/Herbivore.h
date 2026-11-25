#ifndef HERBIVORE_H
#define HERBIVORE_H

#include <string>

#include "Entity.h"
#include "Grid.h"

class Herbivore : public Entity
{
public:
    static constexpr int MATURITY_AGE = 30;
    static constexpr int REPRODUCTION_COOLDOWN = 15;
    static constexpr int ELDERLY_THRESHOLD = 65;
    static constexpr int MAX_AGE = 100; // guarranted death

private:
    int age = 0;
    int cooldown = 0; // 0 = ready to reproduce
    bool isFemale = false; // Either a male or a female

public:
    explicit Herbivore(std::string entityName = "Herbivore")
        : Entity(std::move(entityName))
    {
        // Rolling Male / Female
        const int maleFemaleRandom = Random::inRange(0, 1);
        isFemale = maleFemaleRandom == 1;
    }

    void update() override
    {
        ++age;
        if (cooldown > 0) --cooldown;
        if (age >= ELDERLY_THRESHOLD) rollDeath();
    }

    //* ---- Rendering ----

    [[nodiscard]] Species species() const noexcept override { return Species::Herbivore; }

    [[nodiscard]] sf::Color color() const noexcept override
    {
        // The newer entity are clearer
        return isMature() ? sf::Color(60, 200, 60) : sf::Color(160, 240, 160);
    }

    //* ---- State ----

    [[nodiscard]] int getAge() const noexcept { return age; }
    void setAge(const int newAge) noexcept { age = newAge; }

    [[nodiscard]] bool isMature() const noexcept { return age >= MATURITY_AGE; }
    [[nodiscard]] bool canReproduce() const noexcept { return isMature() && cooldown == 0; }

    [[nodiscard]] bool isAFemale() const { return isFemale; }

    void rollDeath()
    {
        // Proportion d'âge "vécu" au-delà du seuil : 0.0 au seuil -> 1.0 à MAX_AGE
        const float elderlyProgress =
            static_cast<float>(age - ELDERLY_THRESHOLD) /
            static_cast<float>(MAX_AGE - ELDERLY_THRESHOLD);

        // Probabilité de mourir à ce cycle, bornée entre 0 et 1
        const float deathChance = std::clamp(elderlyProgress, 0.0f, 1.0f);

        // Tirage sur 0..999 pour avoir une précision de 0.1 %
        const int roll = Random::inRange(0, 999);
        if (static_cast<float>(roll) < deathChance * 1000.0f)
        {
            kill();
        }
    }

    //* ---- Reproducing ----
    void reproduce(Grid& grid) override
    {
        if (!canReproduce()) return;

        for (const sf::Vector2i& direction : Grid::neighbourhood())
        {
            Entity* neighbour = grid.entityAt(position + direction);

            // Test enum d'abord : on n'appelle static_cast que si c'est bien un herbivore.
            if (neighbour == nullptr || neighbour->species() != Species::Herbivore) continue;

            auto* partner = dynamic_cast<Herbivore*>(neighbour);
            if (!partner->canReproduce()) continue;
            if (isAFemale() == partner->isAFemale()) continue;

            const std::optional<sf::Vector2i> cradle = grid.randomFreeNeighbour(position);
            if (!cradle) return; // pas de place autour : ce sera pour un autre tour

            if (grid.spawn<Herbivore>(*cradle, name + " Jr") != nullptr)
            {
                cooldown = REPRODUCTION_COOLDOWN;
                partner->cooldown = REPRODUCTION_COOLDOWN;
            }
            return;
        }
    }

    void killingHerbivores(Grid& grid, const bool printKillLog) override { } // USED IN CARNIVORE - PROB SHOULDN'T BE DONE THAT WAY

protected:
    sf::Vector2i chooseDirection(Grid& grid) const override
    {
        const std::optional<sf::Vector2i> target = grid.randomFreeNeighbour(position);
        return target.value_or(position); // aucune case libre -> on reste sur place
    }
};

#endif // HERBIVORE_H