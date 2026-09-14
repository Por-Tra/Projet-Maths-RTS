#ifndef GRID_H
#define GRID_H

#include <vector>
#include <stdexcept>
#include <iostream>

#include "Cell.h"

class Grid {
    sf::Vector2<int> grid_size;
    std::vector<std::vector<Cell*>> grid;

public:
    Grid(sf::Vector2<int> grid_size) : grid_size(grid_size), grid(grid_size.y, std::vector<Cell*>(grid_size.x))
    {
        // Build a grid of empty cells

        for (std::size_t y = 0; y < grid_size.y; ++y) {
            for (std::size_t x = 0; x < grid_size.x; ++x) {
                grid[y][x] = new Cell();
            }
        }
    }

    ~Grid() {
        for (auto& row: grid) {
            for (Cell* cell: row) {
                delete cell;
            }
        }
    }

    int getWidth() const { return grid_size.x; }
    int getHeight() const { return grid_size.y; }

    bool bounds(sf::Vector2<int> position) const {
        return position.x >= 0 && position.y >= 0 && position.x < static_cast<int>(grid_size.x) && position.y < static_cast<int>(grid_size.y);
    }

    Cell* getCellAt(sf::Vector2<int> position) {
        if (!bounds(position)) {
            throw std::out_of_range("Grid::at out of limits");
        }
        return grid[position.y][position.x];
    }

    void setCellAt(sf::Vector2<int> position, Entity* entity) {
        if (!bounds(position)) {
            std::cout << "Grid::clearCellAt out of limits" << std::endl;
            return;

        }
        grid[position.y][position.x]->setContent(entity);
    }

    Entity* clearCellAt(sf::Vector2<int> position) {
        if (!bounds(position)) {
            std::cout << "Grid::clearCellAt out of limits" << std::endl;
            return nullptr;

        }
        return grid[position.y][position.x]->clearContent();
    }

    void moveCellToPosition(sf::Vector2<int> cellAtPos, sf::Vector2<int> newPos) {
        Entity* entity = clearCellAt(cellAtPos);
        setCellAt(newPos, entity);
    }

    friend std::ostream& operator<<(std::ostream& os, const Grid& grid) {
        for (std::size_t y = 0; y < grid.getHeight(); ++y) {
            for (std::size_t x = 0; x < grid.getWidth(); ++x) {
                if (grid.grid[y][x]->isEmpty()) {
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

#endif // GRID_H