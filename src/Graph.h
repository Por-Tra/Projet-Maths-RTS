#ifndef GRAPH_H
#define GRAPH_H

#include <algorithm>
#include <cmath>
#include <limits>
#include <string>
#include <vector>

#include "Grid.h"
#include "MathsFormulas.h"
#include "Plot.h"

enum class GraphType { ExponentialPopulationWithMortality, LotkaVolterra };

// Population observee et, selon le modele, population theorique au cours du temps.
class Graph
{
public:
    Graph(Grid& simulationGrid, GraphType graphType, const std::string& fontPath = "")
        : grid(simulationGrid), type(graphType), plot(fontPath)
    {
    }

    GraphType getType() const noexcept { return type; }
    void resetZoom() noexcept { plot.resetZoom(); }

    void handleEvent(const sf::Event& event, const sf::RenderWindow& window)
    {
        plot.handleEvent(event, window.getSize(), dataRange(), legend());
    }

    void update(float time)
    {
        if (!std::isfinite(time) || time < 0.f) return;
        if (!samples.empty() && time <= samples.back().time) return;

        const float population = static_cast<float>(grid.herbivorePopulation());
        if (samples.empty()) initialPopulation = population;

        ++sampleCounter;
        if (sampleCounter % sampleStride != 0) return;

        const float prediction = hasFormula()
            ? MathsFormulas::exp_population_with_mortality(
                initialPopulation, GROWTH_RATE, MORTALITY_RATE, time)
            : population;
        samples.push_back({time, population, prediction});
        if (samples.size() > MAX_SAMPLES) compactSamples();
    }

    void draw(sf::RenderTarget& target) const
    {
        plot.draw(target, dataRange(), legend(), "Temps (pas de simulation)", "Population",
            [&](const sf::FloatRect& bounds, const Plot::Range& range) {
                if (hasFormula()) drawCurve(target, bounds, range, true);
                drawCurve(target, bounds, range, false);
            });
    }

private:
    static constexpr std::size_t MAX_SAMPLES = 2000;
    static constexpr float GROWTH_RATE = 0.10f;
    static constexpr float MORTALITY_RATE = 0.03f;
    inline static const sf::Color OBSERVED_COLOR = sf::Color::Blue;
    inline static const sf::Color FORMULA_COLOR = sf::Color::Red;

    struct Sample
    {
        float time;
        float observed;
        float predicted;
    };

    Grid& grid;
    GraphType type;
    Plot plot;
    std::vector<Sample> samples;
    float initialPopulation{0.f};
    std::size_t sampleStride{1};
    std::size_t sampleCounter{0};

    bool hasFormula() const noexcept
    {
        return type == GraphType::ExponentialPopulationWithMortality;
    }

    std::vector<Plot::LegendEntry> legend() const
    {
        std::vector<Plot::LegendEntry> entries{{"Population observee", OBSERVED_COLOR}};
        if (hasFormula()) entries.push_back({"Formule (modele)", FORMULA_COLOR});
        return entries;
    }

    Plot::Range dataRange() const
    {
        Plot::Range range;
        if (!samples.empty()) range.x1 = std::max(1.0, static_cast<double>(samples.back().time));

        double peak = 1.0;
        for (const Sample& sample : samples)
        {
            if (std::isfinite(sample.observed)) peak = std::max(peak, static_cast<double>(sample.observed));
            if (hasFormula() && std::isfinite(sample.predicted))
                peak = std::max(peak, static_cast<double>(sample.predicted));
        }
        range.y1 = Plot::axisMaximum(peak);
        return range;
    }

    void drawCurve(sf::RenderTarget& target, const sf::FloatRect& bounds,
                   const Plot::Range& range, bool formula) const
    {
        const sf::Color color = formula ? FORMULA_COLOR : OBSERVED_COLOR;
        for (std::size_t i = 1; i < samples.size(); ++i)
        {
            const Sample& previous = samples[i - 1];
            const Sample& current = samples[i];
            Plot::segment(target, bounds, range,
                {previous.time, formula ? previous.predicted : previous.observed},
                {current.time, formula ? current.predicted : current.observed}, color);
        }
    }

    // Garde un point sur deux, ainsi que le dernier, sans perdre le debut de la courbe.
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

#endif // GRAPH_H
