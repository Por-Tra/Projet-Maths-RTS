#ifndef MYWINDOW_H
#define MYWINDOW_H

#include <SFML/Graphics.hpp>
#include <vector>
#include <iostream>

class MyWindow
{
    sf::RenderWindow window;
    std::vector<sf::Shape*> shapes; // All shape on the window
    Grid* grid = nullptr;
    sf::Vector2<int> currentPosition = {0, 0};

public:
    MyWindow(const char* title, unsigned width, unsigned height, Grid* grid)
    {
        window.create(sf::VideoMode({width, height}), title);
        window.setFramerateLimit(60);

        this->grid = grid;
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

        shapes.clear();
    }


    void moveCellToDown(sf::Vector2<int> cellPosition) {
        sf::Vector2 newPosition = {cellPosition.x, cellPosition.y + 1};
        grid->moveCellToPosition(cellPosition, newPosition);
        currentPosition = newPosition;

        std::cout << *grid << std::endl;
        std::cout << "----------------------------------------- \n";
    }

    void moveCellToUp(sf::Vector2<int> cellPosition) {
        sf::Vector2 newPosition = {cellPosition.x, cellPosition.y - 1};
        grid->moveCellToPosition(cellPosition, newPosition);
        currentPosition = newPosition;

        std::cout << *grid << std::endl;
        std::cout << "----------------------------------------- \n";
    }

    void moveCellToLeft(sf::Vector2<int> cellPosition) {
        sf::Vector2 newPosition = {cellPosition.x - 1, cellPosition.y};
        grid->moveCellToPosition(cellPosition, newPosition);
        currentPosition = newPosition;

        std::cout << *grid << std::endl;
        std::cout << "----------------------------------------- \n";
    }

    void moveCellToRight(sf::Vector2<int> cellPosition) {
        sf::Vector2 newPosition = {cellPosition.x + 1, cellPosition.y};
        grid->moveCellToPosition(cellPosition, newPosition);
        currentPosition = newPosition;

        std::cout << *grid << std::endl;
        std::cout << "----------------------------------------- \n";
    }

    void run()
    {
        while (window.isOpen())
        {
            while (const std::optional event = window.pollEvent())
            {
                if (event->is<sf::Event::Closed>())
                {
                    window.close();
                }
                else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
                {
                    if (keyPressed->scancode == sf::Keyboard::Scancode::Up)
                        moveCellToUp(currentPosition);


                    if (keyPressed->scancode == sf::Keyboard::Scancode::Down)
                        moveCellToDown(currentPosition);

                    if (keyPressed->scancode == sf::Keyboard::Scancode::Left)
                        moveCellToLeft(currentPosition);


                    if (keyPressed->scancode == sf::Keyboard::Scancode::Right)
                        moveCellToRight(currentPosition);




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

};


#endif // MYWINDOW_H