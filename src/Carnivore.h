#ifndef CARNIVORE_H
#define CARNIVORE_H

#include <string>

#include "Entity.h"
#include "Grid.h"
#include "Random.h"

class Carnivore : public Entity
{
public:
    static constexpr int MATURITY_AGE = 100;
    static constexpr int REPRODUCTION_COOLDOWN = 30;

    // Satiety
    static constexpr float MIN_SATIETY_LEVEL = 0.0f;
    static constexpr float MAX_SATIETY_LEVEL = 100.0f;
    static constexpr float SATIETY_CONSUME_THRESHOLD = 5.0f; // Above that value the carnivore can consume another entity
    static constexpr float SATIETY_GAIN_VALUE = 2.5f;
    static constexpr float SATIETY_LOSE_VALUE = 0.1f;

private:
    int age = 0;
    int cooldown = 0; // 0 = ready to reproduce
    float satietyLevel = MAX_SATIETY_LEVEL;

public:
    explicit Carnivore(std::string entityName = "Carnivore")
        : Entity(std::move(entityName))
    {
    }

    void update() override
    {
        ++age;
        if (cooldown > 0) --cooldown;

        loseSatiety(SATIETY_LOSE_VALUE); // Lose satiety each update
        if (satietyLevel == MIN_SATIETY_LEVEL) { // Dies to hunger
            std::cout << "[" << name << "]" << " Died to hunger." << std::endl;
            kill();
        }
    }

    //* ---- Rendering ----

    [[nodiscard]] Species species() const noexcept override { return Species::Carnivore; }

    [[nodiscard]] sf::Color color() const noexcept override
    {
        // The newer entity are clearer
        return isMature() ? sf::Color(236, 56, 90) : sf::Color(139, 0, 0);
    }

    //* ---- State ----

    [[nodiscard]] int getAge() const noexcept { return age; }
    void setAge(const int newAge) noexcept { age = newAge; }

    [[nodiscard]] bool isMature() const noexcept { return age >= MATURITY_AGE; }
    [[nodiscard]] bool canReproduce() const noexcept { return isMature() && cooldown == 0; }

    //* ---- Satiety Level Management ----

    void gainSatiety(const float value) {
        satietyLevel += value;

        if (satietyLevel > MAX_SATIETY_LEVEL) satietyLevel = MAX_SATIETY_LEVEL; // Clamping
    }

    void loseSatiety(const float value) {
        satietyLevel -= value;

        if (satietyLevel < MIN_SATIETY_LEVEL) satietyLevel = MIN_SATIETY_LEVEL; // Clamping
    }

    [[nodiscard]] bool isAbleToConsume() const { return satietyLevel >= SATIETY_CONSUME_THRESHOLD; }
    [[nodiscard]] float satietyRatio() const noexcept { return satietyLevel / MAX_SATIETY_LEVEL; } // 0.0 -> 1.0

    //* ---- Reproducing ----
    void reproduce(Grid& grid) override
    {
        if (!canReproduce()) return;

        for (const sf::Vector2i& direction : Grid::neighbourhood())
        {
            Entity* neighbour = grid.entityAt(position + direction);

            // Test enum d'abord : on n'appelle static_cast que si c'est bien un herbivore.
            if (neighbour == nullptr || neighbour->species() != Species::Carnivore) continue;

            auto* partner = dynamic_cast<Carnivore*>(neighbour);
            if (!partner->canReproduce()) continue;

            // Chance de reproduction = satiété moyenne du couple (0% affamés -> 100% rassasiés)
            const float reproductionChance = (satietyRatio() + partner->satietyRatio()) * 0.5f;
            const int roll = Random::inRange(0, 99);

            // Roll failed : Kill
            if (roll >= static_cast<int>(reproductionChance * 100.0f))
            {
                partner->kill();
                return;
            }

            // Reproduce
            const std::optional<sf::Vector2i> cradle = grid.randomFreeNeighbour(position);
            if (!cradle) return;

            if (grid.spawn<Carnivore>(*cradle, name + " Jr") != nullptr)
            {
                cooldown = REPRODUCTION_COOLDOWN;
                partner->cooldown = REPRODUCTION_COOLDOWN;
                return;
            }
        }
    }

    //* ---- CONSUME Herbivores -----
    void killingHerbivores(Grid& grid, const bool printKillLog) override {
        if (!isAbleToConsume()) return; // Satiety Level isn't high enough

        for (const sf::Vector2i& direction : Grid::neighbourhood())
        {
            Entity* neighbour = grid.entityAt(position + direction);

            // Verification if neighbour is a Herbivore
            if (neighbour == nullptr || neighbour->species() != Species::Herbivore) continue;

            auto* prey = dynamic_cast<Herbivore*>(neighbour);

            // Proceed Killing and gain satiety
            prey->kill();
            gainSatiety(SATIETY_GAIN_VALUE);

            if (printKillLog) // Killing Info
                std::cout << "[" << name << "]" << " > Killed a Herbivore. (" << prey->getName() << " / +" << SATIETY_GAIN_VALUE << "S)" << std::endl;

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