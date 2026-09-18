#include <fstream>
#include <iostream>
#include <string>

#include "Application.h"
#include "Grid.h"
#include "Herbivore.h"
#include "tools.h"

namespace
{
    constexpr int WINDOW_WIDTH = 5000;
    constexpr int WINDOW_HEIGHT = 5000;
    constexpr int INITIAL_HERBIVORES = 100;
    constexpr float TICKS_PER_SECOND = 150.f;

    void seedPopulation(Grid& grid, int count)
    {
        int placed = 0;
        int guard = 0;

        while (placed < count && guard < count * 50)
        {
            ++guard;

            const sf::Vector2i position{Random::inRange(0, grid.getCols() - 1),
                                        Random::inRange(0, grid.getRows() - 1)};

            Herbivore* herbivore = grid.spawn<Herbivore>(position, "H" + std::to_string(placed));
            if (herbivore == nullptr) continue; // case deja prise, on retente

            herbivore->setAge(Random::inRange(Herbivore::MATURITY_AGE, Herbivore::MATURITY_AGE + 20));
            ++placed;
        }
    }

    //* Mode sans fenetre : utile pour produire le CSV vite et comparer aux
    //* formules de MathsFormulas.h. Possible uniquement parce que la simulation
    //* ne depend plus du rendu.
    void runHeadless(Grid& grid, int steps)
    {
        std::ofstream file("simulation_data.csv"); // ouvre en mode troncature : pas besoin de std::remove
        if (!file.is_open())
        {
            std::cerr << "Impossible d'ouvrir simulation_data.csv\n";
            return;
        }

        writeCsvHeader(file);

        for (int t = 0; t < steps; ++t)
        {
            grid.step();
            appendCsvLine(file, t, grid);
        }

        std::cout << "CSV ecrit : " << steps << " tours, population finale = "
                  << grid.population() << '\n';
    }
}

int main(int argc, char** argv)
{
    Grid grid(WINDOW_WIDTH, WINDOW_HEIGHT);
    seedPopulation(grid, INITIAL_HERBIVORES);

    // ./main --csv  -> genere le CSV sans ouvrir de fenetre
    if (argc > 1 && std::string(argv[1]) == "--csv")
    {
        runHeadless(grid, 1000);
        return 0;
    }

    Application app(grid, TICKS_PER_SECOND);
    app.run();

    return 0;
}