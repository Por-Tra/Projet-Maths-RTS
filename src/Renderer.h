#ifndef RENDERER_H
#define RENDERER_H

#include <cmath>
#include <memory>

#include <SFML/Graphics.hpp>

#include "Cell.h"
#include "Entity.h"
#include "Grid.h"

//* Tout le dessin est ici. La simulation ne connait pas SFML, le rendu ne modifie
//* pas la simulation.
//*
//* Performance : ton ancienne boucle faisait un window.draw() par cellule, soit
//* 1089 appels de dessin par frame (chacun = un changement d'etat + un envoi au GPU),
//* et zero appel pour les entites. Ici tout est empaquete dans des sf::VertexArray :
//*   - le fond      : 1 draw
//*   - la grille    : 1 draw (les lignes, construites une fois pour toutes)
//*   - les entites  : 1 draw (reconstruit chaque frame, O(nombre d'entites))
//* Soit 3 appels de dessin quelle que soit la taille de la grille.
class Renderer
{
    sf::RectangleShape background;
    sf::VertexArray gridLines{sf::PrimitiveType::Lines};
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
        window.draw(background);
        window.draw(gridLines);

        //! LE BUG D'AFFICHAGE : dans ton code, cette partie n'existait pas.
        //! La boucle ne dessinait que cell->cell. Les entites n'etaient JAMAIS
        //! envoyees a la fenetre, donc invisibles meme si la simulation tournait.
        buildEntityGeometry(grid);
        window.draw(entityTriangles);
    }

private:
    void buildStaticGeometry(const Grid& grid)
    {
        const float width = grid.pixelWidth();
        const float height = grid.pixelHeight();

        background.setSize(sf::Vector2f(width, height));
        background.setFillColor(sf::Color::White);
        background.setPosition(sf::Vector2f(0.f, 0.f));

        const sf::Color lineColor(190, 190, 190);

        gridLines.clear();
        for (int x = 0; x <= grid.getCols(); ++x)
        {
            const float px = static_cast<float>(x) * Cell::CELL_SIZE;
            gridLines.append(sf::Vertex{sf::Vector2f(px, 0.f), lineColor});
            gridLines.append(sf::Vertex{sf::Vector2f(px, height), lineColor});
        }
        for (int y = 0; y <= grid.getRows(); ++y)
        {
            const float py = static_cast<float>(y) * Cell::CELL_SIZE;
            gridLines.append(sf::Vertex{sf::Vector2f(0.f, py), lineColor});
            gridLines.append(sf::Vertex{sf::Vector2f(width, py), lineColor});
        }
    }

    void buildEntityGeometry(const Grid& grid)
    {
        entityTriangles.clear();

        for (const std::unique_ptr<Entity>& entity : grid.getEntities())
        {
            const sf::Vector2i cell = entity->getPosition();

            //! DEUXIEME BUG D'AFFICHAGE : dans ton Entity::setPosition tu faisais
            //!     shape.setPosition(sf::Vector2f(newPosition.x, newPosition.y));
            //! avec des coordonnees de GRILLE (0..32), donc toutes les entites se
            //! retrouvaient empilees dans le coin haut-gauche sur ~30 pixels.
            //! Il faut convertir case -> pixels, et centrer dans la case.
            const sf::Vector2f center(
                (static_cast<float>(cell.x) + 0.5f) * Cell::CELL_SIZE,
                (static_cast<float>(cell.y) + 0.5f) * Cell::CELL_SIZE);

            appendCircle(center, Cell::CELL_SIZE * RADIUS_RATIO, entity->color());
        }
    }

    // Approxime un disque par un eventail de triangles, ajoute au lot commun.
    void appendCircle(sf::Vector2f center, float radius, sf::Color color)
    {
        constexpr float TWO_PI = 6.28318530718f;

        sf::Vector2f previous(center.x + radius, center.y);

        for (int i = 1; i <= CIRCLE_SEGMENTS; ++i)
        {
            const float angle = TWO_PI * static_cast<float>(i) / static_cast<float>(CIRCLE_SEGMENTS);
            const sf::Vector2f current(center.x + radius * std::cos(angle),
                                       center.y + radius * std::sin(angle));

            entityTriangles.append(sf::Vertex{center, color});
            entityTriangles.append(sf::Vertex{previous, color});
            entityTriangles.append(sf::Vertex{current, color});

            previous = current;
        }
    }
};

#endif // RENDERER_H
