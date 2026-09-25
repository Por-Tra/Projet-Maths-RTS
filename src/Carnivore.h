#ifndef CARNIVORE_H
#define CARNIVORE_H

#include <string>
#include <cmath>

#include "Entity.h"
#include "Grid.h"
#include "Random.h"



class Carnivore : public Entity
{
public:
    // 
    static constexpr int MATURITY_AGE = 100;
    static constexpr int REPRODUCTION_COOLDOWN = 30;
    static constexpr float DEATH_PROBABILITY = 1.5f; // 0.3% Chance to die each cycle.

private:
    int age = 0;
    int cooldown = 0; // 0 = ready to reproduce

public:
    explicit Carnivore(std::string entityName = "Carnivore")
        : Entity(std::move(entityName))
    {
    }

    //* ---- Rendering ----

    Species species() const noexcept override { return Species::Carnivore; }

    sf::Color color() const noexcept override
    {
        // The newer entity are clearer
        return isMature() ? sf::Color(236, 56, 90) : sf::Color(139, 0, 0);
    }

    //* ---- State ----

    int getAge() const noexcept { return age; }
    void setAge(int newAge) noexcept { age = newAge; }

    bool isMature() const noexcept { return age >= MATURITY_AGE; }
    bool canReproduce() const noexcept { return isMature() && cooldown == 0; }

    void update() override
    {
        ++age;
        if (cooldown > 0) --cooldown;

        // Roll death
        if (age >= MATURITY_AGE) {

            float randomisedNumber = Random::inRangef(1.0f, 100.0f);
            std::cout << "Randomised Number: " << randomisedNumber ;

            if (randomisedNumber <= DEATH_PROBABILITY) {
                kill();
                std::cout << " Mourir";
            }

            std::cout << std::endl;
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

            Carnivore* partner = static_cast<Carnivore*>(neighbour);
            if (!partner->canReproduce()) continue;

            const std::optional<sf::Vector2i> cradle = grid.randomFreeNeighbour(position);
            if (!cradle) return; // pas de place autour : ce sera pour un autre tour

            if (grid.spawn<Carnivore>(*cradle, name + " Jr") != nullptr)
            {
                cooldown = REPRODUCTION_COOLDOWN;
                partner->cooldown = REPRODUCTION_COOLDOWN;
            }
            return;
        }
    }

protected:
    sf::Vector2i chooseDirection(Grid& grid) const override
    {
        const std::optional<sf::Vector2i> target = grid.randomFreeNeighbour(position);
        return target.value_or(position); // aucune case libre -> on reste sur place
    }
};

#endif // CARNIVORE_H