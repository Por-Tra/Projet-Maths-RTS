#include <SFML/Graphics.hpp>

#include "Grid.h"
#include "MathsFormulas.h"
#include "Herbivore.h"
#include "tools.h"

#if defined(_MSC_VER) // Compile for Windows
    #define EXPORT_API extern "C" __declspec(dllexport)
#else // Compile for Linux / MacOS
    #define EXPORT_API extern "C" __attribute__((visibility("default")))
#endif

const int WINDOW_WIDTH = 1000;
const int WINDOW_HEIGHT = 1000;

int main()
{
    // delete CSV file if it exists
    std::remove("simulation_data.csv");

    // Engine Test
    Grid* grid = new Grid(WINDOW_WIDTH, WINDOW_HEIGHT);
    
    Herbivore* H1 = new Herbivore("Herbivore1");
    grid->setCellAt({0, 0}, H1);
    H1->setAge(30);

    Herbivore* H2 = new Herbivore("Herbivore2");
    grid->setCellAt({5, 0}, H2);
    H2->setAge(30);

    std::ofstream file("simulation_data.csv");
    int t = 0;

    if (file.is_open()) {
        // On écrit l'en-tête une fois pour toutes
        file << "Time,HerbivoreCount\n";

        while (t < 1000) 
        {
            
            for (int row = 0; row < grid->getRows(); ++row) 
            {
                for (int col = 0; col < grid->getCols(); ++col) 
                {
                    Cell* cell = grid->getCellAt({col, row});
                    if (cell && !cell->isEmpty()) 
                    {
                        Entity* entity = cell->getContent();
                        entity->update();
                        entity->move(*grid);
                        entity->reproduce(*grid);
                    }
                }
            }

            // std::cout << *grid << std::endl;
            createCSV(file, t, *grid);

            t++;
        }
    }



    // graphic Test
    // Grid grid2(WINDOW_WIDTH, WINDOW_HEIGHT);
    // grid2.run();


    return 0;
}
