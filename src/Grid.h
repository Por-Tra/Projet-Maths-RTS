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

    void setCellAt(sf::Vector2<int> position, Entity* entity) {
        if (!bounds(position)) {
            std::cout << "Grid::clearCellAt out of limits" << std::endl;
            return;
        }

        grid[position.y][position.x]->setContent(entity);

        if (entity)
        {
            entity->setPosition(position); // Update entity pos
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

    void moveCellToPosition(sf::Vector2<int> cellAtPos, sf::Vector2<int> newPos) {
        Entity* entity = clearCellAt(cellAtPos);
        setCellAt(newPos, entity); // Update entity pos
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

//! Dont touch pls
inline void Entity::move(Grid& grid)
{
    sf::Vector2<int> newPosition = chooseDirection(grid);
    if (grid.bounds(newPosition))
    {
        grid.moveCellToPosition(position, newPosition);
    }
}

#endif // GRID_H