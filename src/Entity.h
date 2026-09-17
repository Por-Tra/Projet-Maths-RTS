#ifndef ENTITY_H
#define ENTITY_H

#include <string>
#include <utility>

#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Vector2.hpp>

//! Dont remove pls
class Grid; // Forward declaration of Grid class

//* Type d'entite. Sert a remplacer les dynamic_cast dans les boucles chaudes :
//* un dynamic_cast fait une recherche dans la table RTTI, un enum est une comparaison d'entier.
//* Garder dynamic_cast pour du code froid (debug, outils), pas pour la simulation.
enum class Species
{
    Herbivore,
    Carnivore,
    Plant
};

//* Classe abstraite de base.
//* Elle ne contient PLUS de sf::CircleShape : le modele de simulation ne doit rien
//* savoir du rendu. C'est le Renderer qui dessine, a partir de position() et color().
//* Gain : ~250 octets economises par entite, et on peut faire tourner la simulation
//* sans fenetre (mode headless pour generer le CSV).
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

    // Une entite vit a une seule place dans la grille : la copier n'a pas de sens.
    Entity(const Entity&) = delete;
    Entity& operator=(const Entity&) = delete;

    const std::string& getName() const noexcept { return name; }
    sf::Vector2i getPosition() const noexcept { return position; }

    bool isAlive() const noexcept { return alive; }
    void kill() noexcept { alive = false; }

    //* Pour le rendu et les statistiques
    virtual Species species() const noexcept = 0;
    virtual sf::Color color() const noexcept = 0;

    void move(Grid& grid);

    virtual void reproduce(Grid& grid) = 0;

    virtual void update() = 0;

private:
    friend class Grid;
    void setPosition(sf::Vector2i newPosition) noexcept { position = newPosition; }
};

#endif // ENTITY_H