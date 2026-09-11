#ifndef MYWINDOW_H
#define MYWINDOW_H

#include <SFML/Graphics.hpp>
#include <vector>
#include <iostream>

class MyWindow
{

public:
    MyWindow(const char* title, unsigned width, unsigned height)
    {
        window.create(sf::VideoMode({width, height}), title);
        window.setFramerateLimit(60);
    }
    ~MyWindow() = default;

    void addShape(sf::Shape* shape)
    {
        shapes.push_back(shape);
    }

    void clearShapes() {
        for (auto* shape: shapes) {
            free(shape);
        }

        std::cout << "Successfully cleared shapes." << std::endl;
        shapes.clear();
    }

    void run()
    {
        while (window.isOpen())
        {
            while (const std::optional event = window.pollEvent())
            {
                // "close requested" event: we close the window
                if (event->is<sf::Event::Closed>())
                    window.close();


                if (const auto* mouseMoved = event->getIf<sf::Event::MouseMoved>())
                {
                    std::cout << "new mouse x: " << mouseMoved->position.x << std::endl;
                    std::cout << "new mouse y: " << mouseMoved->position.y << std::endl;
                }

                if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
                {
                    clearShapes();
                }


            }

            window.clear(sf::Color(30, 30, 30));
            
            for (const auto& shape : shapes)
            {
                window.draw(*shape);
            }

            window.display();
        }

    }

private:
    sf::RenderWindow window;
    std::vector<sf::Shape*> shapes; // All shape on the window

};


#endif // MYWINDOW_H