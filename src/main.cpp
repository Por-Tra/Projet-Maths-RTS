#include <SFML/Graphics.hpp>

#include "Grid.h"
#include "TestEntity.h"
#include "MathsFormulas.h"
#include "TestEntity.h"

#if defined(_MSC_VER) // Compile for Windows
    #define EXPORT_API extern "C" __declspec(dllexport)
#else // Compile for Linux / MacOS
    #define EXPORT_API extern "C" __attribute__((visibility("default")))
#endif

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

int main()
{
    Grid grid(WINDOW_WIDTH, WINDOW_HEIGHT);
    grid.run();


    return 0;
}
