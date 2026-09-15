#ifndef HERBIVORE_H
#define HERBIVORE_H

#include "Entity.h"
#include "Grid.h"
#include <random>

class Herbivore : public Entity {

    

public:
    explicit Herbivore(const std::string& name)
        : Entity(name)
    {
    }

    explicit Herbivore()
        : Entity() {

    }

    void update() override
    {
        // Comportement spécifique à Herbivore
    }

    void reproduce(Grid& grid) override
    {
        std::vector<sf::Vector2<int>> directions = {
            {0, -1},  // Up
            {0, 1},   // Down
            {-1, 0},  // Left
            {1, 0}    // Right
        };

        for (const auto& dir : directions) {
            
            // Check if an other herbivore is in the adjacent cell
            sf::Vector2<int> adjacentPosition = position + dir;
            if (grid.bounds(adjacentPosition)) {
                Cell* adjacentCell = grid.getCellAt(adjacentPosition);
                Entity* adjacentEntity = adjacentCell->getContent();
                if (adjacentEntity && dynamic_cast<Herbivore*>(adjacentEntity)) {
                    // Found another herbivore, try to reproduce
                    for (const auto& dir2 : directions) {
                        sf::Vector2<int> newPosition = position + dir2;
                        if (grid.bounds(newPosition) && grid.getCellAt(newPosition)->isEmpty()) {
                            Herbivore* newHerbivore = new Herbivore("NewHerbivore");
                            grid.setCellAt(newPosition, newHerbivore);
                            return; // Reproduction successful, exit the function
                        }
                    }
                }
            }
        }
    }

    sf::Vector2<int> chooseDirection(Grid& grid) const override
    {
        // Direction vector
        std::vector<sf::Vector2<int>> directions = {
            {0, -1},  // Up
            {0, 1},   // Down
            {-1, 0},  // Left
            {1, 0}    // Right
        };

        // Random shuffle direction
        std::random_device rd;
        std::mt19937 gen(rd());
        std::shuffle(directions.begin(), directions.end(), gen);

        for (const auto& dir : directions) {
            sf::Vector2<int> newPosition = position + dir;
            if (grid.bounds(newPosition)) {
                return newPosition;
            }
        }

        return position; // No valid move found, stay in place
    }

};


#endif // HERBIVORE_H