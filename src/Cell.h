#ifndef CELL_H
#define CELL_H

#include <SFML/System/Vector2.hpp>

class Entity;

//* Cellule allegee.
//* Avant : chaque Cell embarquait un sf::RectangleShape (~250 octets, un sf::Transform,
//* une matrice, etc.) et etait allouee individuellement avec `new`. Pour une grille
//* 33x33 ca fait 1089 allocations + 1089 formes redessinees une par une chaque frame.
//* Maintenant une Cell = un pointeur + deux entiers (16 octets), stockee PAR VALEUR
//* dans un seul vecteur contigu.
class Cell
{
    Entity* cell_content = nullptr; // observateur : la Grid est proprietaire des entites
    sf::Vector2i coordinates{0, 0};

public:
    static constexpr float CELL_SIZE = 30.f;

    Cell() = default;
    Cell(int xPos, int yPos)
        : coordinates{xPos, yPos}
    {
    }

    // ---- Coordonnees ----

    int getX() const noexcept { return coordinates.x; }
    int getY() const noexcept { return coordinates.y; }
    sf::Vector2i getCoordinates() const noexcept { return coordinates; }

    // ---- Contenu ----

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