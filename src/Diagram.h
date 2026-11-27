#ifndef DIAGRAM_H
#define DIAGRAM_H

#include <algorithm>
#include <cmath>
#include <limits>
#include <string>
#include <vector>

#include "Grid.h"
#include "Plot.h"

// Diagramme de phase : herbivores en abscisse, carnivores en ordonnee.
class Diagram
{
public:
    explicit Diagram(Grid& simulationGrid, const std::string& fontPath = "")
        : grid(simulationGrid), plot(fontPath)
    {
    }

    void resetZoom() noexcept { plot.resetZoom(); }

    void clear()
    {
        samples.clear();
        lastTime = -1.f;
        sampleStride = 1;
        sampleCounter = 0;
        resetZoom();
    }

    void handleEvent(const sf::Event& event, const sf::RenderWindow& window)
    {
        plot.handleEvent(event, window.getSize(), dataRange(), legend());
    }

    void update(float time)
    {
        if (!std::isfinite(time) || time < 0.f || time <= lastTime) return;

        ++sampleCounter;
        if (sampleCounter % sampleStride != 0) return;

        lastTime = time;
        samples.push_back({static_cast<float>(grid.herbivorePopulation()),
                           static_cast<float>(grid.carnivorePopulation())});
        if (samples.size() > MAX_SAMPLES) compactSamples();
    }

    void draw(sf::RenderTarget& target) const
    {
        plot.draw(target, dataRange(), legend(), "Herbivores", "Carnivores",
            [&](const sf::FloatRect& bounds, const Plot::Range& range) {
                for (std::size_t i = 1; i < samples.size(); ++i)
                    Plot::segment(target, bounds, range, samples[i - 1], samples[i], TRAJECTORY_COLOR);
            });
    }

private:
    static constexpr std::size_t MAX_SAMPLES = 2000;
    inline static const sf::Color TRAJECTORY_COLOR = sf::Color::Blue;

    Grid& grid;
    Plot plot;
    std::vector<Plot::Point> samples;
    float lastTime{-1.f};
    std::size_t sampleStride{1};
    std::size_t sampleCounter{0};

    static std::vector<Plot::LegendEntry> legend()
    {
        return {{"Trajectoire", TRAJECTORY_COLOR}};
    }

    Plot::Range dataRange() const
    {
        double maxHerbivores = 0.0;
        double maxCarnivores = 0.0;
        for (const Plot::Point& sample : samples)
        {
            if (std::isfinite(sample.x)) maxHerbivores = std::max(maxHerbivores, sample.x);
            if (std::isfinite(sample.y)) maxCarnivores = std::max(maxCarnivores, sample.y);
        }
        return {0.0, Plot::axisMaximum(maxHerbivores, 5.0),
                0.0, Plot::axisMaximum(maxCarnivores, 5.0)};
    }

    // Le sous-echantillonnage conserve le debut et le dernier etat de la simulation.
    void compactSamples()
    {
        const std::size_t count = samples.size();
        std::size_t kept = 0;
        for (std::size_t i = 0; i < count; i += 2) samples[kept++] = samples[i];
        if (count % 2 == 0) samples[kept++] = samples.back();
        samples.resize(kept);
        if (sampleStride <= std::numeric_limits<std::size_t>::max() / 2) sampleStride *= 2;
    }
};

#endif // DIAGRAM_H
