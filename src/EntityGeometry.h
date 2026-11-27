#ifndef ENTITY_GEOMETRY_H
#define ENTITY_GEOMETRY_H

#include <cstddef>
#include <vector>
#include <SFML/Graphics/Vertex.hpp>

#include "Grid.h"

// The existing four-segment animal shape is a diamond.
// Two triangles fill it without a center vertex or trigonometry.
class EntityGeometry
{
    std::vector<sf::Vertex> vertices;

public:
    static constexpr std::size_t VERTICES_PER_ENTITY = 3;   // triangles
    static constexpr float RADIUS = Cell::CELL_SIZE * 0.35f;

    void rebuild(const Grid& grid)
    {
        vertices.resize(grid.population() * VERTICES_PER_ENTITY);
        auto* output = vertices.data();
        for (const auto& entity : grid.getEntities())
        {
            const auto cell = entity->getPosition();
            const float x = (static_cast<float>(cell.x) + 0.5f) * Cell::CELL_SIZE;
            const float y = (static_cast<float>(cell.y) + 0.5f) * Cell::CELL_SIZE;
            const auto color = entity->color();
            const sf::Vector2f right{x + RADIUS, y}, bottom{x, y + RADIUS};
            const sf::Vector2f left{x - RADIUS, y}, top{x, y - RADIUS};
            output[0] = {right, color};
            output[1] = {bottom, color};
            output[2] = {left, color};
            output[3] = {right, color};
            output[4] = {left, color};
            output[5] = {top, color};
            output += VERTICES_PER_ENTITY;
        }
    }

    [[nodiscard]] const std::vector<sf::Vertex>& getVertices() const noexcept { return vertices; }
};

#endif // ENTITY_GEOMETRY_H
