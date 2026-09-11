#include <SFML/Graphics.hpp>

#include "MyWindow.h"

#if defined(_MSC_VER) // Si on compile sous Windows avec MSVC
    #define EXPORT_API extern "C" __declspec(dllexport)
#else // Pour Linux / macOS
    #define EXPORT_API extern "C" __attribute__((visibility("default")))
#endif

int main()
{
    
    MyWindow window("My Window", 800, 600);

    // Cercle Vert
    auto circle_shape_green = new sf::CircleShape(30, 30);
    circle_shape_green->setFillColor(sf::Color::Green);
    window.addShape(circle_shape_green);

    // Cercle Rouge
    auto circle_shape_red = new sf::CircleShape(30, 30);
    circle_shape_red->setFillColor(sf::Color::Red);
    circle_shape_red->setPosition(sf::Vector2f(10, 10));
    circle_shape_red->setScale(sf::Vector2f(4, 4));

    window.addShape(circle_shape_red);

    window.run();

    return 0;
}
