


#ifndef MYWINDOW_H
#define MYWINDOW_H

#include <SFML/Graphics.hpp>
#include <vector>
#include "shape.h"

class MyWindow
{

public:
    MyWindow(const char* title, int width, int height)
    {
        window.create(sf::VideoMode(width, height), title);
        window.setFramerateLimit(60);
    }
    ~MyWindow() = default;

    void addShape(Shape* shape)
    {
        shapes.push_back(shape);
    }

    void run()
    {
        while (window.isOpen())
        {
            sf::Event event{};
            while (window.pollEvent(event))
            {
                if (event.type == sf::Event::Closed)
                {
                    window.close();
                }
            }

            window.clear(sf::Color(30, 30, 30));
            
            for (const auto& shape : shapes)
            {
                shape->draw(window);
            }

            window.display();
        }

    }

private:
    sf::RenderWindow window;

    std::vector<Shape*> shapes; // All shape on the window


};


#endif // MYWINDOW_H