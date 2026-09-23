#include <fstream>
#include <iostream>

#include "Application.h"
#include "Grid.h"
#include "Herbivore.h"
#include "tools.h"

namespace //Variables container
{
    constexpr int WINDOW_WIDTH = 5000;
    constexpr int WINDOW_HEIGHT = 5000;
    constexpr int INITIAL_HERBIVORES = 1000;
    constexpr float TICKS_PER_SECOND = 150.f;

    void seedPopulation(Grid& grid, int count)
    {
        int placed = 0;
        int guard = 0;

        while (placed < count && guard < count * 50)
        {
            ++guard;

            const sf::Vector2i position{Random::inRange(0, grid.getCols() - 1), Random::inRange(0, grid.getRows() - 1)};

            Herbivore* herbivore = grid.spawn<Herbivore>(position, "H" + std::to_string(placed));
            if (herbivore == nullptr)
            {
                continue;
            }
            

            herbivore->setAge(Random::inRange(Herbivore::MATURITY_AGE, Herbivore::MATURITY_AGE + 20));
            ++placed;

        }
    }
}

int main()
{
    Grid grid(WINDOW_WIDTH, WINDOW_HEIGHT);
    seedPopulation(grid, INITIAL_HERBIVORES);

    Application app(grid, TICKS_PER_SECOND);
    app.setFPSMax(10); // Optimal
    app.run();

    return 0;
}