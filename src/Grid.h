#ifndef GRID_H
#define GRID_H

#include <vector>
#include <stdexcept>
#include <iostream>
#include <SFML/Graphics.hpp>

#include "Cell.h"

class Grid {
    int32_t width;
    int32_t height;
    int32_t cols;
    int32_t rows;
    std::vector<std::vector<Cell*>> gridVector;
    bool isRunning;

public:
    Grid(int gridWidth, int gridHeight)
    {
        this->width = gridWidth;
        this->height = gridHeight;

        this->cols = width / static_cast<int32_t>(Cell::CELL_SIZE);
        this->rows = height / static_cast<int32_t>(Cell::CELL_SIZE);

        this->isRunning = false;
    }

    ~Grid() {
        for (auto& row : gridVector) {
            for (Cell* cell : row) {
                delete cell;
            }
        }
    }

    // ----------------------------

    void initGridVector() {
        for (int row = 0; row < rows; row++) {
            std::vector<Cell*> cellVec;
            for (int col = 0; col < cols; col++) {
                cellVec.push_back(new Cell(col, row));
            }
            gridVector.push_back(cellVec);
        }
    }

    void display(int gWidth, int gHeight) {
        sf::RenderWindow window(sf::VideoMode(sf::Vector2u(gWidth, gHeight)), "Cellular Automata");

        while (window.isOpen() && isRunning)
        {
            while (const std::optional event = window.pollEvent())
            {
                if (event->is<sf::Event::Closed>())
                {
                    window.close();
                }
            }

            window.clear(sf::Color::Black);

            for (auto& row : gridVector) {
                for (Cell* cell : row) {
                    window.draw(cell->cell);
                }
            }

            window.display();
        }
    }

    void setWidth(int gWidth) {
        this->width = gWidth;
    }

    void setHeight(int gHeight) {
        this->height = gHeight;
    }

    void run() {
        std::cout << "Grid is running" << std::endl;
        isRunning = true;
        initGridVector();
        display(width, height);
    }
};

#endif // GRID_H