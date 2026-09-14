#ifndef GRID_H
#define GRID_H

#include <vector>
#include <stdexcept>
#include <iostream>

#include "Cell.h"

class Grid {
    std::size_t width, height;
    std::vector<std::vector<Cell*>> grid;

public:
    Grid(std::size_t width, std::size_t height) : width(width), height(height), grid(height, std::vector<Cell<T>>(width))
    {
        // Build a grid of empty cells

        for (std::size_t y = 0; y < height; ++y) {
            for (std::size_t x = 0; x < width; ++x) {
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

    std::size_t getWidth() const { return width; }
    std::size_t getHeight() const { return height; }

    //! @loazur check this   ----------------------------------------------
    bool bounds(int x, int y) const {
        return x >= 0 && y >= 0 && x < static_cast<int>(width) && y < static_cast<int>(height);
    }

    Cell& getCellAt(int x, int y) {
        if (!bounds(x, y)) {
            throw std::out_of_range("Grid::at out of limits");
        }
        return *grid[y][x];
    }

    const Cell& getCellAt(int x, int y) const {
        if (!bounds(x, y)) {
            throw std::out_of_range("Grid::at out of limits");
        }
        return *grid[y][x];
    }
    //! ----------------------------------------------

    void setCellAt(int x, int y) {
        if (!bounds(x, y)) {
            throw std::out_of_range("Grid::set_cell_empty out of limits");
        }
        grid[y][x]->clearContent();
    }


    friend std::ostream& operator<<(std::ostream& os, const Grid& grid) {
        for (std::size_t y = 0; y < grid.height; ++y) {
            for (std::size_t x = 0; x < grid.width; ++x) {
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