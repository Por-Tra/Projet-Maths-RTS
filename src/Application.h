#ifndef APPLICATION_H
#define APPLICATION_H

#include <string>

#include <SFML/Graphics.hpp>

#include "Grid.h"
#include "Renderer.h"

//* Boucle principale : evenements -> simulation -> rendu.
//*
//* Point cle : le PAS DE SIMULATION est decorrele du FRAMERATE (fixed timestep).
//* Dans ton code la simulation aurait tourne aussi vite que la carte graphique
//* (des centaines de tours par seconde), rendant tout illisible et dependant
//* de la machine. Ici la grille avance a N tours/seconde, quel que soit le PC,
//* et l'affichage tourne a 60 FPS.
class Application
{
    Grid& grid;
    Renderer renderer;
    sf::RenderWindow window;
    sf::Time tickDuration;
    bool paused = false;

    static constexpr float MAX_FRAME_TIME = 0.25f; // garde-fou anti "spirale de la mort"

public:
    explicit Application(Grid& simulationGrid,
                         float ticksPerSecond = 10.f,
                         const std::string& title = "Cellular Automata")
        : grid(simulationGrid),
          renderer(simulationGrid),
          window(sf::VideoMode(sf::Vector2u(static_cast<unsigned int>(simulationGrid.pixelWidth()),
                                            static_cast<unsigned int>(simulationGrid.pixelHeight()))),
                 title),
          tickDuration(sf::seconds(1.f / ticksPerSecond))
    {
        window.setFramerateLimit(60);
    }

    void run()
    {
        sf::Clock clock;
        sf::Time accumulator = sf::Time::Zero;

        while (window.isOpen())
        {
            handleEvents();

            accumulator += clock.restart();
            if (accumulator > sf::seconds(MAX_FRAME_TIME)) accumulator = sf::seconds(MAX_FRAME_TIME);

            while (accumulator >= tickDuration)
            {
                if (!paused) grid.step();
                accumulator -= tickDuration;
            }

            window.clear(sf::Color(30, 30, 35));
            renderer.draw(window, grid);
            window.display();
        }
    }

private:
    void handleEvents()
    {
        while (const std::optional<sf::Event> event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                window.close();
                return;
            }

            if (const sf::Event::KeyPressed* key = event->getIf<sf::Event::KeyPressed>())
            {
                if (key->code == sf::Keyboard::Key::Escape) window.close();
                if (key->code == sf::Keyboard::Key::Space) paused = !paused;
            }
        }
    }
};

#endif // APPLICATION_H
