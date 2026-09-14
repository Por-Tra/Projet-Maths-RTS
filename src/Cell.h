#ifndef CELL_H
#define CELL_H

#include "Entity.h"

class Cell {
    Entity* cell_content = nullptr;

public:
    Cell(Entity* cell_content = nullptr) {
        this->cell_content = cell_content;
    }

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

    //* Setter and getter

    void setContent(Entity* entity)
    {
        cell_content = entity;
    }

    Entity* getContent() {
        return cell_content;
    }


};

#endif // CELL_H
