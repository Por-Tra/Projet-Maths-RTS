#ifndef GRAPH_H
#define GRAPH_H

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <locale>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
#include <SFML/Graphics.hpp>
#include "Grid.h"
#include "MathsFormulas.h"

enum class GraphType { ExponentialPopulationWithMortality, LotkaVolterra };

class Graph
{
public:
    enum class Color { Background, Axes, Grid, Formula, CurrentPopulation, Text };

private:
    Grid& grid;
    GraphType type;
    float initialPopulation{0.f};
    float growthRate{0.10f};
    float mortalityRate{0.03f};
    std::vector<float> times, currentPopulation, formulaPopulation;
    sf::Font font;
    bool hasFont{false};
    static constexpr std::size_t MAX_SAMPLES = 2000;
    std::size_t sampleStride{1}, sampleCounter{0};

    struct Range { double x0{0.0}, x1{1.0}, y0{0.0}, y1{1.0}; };
    struct Layout
    {
        sf::FloatRect plot;
        bool compact{false};
        bool usable{false};
    };
    bool automatic{true};
    Range zoomRange;

    sf::Color color(Color value) const noexcept
    {
        switch (value)
        {
            case Color::Background: return sf::Color(248, 250, 252);
            case Color::Axes: return sf::Color(125, 137, 151);
            case Color::Grid: return sf::Color(229, 234, 240);
            case Color::Formula: return sf::Color(219, 119, 36);
            case Color::CurrentPopulation: return sf::Color(38, 108, 184);
            case Color::Text: return sf::Color(39, 51, 67);
        }
        return sf::Color::Black;
    }

    float textWidth(const std::string& value, unsigned size) const
    {
        if (!hasFont) return 0.f;
        return sf::Text(font, value, size).getLocalBounds().size.x;
    }

    // Positions refer to the visible glyph bounds, not the font's baseline.
    void text(sf::RenderWindow& window, const std::string& value, unsigned size,
              sf::Vector2f position, float alignX = 0.f, float alignY = 0.f,
              sf::Color ink = sf::Color(39, 51, 67)) const
    {
        if (!hasFont) return;
        sf::Text label(font, value, size);
        const auto bounds = label.getLocalBounds();
        label.setOrigin({bounds.position.x + bounds.size.x * alignX,
                         bounds.position.y + bounds.size.y * alignY});
        label.setPosition({std::round(position.x), std::round(position.y)});
        label.setFillColor(ink);
        window.draw(label);
    }

    static void rectangle(sf::RenderWindow& window, sf::FloatRect rect, sf::Color fill)
    {
        sf::RectangleShape shape(rect.size);
        shape.setPosition(rect.position);
        shape.setFillColor(fill);
        window.draw(shape);
    }

    static void line(sf::RenderWindow& window, sf::Vector2f a, sf::Vector2f b,
                     sf::Color ink, float thickness = 1.f)
    {
        const sf::Vector2f d = b - a;
        const float length = std::hypot(d.x, d.y);
        if (length == 0.f) return;
        const sf::Vector2f n{-d.y * thickness / (2.f * length),
                            d.x * thickness / (2.f * length)};
        const sf::Vertex vertices[] = {{a+n, ink}, {a-n, ink}, {b+n, ink},
                                       {b+n, ink}, {a-n, ink}, {b-n, ink}};
        window.draw(vertices, 6, sf::PrimitiveType::Triangles);
    }

    static double niceStep(double span, int target)
    {
        const double raw = span / std::max(1, target);
        if (!(raw > 0.0) || !std::isfinite(raw)) return 1.0;
        const double power = std::pow(10.0, std::floor(std::log10(raw)));
        const double fraction = raw / power;
        return (fraction <= 1.0 ? 1.0 : fraction <= 2.0 ? 2.0 :
                fraction <= 2.5 ? 2.5 : fraction <= 5.0 ? 5.0 : 10.0) * power;
    }

    static std::string formatValue(double value, double step)
    {
        if (std::abs(value) < step * 1e-8) value = 0.0;
        std::ostringstream out;
        out.imbue(std::locale::classic());
        const double magnitude = std::abs(value);
        if (magnitude >= 1e6 || (magnitude > 0.0 && magnitude < 1e-3))
        {
            const int precision = magnitude > 0.0
                ? std::clamp(static_cast<int>(std::ceil(std::log10(magnitude / step))) + 1, 1, 9) : 1;
            out << std::scientific << std::setprecision(precision) << value;
        }
        else
        {
            const int precision = std::clamp(static_cast<int>(std::ceil(-std::log10(step))) + 1, 0, 10);
            out << std::fixed << std::setprecision(precision) << value;
        }
        std::string result = out.str();
        const auto exponent = result.find('e');
        std::string mantissa = result.substr(0, exponent);
        if (mantissa.find('.') != std::string::npos)
        {
            while (!mantissa.empty() && mantissa.back() == '0') mantissa.pop_back();
            if (!mantissa.empty() && mantissa.back() == '.') mantissa.pop_back();
        }
        return mantissa + (exponent == std::string::npos ? "" : result.substr(exponent));
    }

    static std::vector<double> ticks(double low, double high, double step)
    {
        std::vector<double> result;
        const double first = std::ceil(low / step - 1e-9) * step;
        // Integer iteration avoids stalled floating-point loops at extreme zoom.
        for (int i = 0; i < 64; ++i)
        {
            const double value = first + i * step;
            if (value > high + step * 1e-8) break;
            if (value >= low) result.push_back(value);
        }
        return result;
    }

    static std::pair<double,double> zoomAxis(double lo, double hi, double maximum,
                                              double anchor, double factor)
    {
        const double span = std::clamp((hi-lo)*factor, maximum/100000.0, maximum);
        const double start = std::clamp(lo+anchor*(hi-lo)-anchor*span, 0.0, maximum-span);
        return {start,start+span};
    }

    Range dataRange() const
    {
        Range range;
        if (!times.empty()) range.x1 = std::max(1.0, static_cast<double>(times.back()));
        double peak = 1.0;
        for (const auto* series : {&currentPopulation, &formulaPopulation})
            for (float value : *series)
                if (std::isfinite(value)) peak = std::max(peak, static_cast<double>(value));
        const double step = niceStep(peak * 1.1, 5);
        range.y1 = std::ceil(peak * 1.1 / step) * step;
        return range;
    }

    Layout layout(sf::Vector2u size, const Range& range) const
    {
        Layout result;
        result.compact = size.x < 650;
        if (size.x < 360 || size.y < 280) return result;
        const float top = result.compact ? 126.f : 112.f;
        const float height = static_cast<float>(size.y) - top - 82.f;
        const double step = niceStep(range.y1 - range.y0, std::max(2, static_cast<int>(height / 65.f)));
        float labelWidth = 0.f;
        for (double value : ticks(range.y0, range.y1, step))
            labelWidth = std::max(labelWidth, textWidth(formatValue(value, step), 12));
        const float left = std::ceil(std::max(76.f, labelWidth + 43.f));
        result.plot = sf::FloatRect({left, top}, {static_cast<float>(size.x) - left - 34.f, height});
        result.usable = result.plot.size.x >= 100.f && height >= 60.f;
        return result;
    }

    // Clip each segment in data space. Clamping individual samples would draw
    // false horizontal lines along the borders when a curve leaves the zoom.
    static bool clip(double& x0, double& y0, double& x1, double& y1)
    {
        const double dx = x1 - x0, dy = y1 - y0;
        double enter = 0.0, leave = 1.0;
        const double p[] = {-dx, dx, -dy, dy};
        const double q[] = {x0, 1.0-x0, y0, 1.0-y0};
        for (int i = 0; i < 4; ++i)
        {
            if (p[i] == 0.0) { if (q[i] < 0.0) return false; }
            else
            {
                const double t = q[i] / p[i];
                if (p[i] < 0.0) enter = std::max(enter, t);
                else leave = std::min(leave, t);
                if (enter > leave) return false;
            }
        }
        x1 = x0 + leave * dx; y1 = y0 + leave * dy;
        x0 += enter * dx; y0 += enter * dy;
        return true;
    }

    void drawCurve(sf::RenderWindow& window, const std::vector<float>& values,
                   const sf::FloatRect& plot, const Range& range, sf::Color ink) const
    {
        const auto count = std::min(times.size(), values.size());
        auto point = [&](double x, double y) {
            return sf::Vector2f{plot.position.x + static_cast<float>(x) * plot.size.x,
                               plot.position.y + (1.f - static_cast<float>(y)) * plot.size.y};
        };
        for (std::size_t i = 1; i < count; ++i)
        {
            if (!std::isfinite(values[i-1]) || !std::isfinite(values[i])) continue;
            double x0 = (times[i-1] - range.x0) / (range.x1-range.x0);
            double x1 = (times[i] - range.x0) / (range.x1-range.x0);
            double y0 = (values[i-1] - range.y0) / (range.y1-range.y0);
            double y1 = (values[i] - range.y0) / (range.y1-range.y0);
            if (clip(x0, y0, x1, y1)) line(window, point(x0,y0), point(x1,y1), ink, 2.f);
        }
        if (count && std::isfinite(values.back()))
        {
            const double x = (times[count-1] - range.x0) / (range.x1-range.x0);
            const double y = (values[count-1] - range.y0) / (range.y1-range.y0);
            if (x >= 0.0 && x <= 1.0 && y >= 0.0 && y <= 1.0)
            {
                sf::CircleShape dot(3.f);
                dot.setOrigin({3.f,3.f}); dot.setPosition(point(x,y));
                dot.setFillColor(ink); window.draw(dot);
            }
        }
    }

    void drawGrid(sf::RenderWindow& window, const sf::FloatRect& plot, const Range& range) const
    {
        const float bottom = plot.position.y + plot.size.y;
        const double yStep = niceStep(range.y1-range.y0, std::max(2, static_cast<int>(plot.size.y / 65.f)));
        for (double value : ticks(range.y0, range.y1, yStep))
        {
            const float y = plot.position.y + plot.size.y * static_cast<float>((range.y1-value)/(range.y1-range.y0));
            line(window, {plot.position.x,y}, {plot.position.x+plot.size.x,y}, color(Color::Grid));
            line(window, {plot.position.x-5.f,y}, {plot.position.x,y}, color(Color::Axes));
            text(window, formatValue(value,yStep), 12, {plot.position.x-10.f,y}, 1.f, .5f);
        }
        int target = std::max(2, static_cast<int>(plot.size.x / 100.f));
        double xStep = niceStep(range.x1-range.x0, target);
        // Reduce tick density using measured text widths, including scientific notation.
        for (; target > 1; --target)
        {
            xStep = niceStep(range.x1-range.x0, target);
            float widest = 0.f;
            for (double value : ticks(range.x0, range.x1, xStep))
                widest = std::max(widest, textWidth(formatValue(value,xStep),12));
            if (plot.size.x * xStep / (range.x1-range.x0) >= widest + 20.f) break;
        }
        xStep = niceStep(range.x1-range.x0, target);
        for (double value : ticks(range.x0, range.x1, xStep))
        {
            const float x = plot.position.x + plot.size.x * static_cast<float>((value-range.x0)/(range.x1-range.x0));
            line(window, {x,plot.position.y}, {x,bottom}, color(Color::Grid));
            line(window, {x,bottom}, {x,bottom+5.f}, color(Color::Axes));
            const auto label = formatValue(value,xStep);
            const float half = textWidth(label,12) * .5f;
            const float labelX = std::clamp(x, plot.position.x, plot.position.x+plot.size.x+24.f-half);
            text(window, label, 12, {labelX,bottom+12.f}, .5f);
        }
    }

    void compactByHalf()
    {
        std::vector<float> newTimes, newCurrent, newFormula;
        newTimes.reserve(times.size() / 2 + 1);
        newCurrent.reserve(times.size() / 2 + 1);
        newFormula.reserve(times.size() / 2 + 1);

        for (std::size_t i = 0; i < times.size(); i += 2)
        {
            newTimes.push_back(times[i]);
            newCurrent.push_back(currentPopulation[i]);
            newFormula.push_back(formulaPopulation[i]);
        }
        // on s'assure de garder le tout dernier point même si l'indice sauté ne tombait pas dessus
        if (!times.empty() && newTimes.back() != times.back())
        {
            newTimes.push_back(times.back());
            newCurrent.push_back(currentPopulation.back());
            newFormula.push_back(formulaPopulation.back());
        }

        times = std::move(newTimes);
        currentPopulation = std::move(newCurrent);
        formulaPopulation = std::move(newFormula);
    }

public:
    // Optional explicit font path; existing two-argument construction is preserved.
    Graph(Grid& simulationGrid, GraphType graphType, const std::string& fontPath = "")
        : grid(simulationGrid), type(graphType)
    {
        std::vector<std::string> paths;
        if (!fontPath.empty()) paths.push_back(fontPath);
        if (const char* env = std::getenv("GRAPH_FONT")) paths.emplace_back(env);
        for (const std::string base : {"", "../", "../../"})
            for (const std::string name : {"asset/Lato-Regular.ttf", "assets/Lato-Regular.ttf",
                                           "asset/DejaVuSans.ttf", "asset/Lato-Bold.ttf"})
                paths.push_back(base + name);
        paths.insert(paths.end(), {
            "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
            "/usr/share/fonts/truetype/liberation2/LiberationSans-Regular.ttf",
            "/usr/share/fonts/TTF/DejaVuSans.ttf",
            "C:/Windows/Fonts/segoeui.ttf", "C:/Windows/Fonts/arial.ttf",
            "/System/Library/Fonts/Supplemental/Arial.ttf"});
        for (const auto& path : paths)
        {
            if (std::ifstream(path).good() && font.openFromFile(path))
            { hasFont = true; break; }
        }
        if (!hasFont)
            std::cerr << "[Graph] Police introuvable. Placez Lato-Regular.ttf dans asset/ "
                         "ou indiquez un fichier TTF via GRAPH_FONT ou le constructeur de Graph.\n";
    }

    void resetZoom() noexcept { automatic = true; }

    void handleEvent(const sf::Event& event, const sf::RenderWindow& window)
    {
        if (const auto* key = event.getIf<sf::Event::KeyPressed>())
            if (key->code == sf::Keyboard::Key::R || key->code == sf::Keyboard::Key::Home)
                resetZoom();
        const auto* wheel = event.getIf<sf::Event::MouseWheelScrolled>();
        if (!wheel || wheel->wheel != sf::Mouse::Wheel::Vertical ||
            !std::isfinite(wheel->delta) || wheel->delta == 0.f) return;
        const Range full = dataRange();
        const Range current = automatic ? full : zoomRange;
        const Layout ui = layout(window.getSize(), current);
        const sf::Vector2f mouse{static_cast<float>(wheel->position.x), static_cast<float>(wheel->position.y)};
        if (!ui.usable || !ui.plot.contains(mouse)) return;
        const double ax = (mouse.x-ui.plot.position.x) / ui.plot.size.x;
        const double ay = 1.0 - (mouse.y-ui.plot.position.y) / ui.plot.size.y;
        const double factor = std::pow(1.2, -std::clamp(static_cast<double>(wheel->delta), -20.0, 20.0));
        const auto x = zoomAxis(current.x0,current.x1,full.x1,ax,factor);
        const auto y = zoomAxis(current.y0,current.y1,full.y1,ay,factor);
        zoomRange = {x.first,x.second,y.first,y.second};
        automatic = x.second-x.first >= full.x1*(1.0-1e-10) &&
                    y.second-y.first >= full.y1*(1.0-1e-10);
    }

    void update(float time)
    {
        if (!std::isfinite(time) || time < 0.f) return;
        const float population = static_cast<float>(grid.herbivorePopulation());
        if (times.empty())
        {
            initialPopulation = population;
        }

        if (!times.empty() && time <= times.back()) return;

        // Sous-échantillonnage progressif : une fois le tampon compacté, on
        // n'enregistre qu'un pas de temps sur "sampleStride" pour continuer à
        // couvrir toute la durée écoulée sans jamais effacer le début.
        ++sampleCounter;
        if (sampleStride > 1 && (sampleCounter % sampleStride) != 0) return;

        times.push_back(time);
        currentPopulation.push_back(population);

        if (type == GraphType::ExponentialPopulationWithMortality)
        {
            formulaPopulation.push_back(MathsFormulas::exp_population_with_mortality(
                initialPopulation, growthRate, mortalityRate, time));
        }
        else
        {
            formulaPopulation.push_back(population);
        }

        if (times.size() > MAX_SAMPLES)
        {
            compactByHalf();
            if (sampleStride <= std::numeric_limits<std::size_t>::max() / 2) sampleStride *= 2;
        }
    }

    void draw(sf::RenderWindow& window)
    {
        const auto size = window.getSize();
        if (size.x == 0 || size.y == 0) return;
        // Screen-space view: one unit = one pixel, independent of gameView.
        const sf::View saved = window.getView();
        const sf::View screen(sf::FloatRect({0.f,0.f}, {static_cast<float>(size.x),static_cast<float>(size.y)}));
        window.setView(screen);
        rectangle(window, {{0.f,0.f},{static_cast<float>(size.x),static_cast<float>(size.y)}}, color(Color::Background));
        const Range range = automatic ? dataRange() : zoomRange;
        const Layout ui = layout(size,range);
        if (!ui.usable)
        {
            text(window, "Agrandissez la fenetre", 14, {size.x*.5f,size.y*.5f}, .5f,.5f);
            window.setView(saved);
            return;
        }
        const auto& plot = ui.plot;
        text(window, "Evolution de la population", ui.compact ? 19 : 23, {24.f,20.f});
        std::string subtitle = type == GraphType::ExponentialPopulationWithMortality
            ? "Modele exponentiel avec mortalite" : "Modele Lotka-Volterra";
        if (!ui.compact && type == GraphType::ExponentialPopulationWithMortality)
        {
            std::ostringstream detail;
            detail.imbue(std::locale::classic());
            detail << "P(t) = P0 exp((r - m)t)   |   P0 = " << initialPopulation
                   << "   r = " << growthRate << "   m = " << mortalityRate;
            if (textWidth(detail.str(),12) <= static_cast<float>(size.x)-48.f)
                subtitle = detail.str();
        }
        text(window, subtitle, 12, {24.f,51.f}, 0.f,0.f, sf::Color(98,111,128));
        if (!ui.compact)
            text(window, automatic ? "SUIVI AUTO" : "ZOOM MANUEL", 11,
                 {static_cast<float>(size.x)-24.f,28.f}, 1.f,0.f, color(Color::CurrentPopulation));

        // Legend outside the data area, sized from actual glyph bounds.
        float legendX = 24.f;
        float legendY = 83.f;
        const bool modelVisible = type == GraphType::ExponentialPopulationWithMortality;
        for (int entry = 0; entry < (modelVisible ? 2 : 1); ++entry)
        {
            const std::string label = entry == 0 ? "Population observee" : "Formule (modele)";
            const auto ink = color(entry == 0 ? Color::CurrentPopulation : Color::Formula);
            const float width = 34.f + textWidth(label,12) + 24.f;
            if (legendX + width > static_cast<float>(size.x)-16.f)
            { legendX=24.f; legendY+=22.f; }
            line(window, {legendX,legendY}, {legendX+24.f,legendY}, ink, 2.5f);
            text(window,label,12,{legendX+34.f,legendY},0.f,.5f);
            legendX += width;
        }
        rectangle(window,plot,sf::Color::White);
        drawGrid(window,plot,range);

        // Restrict thick lines and endpoint markers to the plot viewport.
        sf::View curveView(plot);
        curveView.setViewport(sf::FloatRect(
            {plot.position.x/size.x,plot.position.y/size.y},
            {plot.size.x/size.x,plot.size.y/size.y}));
        window.setView(curveView);
        if (modelVisible) drawCurve(window,formulaPopulation,plot,range,color(Color::Formula));
        drawCurve(window,currentPopulation,plot,range,color(Color::CurrentPopulation));
        window.setView(screen);
        const float bottom = plot.position.y+plot.size.y;
        line(window,plot.position,{plot.position.x,bottom},color(Color::Axes));
        line(window,{plot.position.x,bottom},{plot.position.x+plot.size.x,bottom},color(Color::Axes));
        text(window,"Temps (pas de simulation)",13,{plot.position.x+plot.size.x*.5f,bottom+37.f},.5f);
        if (hasFont)
        {
            sf::Text label(font,"Population",13);
            const auto bounds=label.getLocalBounds();
            label.setOrigin({bounds.position.x+bounds.size.x*.5f,bounds.position.y+bounds.size.y*.5f});
            label.setRotation(sf::degrees(-90.f));
            label.setPosition({20.f,plot.position.y+plot.size.y*.5f});
            label.setFillColor(color(Color::Text));
            window.draw(label);
        }
        const std::string help = ui.compact ? "Molette : zoom  |  R : auto  |  Tab : simulation"
            : "Molette : zoom au curseur   |   R / Home : cadrage auto   |   Espace : pause   |   Tab : simulation";
        text(window,help,ui.compact ? 10 : 11,{24.f,static_cast<float>(size.y)-15.f},0.f,.5f,sf::Color(98,111,128));
        if (times.empty())
            text(window,"En attente de donnees",14,{plot.position.x+plot.size.x*.5f,plot.position.y+plot.size.y*.5f},.5f,.5f);
        window.setView(saved);
    }

    GraphType getType() const noexcept { return type; }
};

#endif // GRAPH_H