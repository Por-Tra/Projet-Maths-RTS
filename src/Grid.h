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

//* La Grid est le MODELE de simulation. Elle n'ouvre plus de fenetre et ne dessine
//* plus rien : voir Renderer.h et Application.h.
//*
//* Deux structures complementaires :
//*   - `cells`    : index spatial "qui est ou ?" -> acces O(1) par coordonnees
//*   - `entities` : liste des etres vivants      -> iteration O(N) par tick
//*
//* C'est ce qui remplace ton double for sur toute la grille. Avec 33x33 = 1089 cases
//* et 10 herbivores, l'ancienne boucle faisait 1089 iterations pour 10 entites utiles :
//* 99 % du travail etait du vide. Maintenant on itere sur les 10.
class Grid
{
    int cols{0}; // nombre de colonnes = x
    int rows{0}; // nombre de lignes   = y

    //* Vecteur PLAT, indexe par y * cols + x.
    //* Avant : std::vector<std::vector<Cell*>> = 1 + rows allocations, memoire eparpillee,
    //* deux dereferencements de pointeur par acces, et un `delete` manuel par cellule.
    //* Maintenant : une seule allocation contigue, tres favorable au cache CPU, et zero delete.
    std::vector<Cell> cells;

    //* La grille est PROPRIETAIRE des entites (unique_ptr = liberation automatique).
    //* Avant, les `new Herbivore` n'etaient jamais liberes : fuite memoire a chaque naissance.
    std::vector<std::unique_ptr<Entity>> entities;

    std::size_t index(sf::Vector2i position) const noexcept
    {
        return static_cast<std::size_t>(position.y) * static_cast<std::size_t>(cols)
             + static_cast<std::size_t>(position.x);
    }

public:
    //* Voisinage de von Neumann (4 directions). Partage par toutes les entites.
    static const std::array<sf::Vector2i, 4>& neighbourhood()
    {
        static const std::array<sf::Vector2i, 4> directions{{
            sf::Vector2i{0, -1}, sf::Vector2i{0, 1},
            sf::Vector2i{-1, 0}, sf::Vector2i{1, 0}
        }};
        return directions;
    }

    Grid(int gridWidth, int gridHeight)
        : cols(gridWidth / static_cast<int>(Cell::CELL_SIZE)),
          rows(gridHeight / static_cast<int>(Cell::CELL_SIZE))
    {
        if (cols <= 0 || rows <= 0)
        {
            throw std::invalid_argument("Grid: dimensions trop petites pour une seule cellule");
        }

        //* Initialisation UNE SEULE FOIS, dans le constructeur.
        //* Ton bug : le constructeur appelait initGridVector(), puis run() le rappelait,
        //* ce qui empilait une deuxieme grille de lignes a la suite de la premiere
        //* (gridVector faisait 2 * rows lignes, dont la moitie invisible et fuitee).
        cells.reserve(static_cast<std::size_t>(cols) * static_cast<std::size_t>(rows));
        for (int y = 0; y < rows; ++y)
        {
            for (int x = 0; x < cols; ++x)
            {
                cells.emplace_back(x, y);
            }
        }
    }

    // unique_ptr gere tout : pas de destructeur manuel, pas de fuite, pas de double free.
    Grid(const Grid&) = delete;
    Grid& operator=(const Grid&) = delete;

    //* Dimensions
    //* -----------------------------------------------------------------------------------------------

    int getCols() const noexcept { return cols; }
    int getRows() const noexcept { return rows; }

    float pixelWidth() const noexcept { return static_cast<float>(cols) * Cell::CELL_SIZE; }
    float pixelHeight() const noexcept { return static_cast<float>(rows) * Cell::CELL_SIZE; }

    bool bounds(sf::Vector2i position) const noexcept
    {
        return position.x >= 0 && position.x < cols
            && position.y >= 0 && position.y < rows;
    }

    //* Acces aux cases
    //* -----------------------------------------------------------------------------------------------

    //! getCellAt renvoie maintenant une REFERENCE, plus un pointeur.
    //! Migration : `grid.getCellAt(p)->isEmpty()` devient `grid.getCellAt(p).isEmpty()`
    Cell& getCellAt(sf::Vector2i position)
    {
        if (!bounds(position)) throw std::out_of_range("Grid::getCellAt out of limits");
        return cells[index(position)];
    }

    const Cell& getCellAt(sf::Vector2i position) const
    {
        if (!bounds(position)) throw std::out_of_range("Grid::getCellAt out of limits");
        return cells[index(position)];
    }

    // Version sans exception, pratique dans les boucles : renvoie nullptr hors limites.
    Entity* entityAt(sf::Vector2i position) const noexcept
    {
        return bounds(position) ? cells[index(position)].getContent() : nullptr;
    }

    // "Libre" = dans la grille ET vide. Les deux tests d'un coup, c'est ce qu'on veut 99 % du temps.
    bool isFree(sf::Vector2i position) const noexcept
    {
        return bounds(position) && cells[index(position)].isEmpty();
    }

    //* Naissances / deplacements / morts
    //* -----------------------------------------------------------------------------------------------

    //* Fabrique + place + prend possession de l'entite, en une seule operation.
    //* Exemple : grid.spawn<Herbivore>({10, 10}, "Bunny");
    //* Renvoie nullptr si la case est occupee ou hors grille.
    template <typename T, typename... Args>
    T* spawn(sf::Vector2i position, Args&&... args)
    {
        if (!isFree(position)) return nullptr;

        auto owned = std::make_unique<T>(std::forward<Args>(args)...);
        T* raw = owned.get();
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

    //* Ancien moveCellToPosition : il ecrasait sans verifier. Si la case d'arrivee
    //* etait occupee, l'entite qui s'y trouvait disparaissait de la grille tout en
    //* restant vivante -> fuite + entite fantome. Ici on refuse le deplacement.
    void moveEntity(sf::Vector2i from, sf::Vector2i to)
    {
        if (from == to) return;
        if (!bounds(from) || !isFree(to)) return;

        Entity* entity = cells[index(from)].clearContent();
        placeAt(to, entity);
    }

    //* Une case libre au hasard parmi les 4 voisines, sans allocation.
    //* Avant, reproduce() parcourait TOUTE la grille pour construire un std::vector
    //* de cases vides, a chaque naissance : O(cols * rows) + une allocation dynamique.
    //* Ici c'est O(4) sur la pile.
    std::optional<sf::Vector2i> randomFreeNeighbour(sf::Vector2i origin) const
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

    //* Un tour de simulation
    //* -----------------------------------------------------------------------------------------------
    void step()
    {
        //* On fige le nombre d'entites AVANT la boucle : les bebes nes pendant ce tick
        //* sont ajoutes a la fin du vecteur et n'agiront qu'au tick suivant.
        //*
        //* Bonus, et c'est important : avec l'ancien double for, une entite qui se
        //* deplacait vers le bas ou la droite etait revisitee dans le MEME tick et
        //* jouait son tour plusieurs fois. Ici chaque entite a un index stable :
        //* exactement un update/move/reproduce par tick, quoi qu'elle fasse.
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

    //* Retire les entites mortes de la grille puis du vecteur (une seule passe de compactage).
    void collectDead()
    {
        for (const std::unique_ptr<Entity>& entity : entities)
        {
            if (entity->isAlive()) continue;

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

    //* Statistiques
    //* -----------------------------------------------------------------------------------------------

    std::size_t population() const noexcept { return entities.size(); }

    //* O(nombre d'entites) au lieu de O(cols * rows) + un dynamic_cast par case.
    std::size_t populationOf(Species species) const noexcept
    {
        return static_cast<std::size_t>(
            std::count_if(entities.begin(), entities.end(),
                          [species](const std::unique_ptr<Entity>& entity)
                          { return entity->species() == species; }));
    }

    // Lecture seule, pour le rendu et les outils.
    const std::vector<std::unique_ptr<Entity>>& getEntities() const noexcept { return entities; }

    //* DEBUG
    friend std::ostream& operator<<(std::ostream& os, const Grid& grid)
    {
        // rows/cols sont des int : on boucle en int, plus de warning signed/unsigned.
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
inline void Entity::move(Grid& grid)
{
    const sf::Vector2i target = chooseDirection(grid);
    if (target != position && grid.bounds(target))
    {
        grid.moveEntity(position, target);
    }
}

#endif // GRID_H