#include <fstream>
#include <iostream>

#include "Application.h"
#include "Grid.h"
#include "Herbivore.h"
#include "Carnivore.h"
#include "tools.h"

namespace //Variables container
{
    constexpr int WINDOW_WIDTH = 1000;
    constexpr int WINDOW_HEIGHT = 1000;
    constexpr int INITIAL_HERBIVORES = 0;
    constexpr int INITIAL_CARNIVORES = 100;
    constexpr float TICKS_PER_SECOND = 100.f;

    void seedPopulation(Grid& grid, int h_count, int c_count)
    {
        // Herbivores
        int h_placed = 0;
        int h_guard = 0;

        while (h_placed < h_count && h_guard < h_count * 50)
        {
            ++h_guard;

            const sf::Vector2i position{Random::inRange(0, grid.getCols() - 1), Random::inRange(0, grid.getRows() - 1)};

            auto* herbivore = grid.spawn<Herbivore>(position, "H" + std::to_string(h_placed));
            if (herbivore == nullptr)
            {
                continue;
            }


            herbivore->setAge(Random::inRange(Herbivore::MATURITY_AGE, Herbivore::MATURITY_AGE + 20));
            ++h_placed;
        }

        // Carnivores
        int c_placed = 0;
        int c_guard = 0;

        while (c_placed < c_count && c_guard < c_count * 50)
        {
            ++c_guard;

            const sf::Vector2i position{Random::inRange(0, grid.getCols() - 1), Random::inRange(0, grid.getRows() - 1)};

            auto* carnivore = grid.spawn<Carnivore>(position, "C" + std::to_string(c_placed));
            if (carnivore == nullptr)
            {
                continue;
            }


            carnivore->setAge(Random::inRange(Carnivore::MATURITY_AGE, Carnivore::MATURITY_AGE + 20));
            ++c_placed;
        }
    }
}

int main()
{
    Grid grid(WINDOW_WIDTH, WINDOW_HEIGHT);
    seedPopulation(grid, INITIAL_HERBIVORES, INITIAL_CARNIVORES);

    Application app(grid, TICKS_PER_SECOND);
    app.setFPSMax(10); // Optimal
    app.run();

    return 0;
}