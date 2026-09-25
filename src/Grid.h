#ifndef GRID_H
#define GRID_H

#include <algorithm>
#include <array>
#include <cstddef>
#include <memory>
#include <optional>
#include <ostream>
#include <stdexcept>
#include <utility>
#include <vector>

#include <SFML/System/Vector2.hpp>

#include "Cell.h"
#include "Entity.h"
#include "Random.h"

class Grid
{
    int cols{0};
    int rows{0};

    std::vector<Cell> cells;
    std::vector<std::unique_ptr<Entity>> entities;
    std::size_t birthCount{0};
    std::size_t deathCount{0};
    std::size_t herbivoreCount{0};
    std::size_t carnivoreCount{0};

    [[nodiscard]] std::size_t index(sf::Vector2i position) const noexcept
    {
        return static_cast<std::size_t>(position.y) * static_cast<std::size_t>(cols)
             + static_cast<std::size_t>(position.x);
    }

public:
    //* Von Neumann Neighborhood (4 directions). Shared with all entities.
    static const std::array<sf::Vector2i, 4>& neighbourhood()
    {
        static const std::array<sf::Vector2i, 4> directions{{
            sf::Vector2i{0, -1}, 
            sf::Vector2i{0, 1},
            sf::Vector2i{-1, 0}, 
            sf::Vector2i{1, 0}
        }};
        return directions;
    }

    Grid(int gridWidth, int gridHeight)
        : cols(gridWidth / static_cast<int>(Cell::CELL_SIZE)),
          rows(gridHeight / static_cast<int>(Cell::CELL_SIZE))
    {
        if (cols <= 0 || rows <= 0)
        {
            throw std::invalid_argument("Grid: size too short for a single cell");
        }

        cells.reserve(static_cast<std::size_t>(cols) * static_cast<std::size_t>(rows));
        for (int y = 0; y < rows; ++y)
        {
            for (int x = 0; x < cols; ++x)
            {
                cells.emplace_back(x, y);
            }
        }
    }

    Grid(const Grid&) = delete;
    Grid& operator=(const Grid&) = delete;

    //* ---- Dimensions ----

    int getCols() const noexcept { return cols; }
    int getRows() const noexcept { return rows; }

    float pixelWidth() const noexcept { return static_cast<float>(cols) * Cell::CELL_SIZE; }
    float pixelHeight() const noexcept { return static_cast<float>(rows) * Cell::CELL_SIZE; }

    bool bounds(sf::Vector2i position) const noexcept
    {
        return position.x >= 0 && position.x < cols && position.y >= 0 && position.y < rows;
    }

    // ---- Cells Management ----

    Cell& getCellAt(sf::Vector2i position)
    {
        if (!bounds(position))
        {
            throw std::out_of_range("Grid::getCellAt out of limits");
        } 
        return cells[index(position)];
    }

    const Cell& getCellAt(sf::Vector2i position) const
    {
        if (!bounds(position))
        {
            throw std::out_of_range("Grid::getCellAt out of limits");
        } 
        return cells[index(position)];
    }

    Entity* entityAt(sf::Vector2i position) const noexcept
    {
        return bounds(position) ? cells[index(position)].getContent() : nullptr;
    }

    bool isFree(sf::Vector2i position) const noexcept
    {
        return bounds(position) && cells[index(position)].isEmpty();
    }

    //* Birth / Movement / Death
    // *----------------------------------------------------------------------------

    template <typename T, typename... Args>
    T* spawn(sf::Vector2i position, Args&&... args)
    {
        if (!isFree(position)) return nullptr;

        auto owned = std::make_unique<T>(std::forward<Args>(args)...);
        T* raw = owned.get();
        ++birthCount;
        if (raw->species() == Species::Herbivore) 
        {
            ++herbivoreCount;
        }

        if (raw->species() == Species::Carnivore) {
            ++carnivoreCount;
        }

        entities.push_back(std::move(owned));
        placeAt(position, raw);
        return raw;
    }

    void placeAt(sf::Vector2i position, Entity* entity)
    {
        if (!bounds(position)) return;

        cells[index(position)].setContent(entity);
        if (entity) entity->setPosition(position);
    }

    void moveEntity(sf::Vector2i from, sf::Vector2i to)
    {
        if (from == to) return;
        if (!bounds(from) || !isFree(to)) return;

        Entity* entity = cells[index(from)].clearContent();
        placeAt(to, entity);
    }


    [[nodiscard]] std::optional<sf::Vector2i> randomFreeNeighbour(sf::Vector2i origin) const
    {
        std::array<sf::Vector2i, 4> freeCells{};
        int count = 0;

        for (const sf::Vector2i& direction : neighbourhood())
        {
            const sf::Vector2i candidate = origin + direction;
            if (isFree(candidate)) freeCells[static_cast<std::size_t>(count++)] = candidate;
        }

        if (count == 0) return std::nullopt;
        return freeCells[static_cast<std::size_t>(Random::inRange(0, count - 1))];
    }

    void step()
    {
        // Froze the number of entities before the loop:
        // Babies will be added at the of the vector and will act

        const std::size_t actorCount = entities.size();

        for (std::size_t i = 0; i < actorCount; ++i)
        {
            Entity* entity = entities[i].get();
            if (!entity->isAlive()) continue;

            entity->update();
            entity->move(*this);
            entity->reproduce(*this);
        }

        collectDead();
    }

    // Remove dead entities
    void collectDead()
    {
        for (const std::unique_ptr<Entity>& entity : entities)
        {
            if (entity->isAlive()) continue;

            ++deathCount;
            if (entity->species() == Species::Herbivore) 
            {
                --herbivoreCount;
            }

            const sf::Vector2i position = entity->getPosition();
            if (bounds(position) && cells[index(position)].getContent() == entity.get())
            {
                cells[index(position)].clearContent();
            }
        }

        entities.erase(
            std::remove_if(entities.begin(), entities.end(),
                           [](const std::unique_ptr<Entity>& entity) { return !entity->isAlive(); }),
            entities.end());
    }

    //* ---- Stats ----

    std::size_t population() const noexcept { return entities.size(); }
    std::size_t births() const noexcept { return birthCount; }
    std::size_t deaths() const noexcept { return deathCount; }
    std::size_t herbivorePopulation() const noexcept { return herbivoreCount; }

    std::size_t populationOf(Species species) const noexcept
    {
        if (species == Species::Herbivore) return herbivoreCount;

        return static_cast<std::size_t>(
            std::count_if(entities.begin(), entities.end(),
                          [species](const std::unique_ptr<Entity>& entity)
                          { return entity->species() == species; }));
    }

    [[nodiscard]] const std::vector<std::unique_ptr<Entity>>& getEntities() const noexcept { return entities; }

    //* DEBUG
    friend std::ostream& operator<<(std::ostream& os, const Grid& grid)
    {
        for (int y = 0; y < grid.rows; ++y)
        {
            for (int x = 0; x < grid.cols; ++x)
            {
                os << (grid.cells[grid.index(sf::Vector2i{x, y})].isEmpty() ? ". " : "X ");
            }
            os << '\n';
        }
        return os;
    }
};

//! Dont touch pls
inline void Entity::move(Grid& grid) const {
    const sf::Vector2i target = chooseDirection(grid);
    if (target != position && grid.bounds(target))
    {
        grid.moveEntity(position, target);
    }
}

#endif // GRID_H