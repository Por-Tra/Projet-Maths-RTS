#ifndef ENTITY_H
#define ENTITY_H

#include "grid.h"

// ! JSP si ça va marcher
// Fonctionnalité globale des entités :
// - positionner
// - se déplacer
// - se supprimer
class Entity {
public:
    Entity(int init_x, int init_y, Grid<Entity>& init_grid) : x(init_x), y(init_y), grid(init_grid) {
        // Place the entity in the grid at its initial position
        if (!grid.bounds(x, y)) {
            throw std::out_of_range("Entity::Entity initial position out of limits");
        }
        grid.at(x, y).set(*this);
    }
    ~Entity() {
        //! @loazur chesk if not memory problems
        grid.set_cell_empty(x, y); // clear an entity from the grid
    }
    
    void update_position(int x, int y)
    {
        if (!grid.bounds(x, y)) {
            throw std::out_of_range("Entity::update_position out of limits");
        }
        grid.set_cell_empty(this->x, this->y); // clear the old position
        this->x = x;
        this->y = y;
        grid.at(x, y).set(*this); // set the new position
    }

    
private:
    int x, y; // position of the entity
    Grid<Entity>& grid;
};

#endif // ENTITY_H