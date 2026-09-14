#ifndef ENTITY_H
#define ENTITY_H

// Abstract function used to create Entities

class Entity {
    // TODO - Add other values used for each entities

    float hunger_level = 100.0f;

public:
    Entity() = default;

    virtual ~Entity();
    virtual void update() = 0; // Only to make abstract

    // TODO - Base methods of Entities
};

#endif // ENTITY_H