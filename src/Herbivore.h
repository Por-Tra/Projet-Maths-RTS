#ifndef HERBIVORE_H
#define HERBIVORE_H

#include <string>

#include "Entity.h"
#include "Grid.h"
#include "Random.h"

class Herbivore : public Entity
{
public:
    static constexpr int MATURITY_AGE = 30;
    static constexpr int REPRODUCTION_COOLDOWN = 30;
    static constexpr int MAX_AGE = 300; // too old -> dies

private:
    int age = 0;
    int cooldown = 0; // 0 = ready to reproduce

public:
    explicit Herbivore(std::string entityName = "Herbivore")
        : Entity(std::move(entityName))
    {
    }

    //* Identite / rendu
    //* -----------------------------------------------------------------------------------------------

    Species species() const noexcept override { return Species::Herbivore; }

    sf::Color color() const noexcept override
    {
        // Les jeunes sont plus clairs : lisible d'un coup d'oeil a l'ecran.
        return isMature() ? sf::Color(200, 60, 60) : sf::Color(240, 160, 160);
    }

    //* Etat
    //* -----------------------------------------------------------------------------------------------

    int getAge() const noexcept { return age; }
    void setAge(int newAge) noexcept { age = newAge; }

    bool isMature() const noexcept { return age >= MATURITY_AGE; }
    bool canReproduce() const noexcept { return isMature() && cooldown == 0; }

    void update() override
    {
        ++age;
        if (cooldown > 0) --cooldown;
        if (age >= MAX_AGE) kill();
    }

    //* Reproduction
    //* -----------------------------------------------------------------------------------------------
    //* Ancienne version : la variable `attempt` ne servait a rien (jamais < 0 en 4 tours),
    //* un `return` premature sortait de la boucle des qu'un voisin n'etait pas pret,
    //* et surtout le bebe pouvait apparaitre N'IMPORTE OU dans la grille apres un scan
    //* complet de toutes les cases. Ici : partenaire adjacent + bebe adjacent, O(4).
    void reproduce(Grid& grid) override
    {
        if (!canReproduce()) return;

        for (const sf::Vector2i& direction : Grid::neighbourhood())
        {
            Entity* neighbour = grid.entityAt(position + direction);

            // Test enum d'abord : on n'appelle static_cast que si c'est bien un herbivore.
            if (neighbour == nullptr || neighbour->species() != Species::Herbivore) continue;

            Herbivore* partner = static_cast<Herbivore*>(neighbour);
            if (!partner->canReproduce()) continue;

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

protected:
    //* Une seule case libre tiree au sort parmi les voisines.
    //* Avant : un std::vector de directions alloue + std::shuffle + un mt19937 reseme,
    //* a chaque entite et a chaque tick.
    sf::Vector2i chooseDirection(Grid& grid) const override
    {
        const std::optional<sf::Vector2i> target = grid.randomFreeNeighbour(position);
        return target.value_or(position); // aucune case libre -> on reste sur place
    }
};

#endif // HERBIVORE_H