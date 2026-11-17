#ifndef RENDERER_H
#define RENDERER_H

#include <cmath>
#include <memory>

#include <SFML/Graphics.hpp>

#include "Cell.h"
#include "Entity.h"
#include "Grid.h"

class Renderer
{
    sf::RectangleShape background;
    sf::Texture backgroundTexture;
    sf::Sprite backgroundSprite{backgroundTexture};
    sf::VertexArray entityTriangles{sf::PrimitiveType::Triangles};

    static constexpr int CIRCLE_SEGMENTS = 12; // 12 suffit largement a 10 px de rayon
    static constexpr float RADIUS_RATIO = 0.35f;

public:
    explicit Renderer(const Grid& grid)
    {
        buildStaticGeometry(grid);
    }

    void draw(sf::RenderWindow& window, const Grid& grid)
    {
        if (backgroundTexture.getSize().x > 0 && backgroundTexture.getSize().y > 0)
        {
            window.draw(backgroundSprite);
        }
        else
        {
            window.draw(background);
        }

        buildEntityGeometry(grid);
        window.draw(entityTriangles);
    }

private:

    // Build the static geometry of the grid and background
    void buildStaticGeometry(const Grid& grid)
    {
        const float width = grid.pixelWidth();
        const float height = grid.pixelHeight();

        background.setSize(sf::Vector2f(width, height));
        background.setPosition(sf::Vector2f(0.f, 0.f));

        // Create background with image texture
        if (backgroundTexture.loadFromFile("asset/background.png"))
        {
            backgroundSprite.setTexture(backgroundTexture, true);
            const sf::Vector2u textureSize = backgroundTexture.getSize();
            backgroundSprite.setScale({
                    width / static_cast<float>(textureSize.x),
                    height / static_cast<float>(textureSize.y)
                });
            backgroundSprite.setPosition({0.f, 0.f});
        }

    }

    void buildEntityGeometry(const Grid& grid)
    {
        entityTriangles.clear();

        for (const std::unique_ptr<Entity>& entity : grid.getEntities())
        {
            const sf::Vector2i cell = entity->getPosition();

            const sf::Vector2f center(
                (static_cast<float>(cell.x) + 0.5f) * Cell::CELL_SIZE,
                (static_cast<float>(cell.y) + 0.5f) * Cell::CELL_SIZE);

            appendCircle(center, Cell::CELL_SIZE * RADIUS_RATIO, entity->color());
        }
    }

    // Append a filled circle to the vertex array for entity representation
    void appendCircle(sf::Vector2f center, float radius, sf::Color color)
    {
        sf::Vector2f previous(center.x + radius, center.y);

        for (int i = 1; i <= CIRCLE_SEGMENTS; ++i)
        {
            constexpr float TWO_PI = 6.28318530718f;
            const float angle = TWO_PI * static_cast<float>(i) / static_cast<float>(CIRCLE_SEGMENTS);
            const sf::Vector2f current(center.x + radius * std::cos(angle), center.y + radius * std::sin(angle));

            entityTriangles.append(sf::Vertex{center, color});
            entityTriangles.append(sf::Vertex{previous, color});
            entityTriangles.append(sf::Vertex{current, color});

            previous = current;
        }
    }
};

#endif // RENDERER_H
