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

    Cell* getCellAt(sf::Vector2<int> position) {
        if (!bounds(position)) {
            throw std::out_of_range("Grid::at out of limits");
        }
        return gridVector[position.y][position.x];
    }

    bool bounds(sf::Vector2<int> position) {
        return position.x >= 0 && position.x < cols && position.y >= 0 && position.y < rows;
    }

    void setCellAt(sf::Vector2<int> position, Entity* entity) {
        if (!bounds(position)) {
            std::cout << "Grid::clearCellAt out of limits" << std::endl;
            return;
        }

        gridVector[position.y][position.x]->setContent(entity);

        if (entity)
        {
            entity->setPosition(position); // Update entity pos
        } 
    }

    Entity* clearCellAt(sf::Vector2<int> position) {
        if (!bounds(position)) {
            std::cout << "Grid::clearCellAt out of limits" << std::endl;
            return nullptr;

        }
        return gridVector[position.y][position.x]->clearContent();
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

    int32_t getWidth() const {
        return width;
    }

    int32_t getHeight() const {
        return height;
    }

    void run() {
        std::cout << "Grid is running" << std::endl;
        isRunning = true;
        initGridVector();
        display(width, height);
    }

    // DEBUG
    friend std::ostream& operator<<(std::ostream& os, const Grid& grid) {
        for (std::size_t y = 0; y < grid.rows; ++y) {
            for (std::size_t x = 0; x < grid.cols; ++x) {
                if (grid.gridVector[y][x]->isEmpty()) {
                    os << ". ";
                } else {
                    os << "X ";
                }
                
            }
            os << '\n';
        }
        return os;
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