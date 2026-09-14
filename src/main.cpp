#include <SFML/Graphics.hpp>

#include "MathsFormulas.h"
#include "MyWindow.h"

#if defined(_MSC_VER) // Compile for Windows
    #define EXPORT_API extern "C" __declspec(dllexport)
#else // Compile for Linux / MacOS
    #define EXPORT_API extern "C" __attribute__((visibility("default")))
#endif

int main()
{
    MyWindow window("My Window", 800, 600);

    // Green Circle
    auto circle_shape_green = new sf::CircleShape(30, 30);
    circle_shape_green->setFillColor(sf::Color::Green);
    window.addShape(circle_shape_green);

    // Red Circle
    auto circle_shape_red = new sf::CircleShape(30, 30);
    circle_shape_red->setFillColor(sf::Color::Red);
    circle_shape_red->setPosition(sf::Vector2f(10, 10));
    circle_shape_red->setScale(sf::Vector2f(4, 4));

    window.addShape(circle_shape_red);

    window.run();

    return 0;
}
