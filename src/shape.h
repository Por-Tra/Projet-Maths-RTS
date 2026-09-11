#ifndef SHAPE_H
#define SHAPE_H

#include <SFML/Graphics.hpp>

// Abstract class for shape
class Shape
{
public:

    Shape() = default;
    virtual ~Shape() = default;

    virtual void draw(sf::RenderWindow& window) const = 0;
};

class CircleShape : public Shape
{
public:
    CircleShape(float radius, float x, float y, const sf::Color& color)
    {
        m_circle.setRadius(radius);
        m_circle.setPosition(x, y); // Fonction de SFML
        m_circle.setFillColor(color); // IDEM
    }

    void draw(sf::RenderWindow& window) const override
    {
        window.draw(m_circle); // Draw the circle on the window
    }

private:
    sf::CircleShape m_circle; // Circle Type : SFML
};



#endif // SHAPE_H