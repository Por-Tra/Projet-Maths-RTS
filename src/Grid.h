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

#if defined(_MSC_VER) && (defined(_M_X64) || defined(_M_IX86))
#include <xmmintrin.h>
#endif

class Grid
{
    int cols{0};
    int rows{0};

    std::vector<Cell> cells;
    std::vector<std::unique_ptr<Entity>> entities;
    std::size_t birthCount{0};
    std::size_t deathCount{0};
    // Indexes par Species : herbivores, carnivores, plantes.
    std::array<std::size_t, 3> birthsBySpecies{};
    std::array<std::size_t, 3> deathsBySpecies{};
    std::size_t herbivoreCount{0};
    std::size_t carnivoreCount{0};

    [[nodiscard]] std::size_t index(sf::Vector2i position) const noexcept
    {
        return static_cast<std::size_t>(position.y) * static_cast<std::size_t>(cols)
             + static_cast<std::size_t>(position.x);
    }

    template<class Visitor>
    bool visitNeighbourCells(sf::Vector2i origin, Visitor&& visit) const
    {
        if (bounds(origin))
        {
            const std::size_t cellIndex = index(origin);
            const auto stride = static_cast<std::size_t>(cols);
            // Preserve the up/down/left/right order and stop on the first match.
            if (origin.y > 0 && visit(cells[cellIndex - stride], 0)) return true;
            if (origin.y + 1 < rows && visit(cells[cellIndex + stride], 1)) return true;
            if (origin.x > 0 && visit(cells[cellIndex - 1], 2)) return true;
            if (origin.x + 1 < cols && visit(cells[cellIndex + 1], 3)) return true;
        }
        else
        {
            for (int i = 0; i < 4; ++i)
            {
                const auto candidate = origin + neighbourhood()[i];
                if (bounds(candidate) && visit(cells[index(candidate)], i)) return true;
            }
        }
        return false;
    }

    static void prefetchCell(const Cell* cell) noexcept
    {
#if defined(_MSC_VER) && (defined(_M_X64) || defined(_M_IX86))
        _mm_prefetch(reinterpret_cast<const char*>(cell), _MM_HINT_T0);
#elif defined(__GNUC__) || defined(__clang__)
        __builtin_prefetch(cell, 0, 3);
#else
        (void)cell;
#endif
    }

    void prefetchNeighbourhood(sf::Vector2i origin) const noexcept
    {
        if (!bounds(origin)) return;
        const auto cellIndex = index(origin);
        const auto stride = static_cast<std::size_t>(cols);
        prefetchCell(&cells[cellIndex]);
        if (origin.y > 0) prefetchCell(&cells[cellIndex - stride]);
        if (origin.y + 1 < rows) prefetchCell(&cells[cellIndex + stride]);
        if (origin.x > 0) prefetchCell(&cells[cellIndex - 1]);
        if (origin.x + 1 < cols) prefetchCell(&cells[cellIndex + 1]);
    }

public:
    //* Von Neumann Neighborhood (4 directions). Shared with all entities.
    static const std::array<sf::Vector2i, 4>& neighbourhood()
    {
        static constexpr std::array<sf::Vector2i, 4> directions{{
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

    [[nodiscard]] int getCols() const noexcept { return cols; }
    [[nodiscard]] int getRows() const noexcept { return rows; }

    [[nodiscard]] float pixelWidth() const noexcept { return static_cast<float>(cols) * Cell::CELL_SIZE; }
    [[nodiscard]] float pixelHeight() const noexcept { return static_cast<float>(rows) * Cell::CELL_SIZE; }

    [[nodiscard]] bool bounds(sf::Vector2i position) const noexcept
    {
        // Negative coordinates also fail these unsigned comparisons.
        return static_cast<unsigned>(position.x) < static_cast<unsigned>(cols)
            && static_cast<unsigned>(position.y) < static_cast<unsigned>(rows);
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

    [[nodiscard]] const Cell& getCellAt(sf::Vector2i position) const
    {
        if (!bounds(position))
        {
            throw std::out_of_range("Grid::getCellAt out of limits");
        } 
        return cells[index(position)];
    }

    [[nodiscard]] Entity* entityAt(sf::Vector2i position) const noexcept
    {
        return bounds(position) ? cells[index(position)].getContent() : nullptr;
    }

    [[nodiscard]] bool isFree(sf::Vector2i position) const noexcept
    {
        return bounds(position) && cells[index(position)].isEmpty();
    }

    template<class Visitor>
    bool visitOccupiedNeighbours(sf::Vector2i origin, Visitor&& visit) const
    {
        return visitNeighbourCells(origin, [&](const Cell& cell, int) {
            Entity* entity = cell.getContent();
            return entity != nullptr && visit(entity);
        });
    }

    //* Birth / Movement / Death
    // *----------------------------------------------------------------------------

    void reserveEntities(std::size_t count) { entities.reserve(count); }

    template <typename T, typename... Args>
    T* spawn(sf::Vector2i position, Args&&... args)
    {
        if (!isFree(position)) return nullptr;

        auto owned = std::make_unique<T>(std::forward<Args>(args)...);
        T* raw = owned.get();
        const Species species = raw->species();
        entities.push_back(std::move(owned));
        ++birthCount;
        ++birthsBySpecies[static_cast<std::size_t>(species)];
        if (species == Species::Herbivore)
        {
            ++herbivoreCount;
        }

        else if (species == Species::Carnivore) {
            ++carnivoreCount;
        }

        // isFree above has already validated this cell.
        cells[index(position)].setContent(raw);
        raw->setPosition(position);
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
        // isFree above has already validated the destination.
        cells[index(to)].setContent(entity);
        if (entity) entity->setPosition(to);
    }


    [[nodiscard]] std::optional<sf::Vector2i> randomFreeNeighbour(sf::Vector2i origin) const
    {
        std::array<int, 4> freeDirections;
        int count = 0;

        visitNeighbourCells(origin, [&](const Cell& cell, int direction) {
            if (cell.isEmpty()) freeDirections[count++] = direction;
            return false;
        });

        if (count == 0) return std::nullopt;
        return origin + neighbourhood()[freeDirections[Random::inRange(0, count - 1)]];
    }

    void step()
    {
        // Freeze the actor count: newborns first act on the next step.

        const std::size_t actorCount = entities.size();
        constexpr std::size_t PREFETCH_DISTANCE = 16;
        // Small grids already fit in cache; look ahead only on large worlds.
        const bool prefetch = cells.size() > 1024 * 1024;

        for (std::size_t i = 0; i < actorCount; ++i)
        {
            if (prefetch && actorCount - i > PREFETCH_DISTANCE)
                prefetchNeighbourhood(entities[i + PREFETCH_DISTANCE]->getPosition());
            Entity* entity = entities[i].get();
            if (!entity->isAlive()) continue;

            entity->update();
            if (!entity->isAlive()) continue;

            entity->move(*this);
            entity->reproduce(*this);
            entity->killingHerbivores(*this, false);
        }

        collectDead(); //& step of killing
    }

    // Remove dead entities
    void collectDead()
    {
        auto write = entities.begin();  // Write pointer for the next alive entity
        for (auto read = entities.begin(); read != entities.end(); ++read)
        {
            std::unique_ptr<Entity>& entity = *read;    // Reference to the current entity
            if (!entity->isAlive())
            {
                ++deathCount;
                const Species species = entity->species();
                ++deathsBySpecies[static_cast<std::size_t>(species)];
                if (species == Species::Herbivore)
                {
                    --herbivoreCount;
                }
                else if (species == Species::Carnivore)
                {
                    --carnivoreCount;
                }

                const sf::Vector2i position = entity->getPosition();
                if (bounds(position) && cells[index(position)].getContent() == entity.get())
                {
                    cells[index(position)].clearContent();
                }
                continue;
            }

            if (write != read) *write = std::move(entity);
            ++write;
        }

        entities.erase(write, entities.end());
    }

    //* ---- Stats ----

    [[nodiscard]] std::size_t population() const noexcept { return entities.size(); }
    [[nodiscard]] std::size_t births() const noexcept { return birthCount; }
    [[nodiscard]] std::size_t deaths() const noexcept { return deathCount; }
    [[nodiscard]] std::size_t birthsOf(Species species) const noexcept
    {
        return birthsBySpecies[static_cast<std::size_t>(species)];
    }
    [[nodiscard]] std::size_t deathsOf(Species species) const noexcept
    {
        return deathsBySpecies[static_cast<std::size_t>(species)];
    }
    [[nodiscard]] std::size_t herbivorePopulation() const noexcept { return herbivoreCount; }
    [[nodiscard]] std::size_t carnivorePopulation() const noexcept { return carnivoreCount; }

    [[nodiscard]] std::size_t populationOf(Species species) const noexcept
    {
        if (species == Species::Herbivore) return herbivoreCount;
        if (species == Species::Carnivore) return carnivoreCount;

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

inline void Entity::move(Grid& grid) const {
    const sf::Vector2i target = chooseDirection(grid);
    // moveEntity already validates the source and destination.
    grid.moveEntity(position, target);
}

#endif // GRID_H
