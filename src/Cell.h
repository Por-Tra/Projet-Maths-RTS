#ifndef CELL_H
#define CELL_H

#include <SFML/System/Vector2.hpp>

class Entity;

/*
 * Class handles a single cell of the grid, stores it's coordinate and entity stored
 */


class Cell
{
    Entity* cell_content = nullptr; // Entity stored in that cell
    sf::Vector2i coordinates{0, 0};

public:
    static constexpr float CELL_SIZE = 30.f; // Visual size of cell

    Cell() = default;
    Cell(int xPos, int yPos)
        : coordinates{xPos, yPos}
    {
    }

    // ---- Coordinate Management ----

    int getX() const noexcept { return coordinates.x; }
    int getY() const noexcept { return coordinates.y; }
    sf::Vector2i getCoordinates() const noexcept { return coordinates; }

    // ---- Content Management ----

    bool isEmpty() const noexcept { return cell_content == nullptr; }

    Entity* getContent() const noexcept { return cell_content; }

    void setContent(Entity* entity) noexcept { cell_content = entity; }

    Entity* clearContent() noexcept
    {
        Entity* entity = cell_content;
        cell_content = nullptr;
        return entity;
    }
};

#endif // CELL_H