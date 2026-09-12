#ifndef GRID_H
#define GRID_H

#include <vector>
#include <optional>
#include <stdexcept>
#include <iostream>

template <typename T>
class Cell {
public:
    Cell() = default;

    bool isEmpty() const 
    {
        return !cell_content.has_value(); 
    }

    void clear() 
    { 
        cell_content.reset(); 
    }
    
    //* Setter and getter

    // = get value
    T& value() { 
        return *cell_content; 
    }
    const T& value() const // = get value const
    {
        return *cell_content; 
    }

    void set(T entity) 
    { 
        cell_content = std::move(entity); 
    }
    

private:
    std::optional<T> cell_content; //! std::optional = can be empty @loazur
};

template <typename T>
class Grid {
public:
    Grid(std::size_t width, std::size_t height) : width(width), height(height), cell(height, std::vector<Cell<T>>(width)) 
    {
        // Build a grid of empty cells

        for (std::size_t y = 0; y < height; ++y) {
            for (std::size_t x = 0; x < width; ++x) {
                cell[y][x] = Cell<T>();
            }
        }
    }

    std::size_t GetWidth() const { return width; }
    std::size_t GetHeight() const { return height; }

    //! @loazur check this  ----------------------------------------------
    bool bounds(int x, int y) const {
        return x >= 0 && y >= 0 && x < static_cast<int>(width) && y < static_cast<int>(height);
    }

    Cell<T>& at(int x, int y) {
        if (!bounds(x, y)) {
            throw std::out_of_range("Grid::at out of limits");
        }
        return cell[y][x];
    }

    const Cell<T>& at(int x, int y) const {
        if (!bounds(x, y)) {
            throw std::out_of_range("Grid::at out of limites");
        }
        return cell[y][x];
    }
    //! ----------------------------------------------

    void set_cell_empty(int x, int y) {
        if (!bounds(x, y)) {
            throw std::out_of_range("Grid::set_cell_empty out of limits");
        }
        cell[y][x].clear();
    }

    void print() const {
        for (std::size_t y = 0; y < height; ++y) {
            for (std::size_t x = 0; x < width; ++x) {
                if (cell[y][x].isEmpty()) {
                    std::cout << ". "; // Enmpty
                } else {
                    std::cout << "X "; // Occupied
                }
            }
            std::cout << std::endl;
        }
    }

private:
    std::size_t width, height;
    std::vector<std::vector<Cell<T>>> cell;
};

#endif // GRID_H