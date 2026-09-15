#ifndef CELL_H
#define CELL_H

#include <SFML/Graphics.hpp>
#include "Entity.h"

class Cell {
    Entity* cell_content = nullptr;

    std::int32_t xCell; // colonne (
    std::int32_t yCell; // ligne

public:
    static constexpr float CELL_SIZE = 30.f;

    sf::RectangleShape cell;

    Cell(int xPos, int yPos, Entity* cell_content = nullptr) {
        this->xCell = xPos;
        this->yCell = yPos;
        this->cell_content = cell_content;

        // Creating visual
        this->cell = sf::RectangleShape(sf::Vector2f(CELL_SIZE, CELL_SIZE));
        this->cell.setFillColor(sf::Color::White);
        this->cell.setOutlineColor(sf::Color::Black);
        this->cell.setOutlineThickness(1.f);

        this->cell.setPosition(sf::Vector2f(xCell * CELL_SIZE, yCell * CELL_SIZE));
    }

    // ---- Display Related Methods ----

    void setPosition(int xPos, int yPos) {
        xCell = xPos;
        yCell = yPos;

        cell.setPosition(sf::Vector2f(xCell * CELL_SIZE, yCell * CELL_SIZE));
    }

    int32_t getX() {
        return xCell;
    }

    int32_t getY() {
        return yCell;
    }

    // ---- Content Related Methods ----

    bool isEmpty() const
    {
        return cell_content == nullptr;
    }

    Entity* clearContent()
    {
        Entity* entity = cell_content;
        cell_content = nullptr;

        return entity;
    }

    //* Setter / getter

    void setContent(Entity* entity)
    {
        cell_content = entity;
    }

    Entity* getContent() {
        return cell_content;
    }
};

#endif // CELL_H