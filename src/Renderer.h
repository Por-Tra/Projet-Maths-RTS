#ifndef RENDERER_H
#define RENDERER_H

#include <algorithm>
#include <SFML/Graphics.hpp>

#include "Cell.h"
#include "Entity.h"
#include "Grid.h"
#include "EntityGeometry.h"

class Renderer
{
    sf::RectangleShape background;
    sf::Texture backgroundTexture;
    sf::Sprite backgroundSprite{backgroundTexture};
    EntityGeometry entityGeometry;
    sf::VertexBuffer gpuGeometry{sf::PrimitiveType::Triangles, sf::VertexBuffer::Usage::Stream};
    bool hasGeometry = false;
    bool useVertexBuffer = true;
    bool gpuReady = false;

public:
    explicit Renderer(const Grid& grid)
    {
        buildStaticGeometry(grid);
    }

    void draw(sf::RenderTarget& window, const Grid& grid, bool entitiesChanged = true)
    {
        if (backgroundTexture.getSize().x > 0 && backgroundTexture.getSize().y > 0)
        {
            window.draw(backgroundSprite);
        }
        else
        {
            window.draw(background);
        }

        if (entitiesChanged || !hasGeometry)
        {
            entityGeometry.rebuild(grid);
            uploadGeometry();
            hasGeometry = true;
        }
        const auto& vertices = entityGeometry.getVertices();
        if (!vertices.empty())
        {
            if (gpuReady) window.draw(gpuGeometry, 0, vertices.size());
            else window.draw(vertices.data(), vertices.size(), sf::PrimitiveType::Triangles);
        }
    }

private:

    void uploadGeometry()
    {
        gpuReady = false;
        const auto& vertices = entityGeometry.getVertices();
        if (vertices.empty() || !useVertexBuffer) return;

        useVertexBuffer = sf::VertexBuffer::isAvailable();
        if (!useVertexBuffer) return;

        const auto capacity = gpuGeometry.getVertexCount();
        if (capacity < vertices.size())
            useVertexBuffer = gpuGeometry.create(std::max(vertices.size(), capacity + capacity / 2));

        if (useVertexBuffer) gpuReady = gpuGeometry.update(vertices.data(), vertices.size(), 0);
        // Keep the CPU drawing path if allocation/upload is unavailable.
        if (!gpuReady) useVertexBuffer = false;
    }

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

};

#endif // RENDERER_H
