#ifndef ENTITY_H
#define ENTITY_H

#include <string>
#include <SFML/System/Vector2.hpp>


//! Dont remove pls
class Grid; // Forward declaration of Grid class

// Abstract function used to create Entities

class Entity
{
protected:
    std::string name;
    sf::Vector2<int> position{0, 0};

    virtual sf::Vector2<int> chooseDirection(Grid& grid) const = 0;

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

    sf::Vector2<int> getPosition() const { return position; }

    void move(Grid& grid);

    virtual void reproduce(Grid& grid) = 0;

    virtual void update() = 0;

private:
    friend class Grid;
    void setPosition(sf::Vector2<int> newPosition) { position = newPosition; }
};




#endif // ENTITY_H