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
            {0, -1}, {0, 1}, {-1, 0}, {1, 0}
        };

        for (const auto& dir : directions) {
            sf::Vector2<int> adjacentPosition = position + dir;

            if (!grid.bounds(adjacentPosition))
            {
                continue;
            }
                
            Entity* adjacentEntity = grid.getCellAt(adjacentPosition)->getContent();

            if (adjacentEntity && dynamic_cast<Herbivore*>(adjacentEntity)) // if found another Herbivore
            { 
                
                std::vector<sf::Vector2<int>> emptyCells;

                // Find empty cells in the grid
                for (int32_t y = 0; y < grid.getRows(); ++y) 
                {
                    for (int32_t x = 0; x < grid.getCols(); ++x)
                    {
                        sf::Vector2<int> pos{x, y};
                        if (grid.getCellAt(pos)->isEmpty()) {
                            emptyCells.push_back(pos);
                        }
                    }
                }

                if (!emptyCells.empty()) 
                {
                    std::random_device rd;
                    std::mt19937 gen(rd());
                    std::uniform_int_distribution<> dis(0, emptyCells.size() - 1);
                    sf::Vector2<int> selectedPosition = emptyCells[dis(gen)];

                    Herbivore* newHerbivore = new Herbivore("New Herbivore");
                    grid.setCellAt(selectedPosition, newHerbivore);
                }
                return; // Stop loop
            }
        }
    }

    sf::Vector2<int> chooseDirection(Grid& grid) const override
    {
        std::vector<sf::Vector2<int>> directions = {
            {0, -1}, {0, 1}, {-1, 0}, {1, 0}
        };

        std::random_device rd;
        std::mt19937 gen(rd());
        std::shuffle(directions.begin(), directions.end(), gen);

        for (const auto& dir : directions) {
            sf::Vector2<int> newPosition = position + dir;
            if (grid.bounds(newPosition) && grid.getCellAt(newPosition)->isEmpty()) {
                return newPosition;
            }
        }

        return position; //invalid position, no valid move found
    }

};


#endif // HERBIVORE_H