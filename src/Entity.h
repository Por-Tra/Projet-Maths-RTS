#ifndef ENTITY_H
#define ENTITY_H

#include <string>

// Abstract function used to create Entities

class Entity
{
protected:
    std::string name;

public:
    explicit Entity() = default;
    explicit Entity(const std::string& name)
        : name(name)
    {
    }

    virtual ~Entity() = default;

    const std::string& getName() const
    {
        return name;
    }

    virtual void update() = 0;
};

#endif // ENTITY_H