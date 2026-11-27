#ifndef STATS_PANEL_H
#define STATS_PANEL_H

#include <algorithm>
#include <array>
#include <cstddef>
#include <iomanip>
#include <locale>
#include <sstream>
#include <string>

#include <SFML/Graphics.hpp>

#include "Grid.h"
#include "ProjectFont.h"

// Taux observes : naissances ou deces / population exposee, en % par pas.
// La fenetre glissante evite qu'un seul pas rende les taux trop instables.
class PopulationStats
{
public:
    static constexpr std::size_t WINDOW_STEPS = 50;

    PopulationStats(const Grid& grid, Species species)
        : species(species), lastPopulation(grid.populationOf(species)), lastBirths(grid.birthsOf(species)),
         lastDeaths(grid.deathsOf(species))
    {

    }

    void update(const Grid& grid)   //& Mise a jour des statistiques pour la fenetre glissante.
    {
        const std::size_t births = grid.birthsOf(species);
        const std::size_t deaths = grid.deathsOf(species);
        Interval& oldest = history[next];
        birthSum -= oldest.births;
        deathSum -= oldest.deaths;
        populationSum -= oldest.population;

        oldest = {births - lastBirths, deaths - lastDeaths, lastPopulation};
        birthSum += oldest.births;
        deathSum += oldest.deaths;
        populationSum += oldest.population;
        next = (next + 1) % WINDOW_STEPS;
        steps = std::min(steps + 1, WINDOW_STEPS);

        lastPopulation = grid.populationOf(species);
        lastBirths = births;
        lastDeaths = deaths;
    }

    //* Accesseurs pour les statistiques observees.
    std::size_t population() const noexcept { return lastPopulation; }
    std::size_t observedSteps() const noexcept { return steps; }
    double reproductionRate() const noexcept { return rate(birthSum); }
    double mortalityRate() const noexcept { return rate(deathSum); }

private:
    struct Interval
    {
        std::size_t births{0};
        std::size_t deaths{0};
        std::size_t population{0};
    };

    Species species;
    std::array<Interval, WINDOW_STEPS> history{};
    std::size_t next{0};
    std::size_t steps{0};
    std::size_t birthSum{0};
    std::size_t deathSum{0};
    std::size_t populationSum{0};
    std::size_t lastPopulation;
    std::size_t lastBirths;
    std::size_t lastDeaths;

    double rate(std::size_t events) const noexcept
    {
        return populationSum == 0 ? 0.0 : 100.0 * static_cast<double>(events) / static_cast<double>(populationSum);
    }
};

// Surimpression en pixels : independante du zoom et du cadrage de la simulation.
class StatsPanel
{
public:
    explicit StatsPanel(const Grid& grid, const std::string& fontPath = "")
        : herbivores(grid, Species::Herbivore), carnivores(grid, Species::Carnivore),
          totalPopulation(grid.population()), hasFont(ProjectFont::load(font, fontPath))
    {
        label.setFillColor(sf::Color::Black);
        background.setFillColor(sf::Color::White);
    }

    // Appele une fois apres chaque pas, y compris quand un autre onglet est visible.
    void update(const Grid& grid)
    {
        herbivores.update(grid);
        carnivores.update(grid);
        totalPopulation = grid.population();
        dirty = true;
    }

    void draw(sf::RenderTarget& target, int timeStep, bool paused)
    {
        const auto size = target.getSize();
        if (!hasFont || size.x == 0 || size.y == 0) return;
        refreshText(timeStep, paused);

        //* UI elements
        const sf::Vector2f screenSize{static_cast<float>(size.x), static_cast<float>(size.y)};
        const float margin = std::min(12.f, std::min(screenSize.x, screenSize.y) * .05f);
        const float padding = 12.f;
        const auto bounds = label.getLocalBounds();
        const sf::Vector2f panelSize{bounds.size.x + 2.f * padding, bounds.size.y + 2.f * padding};
        const float scale = std::min({1.f, (screenSize.x - 2.f * margin) / panelSize.x, (screenSize.y - 2.f * margin) / panelSize.y});
        
        
        background.setSize(panelSize);
        background.setScale({scale, scale});
        background.setPosition({margin, margin});

        label.setOrigin(bounds.position);
        label.setScale({scale, scale});
        label.setPosition({margin + padding * scale, margin + padding * scale});

        const sf::View saved = target.getView();
        target.setView(sf::View(sf::FloatRect({0.f, 0.f}, screenSize)));
        target.draw(background);
        target.draw(label);
        target.setView(saved);
    }

private:
    PopulationStats herbivores;
    PopulationStats carnivores;
    std::size_t totalPopulation;
    sf::Font font;
    bool hasFont;
    sf::Text label{font, "", 14};
    sf::RectangleShape background;
    bool dirty{true};
    int displayedStep{-1};
    bool displayedPaused{false};

    static void appendPopulation(std::ostringstream& out, const char* name, const PopulationStats& stats)
    {
        out << name << " : " << stats.population() << '\n'
            << "  Reproduction : " << stats.reproductionRate() << " % / pas\n"
            << "  Mortalite : " << stats.mortalityRate() << " % / pas\n";
    }

    void refreshText(int timeStep, bool paused)
    {
        if (!dirty && displayedStep == timeStep && displayedPaused == paused) return;

        std::ostringstream out;
        out.imbue(std::locale::classic());

        out << std::fixed << std::setprecision(2)
            << "Simulation - pas " << timeStep << (paused ? " (pause)" : "") << '\n'
            << "Population totale : " << totalPopulation << "\n\n";
            
        appendPopulation(out, "Herbivores", herbivores);
        out << '\n';
        appendPopulation(out, "Carnivores", carnivores);
        out << "\nTaux moyens sur " << herbivores.observedSteps() << " pas";
        label.setString(out.str());

        displayedStep = timeStep;
        displayedPaused = paused;
        dirty = false;
    }
};

#endif // STATS_PANEL_H
