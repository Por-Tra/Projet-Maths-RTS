#ifndef ENTITY_H
#define ENTITY_H

#include <string>
#include <utility>

#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Vector2.hpp>

class Grid; // Forward declaration of Grid class

enum class Species // Species of Entity (use this enum to compare an entity type)
{
    Herbivore,
    Carnivore,
    Plant
};

/*
 * Entity is an abstract class used to create each single species (Herbivore, Carnivore...)
 * Each entity has a name, a position, and if its alive
 */

class Entity
{
protected:
    std::string name;
    sf::Vector2i position{0, 0};
    bool alive{true};

    //* ATTENTION : malgre son nom, cette methode renvoie la CASE VISEE (coordonnees
    //* absolues), pas un vecteur direction. Renvoyer sa propre position = "je ne bouge pas".
    virtual sf::Vector2i chooseDirection(Grid& grid) const = 0;

public:
    Entity() = default;
    explicit Entity(std::string entityName)
        : name(std::move(entityName))
    {
    }

    virtual ~Entity() = default;

    Entity(const Entity&) = delete;
    Entity& operator=(const Entity&) = delete;

    [[nodiscard]] const std::string& getName() const noexcept { return name; }
    [[nodiscard]] sf::Vector2i getPosition() const noexcept { return position; }

    [[nodiscard]] bool isAlive() const noexcept { return alive; }
    void kill() noexcept { alive = false; }

    [[nodiscard]] virtual Species species() const noexcept = 0;
    [[nodiscard]] virtual sf::Color color() const noexcept = 0;

    void move(Grid& grid) const;

    virtual void reproduce(Grid& grid) = 0;

    virtual void update() = 0;

private:
    friend class Grid;
    void setPosition(sf::Vector2i newPosition) noexcept { position = newPosition; }
};

#endif // ENTITY_H