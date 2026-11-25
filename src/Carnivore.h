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
    static constexpr int REPRODUCTION_COOLDOWN = 60;
    static constexpr int ELDERLY_THRESHOLD = 150;
    static constexpr int MAX_AGE = 250; // guarranted death

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
    bool isFemale = false; // Either a male or a female

public:
    explicit Carnivore(std::string entityName = "Carnivore")
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

        loseSatiety(SATIETY_LOSE_VALUE); // Lose satiety each update
        if (satietyLevel == MIN_SATIETY_LEVEL) { // Dies to hunger
            kill();
        }

        if (age >= ELDERLY_THRESHOLD) rollDeath();
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

    [[nodiscard]] bool isAFemale() const { return isFemale; }

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
            if (neighbour == nullptr || neighbour->species() != Species::Carnivore) continue;

            auto* partner = dynamic_cast<Carnivore*>(neighbour);
            if (!partner->canReproduce()) continue;
            if (isAFemale() == partner->isAFemale()) continue;

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