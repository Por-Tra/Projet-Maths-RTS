#include <SFML/Graphics.hpp>

#include "MyWindow.h"

int main()
{
    
    MyWindow window("My Window", 800, 600);

    CircleShape* circle = new CircleShape(50.0f, 100.0f, 100.0f, sf::Color::Green);
    window.addShape(circle);

    


    window.run();


    return 0;
}
