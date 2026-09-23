#ifndef APPLICATION_H
#define APPLICATION_H

#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>

#include <SFML/Graphics.hpp>

#include "Grid.h"
#include "Renderer.h"
#include "tools.h"

/*
 * Application handle the main loop of the game: event -> simulation -> render
 * Clase handles framerate and keys input
 */

class Application
{
    std::ofstream csvFile{"simulation_data.csv"};
    int timeStep{0};
    Grid& grid;
    Renderer renderer;
    sf::RenderWindow window;
    sf::View gameView;
    sf::Time tickDuration;
    bool paused = false;

    static constexpr float MAX_FRAME_TIME = 0.25f;

public:
    explicit Application(Grid& simulationGrid, float ticksPerSecond = 10.f, const std::string& title = "RTS window")
        : grid(simulationGrid), renderer(simulationGrid), window(sf::VideoMode(initialWindowSize(simulationGrid)), title),
          tickDuration(sf::seconds(1.f / ticksPerSecond))
    {
        configureView(window.getSize());
        window.setFramerateLimit(60);
        if (!csvFile.is_open())
        {
            std::cerr << "Impossible d'ouvrir simulation_data.csv\n";
        }
        else
        {
            writeCsvHeader(csvFile);
            appendCsvLine(csvFile, timeStep, grid);
        }
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
                if (!paused)
                {
                    grid.step();
                    appendCsvLine(csvFile, ++timeStep, grid);
                }
                accumulator -= tickDuration;
            }

            window.clear(sf::Color(30, 30, 35));
            renderer.draw(window, grid);
            window.display();
        }
    }

    void setFPSMax(int fps)
    {
        window.setFramerateLimit(fps);
    }

private:
    static sf::Vector2u initialWindowSize(const Grid& simulationGrid)
    {
        const sf::Vector2u desktopSize = sf::VideoMode::getDesktopMode().size;
        const float availableWidth = static_cast<float>(desktopSize.x) * 0.9f;
        const float availableHeight = static_cast<float>(desktopSize.y) * 0.9f;
        const float scale = std::min({1.f, availableWidth / simulationGrid.pixelWidth(),
                                      availableHeight / simulationGrid.pixelHeight()});

        return sf::Vector2u{
            std::max(1u, static_cast<unsigned int>(simulationGrid.pixelWidth() * scale)),
            std::max(1u, static_cast<unsigned int>(simulationGrid.pixelHeight() * scale))};
    }

    void configureView(sf::Vector2u windowSize)
    {
        if (windowSize.x == 0 || windowSize.y == 0) return;

        const float worldRatio = grid.pixelWidth() / grid.pixelHeight();
        const float windowRatio = static_cast<float>(windowSize.x) / static_cast<float>(windowSize.y);
        sf::FloatRect viewport({0.f, 0.f}, {1.f, 1.f});

        if (windowRatio > worldRatio)
        {
            viewport.size.x = worldRatio / windowRatio;
            viewport.position.x = (1.f - viewport.size.x) * 0.5f;
        }
        else if (windowRatio < worldRatio)
        {
            viewport.size.y = windowRatio / worldRatio;
            viewport.position.y = (1.f - viewport.size.y) * 0.5f;
        }

        gameView.setSize(sf::Vector2f(grid.pixelWidth(), grid.pixelHeight()));
        gameView.setCenter(sf::Vector2f(grid.pixelWidth() * 0.5f, grid.pixelHeight() * 0.5f));
        gameView.setViewport(viewport);
        window.setView(gameView);
    }

    void handleEvents()
    {
        while (const std::optional<sf::Event> event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                window.close();
                return;
            }

            if (const auto* resized = event->getIf<sf::Event::Resized>())
            {
                configureView(resized->size);
            }

            if (const auto* key = event->getIf<sf::Event::KeyPressed>())
            {
                if (key->code == sf::Keyboard::Key::Escape) window.close();
                if (key->code == sf::Keyboard::Key::Space) paused = !paused;
            }
        }
    }
};

#endif // APPLICATION_H
