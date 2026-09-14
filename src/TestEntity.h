#ifndef TESTENTITY_H
#define TESTENTITY_H

#include "Entity.h"

class TestEntity: public Entity {
public:
    explicit TestEntity(const std::string& name)
       : Entity(name)
    {
    }

    explicit TestEntity()
        : Entity() {

    }

    void update() override
    {
        // Comportement spécifique à TestEntity
    }
};

#endif // TESTENTITY_H
