#include <SFML/Graphics.hpp>

#include "Grid.h"
#include "TestEntity.h"
#include "MathsFormulas.h"
#include "TestEntity.h"
#include "Herbivore.h"

#if defined(_MSC_VER) // Compile for Windows
    #define EXPORT_API extern "C" __declspec(dllexport)
#else // Compile for Linux / MacOS
    #define EXPORT_API extern "C" __attribute__((visibility("default")))
#endif

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

int main()
{
    Grid* grid = new Grid({10, 10});
    
    Herbivore* H1 = new Herbivore("Herbivore1");
    grid->setCellAt({0, 0}, H1);

    Herbivore* H2 = new Herbivore("Herbivore2");
    grid->setCellAt({5, 0}, H2);

    std::cout << *grid;

    int t = 0;
    while (t < 100) {
        H1->move(*grid);
        H2->move(*grid);

        H1->reproduce(*grid);
        H2->reproduce(*grid);
        
        std::cout << "After move " << t + 1 << ":\n";
        std::cout << *grid;
        t++;
    }


    // MyWindow window("My Window", 800, 600, grid);
    // window.run();
    Grid grid(WINDOW_WIDTH, WINDOW_HEIGHT);
    grid.run();


    return 0;
}
