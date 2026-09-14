#include <SFML/Graphics.hpp>

#include "Grid.h"
#include "TestEntity.h"
#include "MathsFormulas.h"
#include "MyWindow.h"
#include "TestEntity.h"

#if defined(_MSC_VER) // Compile for Windows
    #define EXPORT_API extern "C" __declspec(dllexport)
#else // Compile for Linux / MacOS
    #define EXPORT_API extern "C" __attribute__((visibility("default")))
#endif

int main()
{
    auto* grid = new Grid({10, 10});
    MyWindow window("My Window", 800, 600, grid);

    auto* testEntity = new TestEntity("CACA");
    grid->setCellAt({0, 0}, testEntity);

    std::cout << *grid;

    window.run();

    return 0;
}
