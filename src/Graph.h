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

// Herbivores et carnivores simules, compares aux effectifs theoriques.
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
        if (!std::isfinite(time) || time < 0.f || time <= lastTime) return;

        const float herbivores = static_cast<float>(grid.herbivorePopulation());
        const float carnivores = static_cast<float>(grid.carnivorePopulation());
        if (lastTime < 0.f)
        {
            initialHerbivores = herbivores;
            initialTime = time;
            theoretical = {herbivores, carnivores};
            modelRates = lotkaVolterraRates(theoretical);
        }
        else if (type == GraphType::LotkaVolterra)
        {
            theoretical = MathsFormulas::lotka_volterra(theoretical, modelRates, static_cast<double>(time) - lastTime);
        }
        lastTime = time;

        // Le modele avance a chaque pas, meme si le graphique saute des echantillons.
        ++sampleCounter;
        if (sampleCounter % sampleStride != 0) return;

        samples.push_back({time, herbivores, carnivores, prediction(time - initialTime)});
        if (samples.size() > MAX_SAMPLES) compactSamples();
    }

    void draw(sf::RenderTarget& target) const
    {
        plot.draw(target, dataRange(), legend(), "Temps (pas de simulation)", "Population",
            [&](const sf::FloatRect& bounds, const Plot::Range& range) {
                // La simulation reste au premier plan si les courbes se croisent.
                drawCurve(target, bounds, range, Curve::PredictedHerbivores, PREDICTED_HERBIVORE_COLOR);
                if (type == GraphType::LotkaVolterra)
                    drawCurve(target, bounds, range, Curve::PredictedCarnivores, PREDICTED_CARNIVORE_COLOR);
                drawCurve(target, bounds, range, Curve::Herbivores, HERBIVORE_COLOR);
                drawCurve(target, bounds, range, Curve::Carnivores, CARNIVORE_COLOR);
            });
    }

private:
    static constexpr std::size_t MAX_SAMPLES = 2000;
    static constexpr float GROWTH_RATE = 0.10f;
    static constexpr float MORTALITY_RATE = 0.03f;
    inline static const sf::Color HERBIVORE_COLOR = sf::Color::Blue;
    inline static const sf::Color CARNIVORE_COLOR = sf::Color::Red;
    inline static const sf::Color PREDICTED_HERBIVORE_COLOR{105, 64, 145};
    inline static const sf::Color PREDICTED_CARNIVORE_COLOR{0, 119, 79};

    enum class Curve { Herbivores, Carnivores, PredictedHerbivores, PredictedCarnivores };

    struct Sample
    {
        float time;
        float herbivores;
        float carnivores;
        MathsFormulas::Populations predicted;
    };

    Grid& grid;
    GraphType type;
    Plot plot;
    std::vector<Sample> samples;
    float initialHerbivores{0.f};
    float initialTime{0.f};
    float lastTime{-1.f};
    MathsFormulas::Populations theoretical{};
    MathsFormulas::LotkaVolterraRates modelRates{};
    std::size_t sampleStride{1};
    std::size_t sampleCounter{0};

    static MathsFormulas::LotkaVolterraRates lotkaVolterraRates(MathsFormulas::Populations initial)
    {
        constexpr double herbivoreGrowth = 0.05;
        constexpr double carnivoreMortality = 0.02;
        // Coefficients indicatifs par pas, dimensionnes d'apres les effectifs initiaux.
        // Equilibre du modele : carnivores initiaux et 60 % des herbivores initiaux.
        const double equilibriumCarnivores = std::max(1.0, initial.carnivores);
        const double equilibriumHerbivores = std::max(1.0, 0.60 * initial.herbivores);
        return {herbivoreGrowth, herbivoreGrowth / equilibriumCarnivores,
                carnivoreMortality, carnivoreMortality / equilibriumHerbivores};
    }

    MathsFormulas::Populations prediction(float elapsed) const
    {
        if (type == GraphType::LotkaVolterra) return theoretical;
        return {MathsFormulas::exp_population_with_mortality(
                    initialHerbivores, GROWTH_RATE, MORTALITY_RATE, elapsed), 0.0};
    }

    std::vector<Plot::LegendEntry> legend() const
    {
        const bool lotkaVolterra = type == GraphType::LotkaVolterra;
        std::vector<Plot::LegendEntry> entries{
            {"Herbivores (simulation)", HERBIVORE_COLOR},
            {"Carnivores (simulation)", CARNIVORE_COLOR},
            {lotkaVolterra ? "Herbivores (Lotka-Volterra)" : "Herbivores (exponentiel)", PREDICTED_HERBIVORE_COLOR}
        };
        if (lotkaVolterra)
            entries.push_back({"Carnivores (Lotka-Volterra)", PREDICTED_CARNIVORE_COLOR});
        return entries;
    }

    Plot::Range dataRange() const
    {
        Plot::Range range;
        if (!samples.empty()) range.x1 = std::max(1.0, static_cast<double>(samples.back().time));

        double peak = 1.0;
        for (const Sample& sample : samples)
        {
            peak = std::max({peak, static_cast<double>(sample.herbivores),
                            static_cast<double>(sample.carnivores)});
        }
        // Le modele peut diverger : il ne doit jamais ecraser l'echelle de la simulation.
        // Plot coupe seulement son trace aux axes, sans modifier les valeurs calculees.
        range.y1 = Plot::axisMaximum(peak);
        return range;
    }

    static double population(const Sample& sample, Curve curve)
    {
        if (curve == Curve::Herbivores) return sample.herbivores;
        if (curve == Curve::Carnivores) return sample.carnivores;
        if (curve == Curve::PredictedHerbivores) return sample.predicted.herbivores;
        return sample.predicted.carnivores;
    }

    void drawCurve(sf::RenderTarget& target, const sf::FloatRect& bounds,
                   const Plot::Range& range, Curve curve, sf::Color color) const
    {
        for (std::size_t i = 1; i < samples.size(); ++i)
        {
            const Sample& previous = samples[i - 1];
            const Sample& current = samples[i];
            Plot::segment(target, bounds, range,
                {previous.time, population(previous, curve)},
                {current.time, population(current, curve)}, color);
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
