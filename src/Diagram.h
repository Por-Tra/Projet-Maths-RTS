#ifndef DIAGRAM_H
#define DIAGRAM_H

#if defined(_WIN32)
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <windows.h>
#elif defined(__APPLE__)
    #include <mach-o/dyld.h>
#endif

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <limits>
#include <locale>
#include <sstream>
#include <string>
#include <system_error>
#include <utility>
#include <vector>
#include <SFML/Graphics.hpp>
#include "Grid.h"

// Diagramme de phase : herbivores (axe X) contre carnivores (axe Y).
// La trajectoire va du pale (ancien) au fonce (recent) ; le point orange est l'etat actuel.
class Diagram
{
public:
    enum class Color { Background, Axes, Grid, Trajectory, Start, Current, Text, Muted };

private:
    // Seule police du projet : asset/Lato-Bold.ttf (aucune police systeme).
    static constexpr const char* FONT_FILE = "Lato-Bold.ttf";
    static constexpr unsigned TICK_SIZE = 11;
    static constexpr std::size_t MAX_SAMPLES = 2000;
    static constexpr double MIN_SPAN = 5.0;

    Grid& grid;
    std::vector<float> herbivores, carnivores;
    float lastTime{-1.f};
    std::size_t sampleStride{1}, sampleCounter{0};
    sf::Font font;
    bool hasFont{false};

    struct Range { double x0{0.0}, x1{1.0}, y0{0.0}, y1{1.0}; };
    struct Layout
    {
        sf::FloatRect plot;
        bool compact{false};
        bool usable{false};
    };
    bool automatic{true};
    Range zoomRange;

    // ------------------------------------------------------------ donnees
    
    float herbivoreCount() const { return static_cast<float>(grid.herbivorePopulation()); }
    float carnivoreCount() const { return static_cast<float>(grid.carnivorePopulation()); }

    // ------------------------------------------------------------- police
    static std::filesystem::path executableDir()
    {
        namespace fs = std::filesystem;
#if defined(_WIN32)
        std::wstring buffer(MAX_PATH, L'\0');
        for (;;)
        {
            const DWORD n = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
            if (n == 0) return {};
            if (n < buffer.size()) { buffer.resize(n); break; }
            buffer.resize(buffer.size() * 2);
        }
        return fs::path(buffer).parent_path();
#elif defined(__APPLE__)
        std::uint32_t size = 0;
        _NSGetExecutablePath(nullptr, &size);
        std::string buffer(size, '\0');
        if (_NSGetExecutablePath(buffer.data(), &size) != 0) return {};
        std::error_code ec;
        return fs::weakly_canonical(fs::path(buffer.c_str()), ec).parent_path();
#else
        std::error_code ec;
        return fs::read_symlink("/proc/self/exe", ec).parent_path();
#endif
    }

    static std::vector<std::filesystem::path> fontCandidates(const std::string& explicitPath)
    {
        namespace fs = std::filesystem;
        std::vector<fs::path> result;
        if (!explicitPath.empty()) result.emplace_back(explicitPath);
        if (const char* env = std::getenv("GRAPH_FONT")) result.emplace_back(env);
#ifdef ASSET_DIR
        result.push_back(fs::path(ASSET_DIR) / FONT_FILE);
#endif
        std::error_code ec;
        const fs::path roots[] = {executableDir(), fs::current_path(ec)};
        for (const fs::path& root : roots)
        {
            fs::path dir = root;
            for (int depth = 0; depth < 5 && !dir.empty(); ++depth)
            {
                result.push_back(dir / "asset" / FONT_FILE);
                const fs::path parent = dir.parent_path();
                if (parent == dir) break;
                dir = parent;
            }
        }
        return result;
    }

    // -------------------------------------------------------------- dessin
    sf::Color color(Color value) const noexcept
    {
        switch (value)
        {
            case Color::Background: return sf::Color(248, 250, 252);
            case Color::Axes:       return sf::Color(170, 180, 192);
            case Color::Grid:       return sf::Color(234, 238, 244);
            case Color::Trajectory: return sf::Color(38, 108, 184);
            case Color::Start:      return sf::Color(176, 196, 222);
            case Color::Current:    return sf::Color(219, 119, 36);
            case Color::Text:       return sf::Color(51, 65, 85);
            case Color::Muted:      return sf::Color(120, 132, 148);
        }
        return sf::Color::Black;
    }

    static sf::Color lerp(sf::Color a, sf::Color b, float t) noexcept
    {
        auto mix = [t](std::uint8_t x, std::uint8_t y) {
            return static_cast<std::uint8_t>(std::lround(x + (y - x) * t));
        };
        return sf::Color(mix(a.r, b.r), mix(a.g, b.g), mix(a.b, b.b));
    }

    float textWidth(const std::string& value, unsigned size) const
    {
        if (!hasFont) return 0.f;
        return sf::Text(font, value, size).getLocalBounds().size.x;
    }

    void text(sf::RenderWindow& window, const std::string& value, unsigned size,
              sf::Vector2f position, float alignX = 0.f, float alignY = 0.f,
              sf::Color ink = sf::Color(51, 65, 85)) const
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

    static void dot(sf::RenderWindow& window, sf::Vector2f center, float radius,
                    sf::Color fill, sf::Color outline, float outlineThickness)
    {
        sf::CircleShape shape(radius);
        shape.setOrigin({radius, radius});
        shape.setPosition(center);
        shape.setFillColor(fill);
        shape.setOutlineColor(outline);
        shape.setOutlineThickness(outlineThickness);
        window.draw(shape);
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

    static double niceCeiling(double peak)
    {
        peak = std::max(peak * 1.1, MIN_SPAN);
        const double step = niceStep(peak, 5);
        return std::ceil(peak / step) * step;
    }

    Range dataRange() const
    {
        double maxH = 0.0, maxC = 0.0;
        for (float v : herbivores) if (std::isfinite(v)) maxH = std::max(maxH, static_cast<double>(v));
        for (float v : carnivores) if (std::isfinite(v)) maxC = std::max(maxC, static_cast<double>(v));
        Range range;
        range.x1 = niceCeiling(maxH);
        range.y1 = niceCeiling(maxC);
        return range;
    }

    Layout layout(sf::Vector2u size, const Range& range) const
    {
        Layout result;
        result.compact = size.x < 650;
        if (size.x < 360 || size.y < 280) return result;
        const float top = result.compact ? 108.f : 94.f;
        const float height = static_cast<float>(size.y) - top - 70.f;
        const double step = niceStep(range.y1 - range.y0, std::max(2, static_cast<int>(height / 65.f)));
        float labelWidth = 0.f;
        for (double value : ticks(range.y0, range.y1, step))
            labelWidth = std::max(labelWidth, textWidth(formatValue(value, step), TICK_SIZE));
        const float left = std::ceil(std::max(70.f, labelWidth + 40.f));
        result.plot = sf::FloatRect({left, top}, {static_cast<float>(size.x) - left - 30.f, height});
        result.usable = result.plot.size.x >= 100.f && height >= 60.f;
        return result;
    }

    // Clip each segment in normalized space (Liang-Barsky) so that a trajectory
    // leaving the zoomed area is cut cleanly instead of being squashed on the border.
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

    sf::Vector2f toScreen(const sf::FloatRect& plot, const Range& range, double h, double c) const
    {
        const double x = (h - range.x0) / (range.x1 - range.x0);
        const double y = (c - range.y0) / (range.y1 - range.y0);
        return {plot.position.x + static_cast<float>(x) * plot.size.x,
                plot.position.y + (1.f - static_cast<float>(y)) * plot.size.y};
    }

    bool inside(const Range& range, double h, double c) const
    {
        return h >= range.x0 && h <= range.x1 && c >= range.y0 && c <= range.y1;
    }

    void drawTrajectory(sf::RenderWindow& window, const sf::FloatRect& plot, const Range& range) const
    {
        const std::size_t count = std::min(herbivores.size(), carnivores.size());
        if (count == 0) return;
        const sf::Color oldest = color(Color::Start), newest = color(Color::Trajectory);
        for (std::size_t i = 1; i < count; ++i)
        {
            if (!std::isfinite(herbivores[i-1]) || !std::isfinite(herbivores[i]) ||
                !std::isfinite(carnivores[i-1]) || !std::isfinite(carnivores[i])) continue;
            double x0 = (herbivores[i-1] - range.x0) / (range.x1 - range.x0);
            double x1 = (herbivores[i]   - range.x0) / (range.x1 - range.x0);
            double y0 = (carnivores[i-1] - range.y0) / (range.y1 - range.y0);
            double y1 = (carnivores[i]   - range.y0) / (range.y1 - range.y0);
            if (!clip(x0, y0, x1, y1)) continue;
            const float age = static_cast<float>(i) / static_cast<float>(count - 1);
            const auto point = [&](double x, double y) {
                return sf::Vector2f{plot.position.x + static_cast<float>(x) * plot.size.x,
                                   plot.position.y + (1.f - static_cast<float>(y)) * plot.size.y};
            };
            line(window, point(x0, y0), point(x1, y1), lerp(oldest, newest, age), 2.f);
        }
        if (std::isfinite(herbivores.front()) && std::isfinite(carnivores.front()) &&
            inside(range, herbivores.front(), carnivores.front()))
            dot(window, toScreen(plot, range, herbivores.front(), carnivores.front()),
                3.5f, sf::Color::White, color(Color::Muted), 1.5f);
        if (std::isfinite(herbivores[count-1]) && std::isfinite(carnivores[count-1]) &&
            inside(range, herbivores[count-1], carnivores[count-1]))
            dot(window, toScreen(plot, range, herbivores[count-1], carnivores[count-1]),
                4.5f, color(Color::Current), sf::Color::White, 1.5f);
    }

    void drawGrid(sf::RenderWindow& window, const sf::FloatRect& plot, const Range& range) const
    {
        const float bottom = plot.position.y + plot.size.y;
        const sf::Color muted = color(Color::Muted);
        const double yStep = niceStep(range.y1-range.y0, std::max(2, static_cast<int>(plot.size.y / 65.f)));
        for (double value : ticks(range.y0, range.y1, yStep))
        {
            const float y = plot.position.y + plot.size.y * static_cast<float>((range.y1-value)/(range.y1-range.y0));
            line(window, {plot.position.x,y}, {plot.position.x+plot.size.x,y}, color(Color::Grid));
            text(window, formatValue(value,yStep), TICK_SIZE, {plot.position.x-8.f,y}, 1.f, .5f, muted);
        }
        int target = std::max(2, static_cast<int>(plot.size.x / 100.f));
        double xStep = niceStep(range.x1-range.x0, target);
        for (; target > 1; --target)
        {
            xStep = niceStep(range.x1-range.x0, target);
            float widest = 0.f;
            for (double value : ticks(range.x0, range.x1, xStep))
                widest = std::max(widest, textWidth(formatValue(value,xStep),TICK_SIZE));
            if (plot.size.x * xStep / (range.x1-range.x0) >= widest + 20.f) break;
        }
        xStep = niceStep(range.x1-range.x0, target);
        for (double value : ticks(range.x0, range.x1, xStep))
        {
            const float x = plot.position.x + plot.size.x * static_cast<float>((value-range.x0)/(range.x1-range.x0));
            line(window, {x,plot.position.y}, {x,bottom}, color(Color::Grid));
            const auto label = formatValue(value,xStep);
            const float half = textWidth(label,TICK_SIZE) * .5f;
            const float maxX = std::max(plot.position.x, plot.position.x+plot.size.x+24.f-half);
            text(window, label, TICK_SIZE, {std::clamp(x, plot.position.x, maxX), bottom+10.f}, .5f, 0.f, muted);
        }
    }

    void compactByHalf()
    {
        std::vector<float> newHerbivores, newCarnivores;
        newHerbivores.reserve(herbivores.size() / 2 + 1);
        newCarnivores.reserve(herbivores.size() / 2 + 1);
        for (std::size_t i = 0; i < herbivores.size(); i += 2)
        {
            newHerbivores.push_back(herbivores[i]);
            newCarnivores.push_back(carnivores[i]);
        }
        // Le dernier point est toujours conserve : c'est l'etat actuel.
        if (!herbivores.empty() && (herbivores.size() - 1) % 2 != 0)
        {
            newHerbivores.push_back(herbivores.back());
            newCarnivores.push_back(carnivores.back());
        }
        herbivores = std::move(newHerbivores);
        carnivores = std::move(newCarnivores);
    }

public:
    // Chemin de police optionnel (sinon : GRAPH_FONT, ASSET_DIR, puis asset/Lato-Bold.ttf
    // recherche depuis l'executable et le repertoire courant).
    explicit Diagram(Grid& simulationGrid, const std::string& fontPath = "")
        : grid(simulationGrid)
    {
        std::error_code ec;
        for (const auto& path : fontCandidates(fontPath))
        {
            if (std::filesystem::is_regular_file(path, ec) && font.openFromFile(path))
            { hasFont = true; break; }
        }
        if (!hasFont)
            std::cerr << "[Diagram] Police introuvable : asset/" << FONT_FILE << ".\n"
                         "Lancez le programme depuis le projet, copiez asset/ a cote de "
                         "l'executable, ou indiquez le fichier via GRAPH_FONT.\n";
    }

    void resetZoom() noexcept { automatic = true; }

    // Efface la trajectoire (ex. : redemarrage de la simulation).
    void clear()
    {
        herbivores.clear();
        carnivores.clear();
        lastTime = -1.f;
        sampleStride = 1;
        sampleCounter = 0;
        automatic = true;
    }

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

    // A appeler a chaque pas de simulation avec le temps ecoule.
    void update(float time)
    {
        if (!std::isfinite(time) || time < 0.f) return;
        if (!herbivores.empty() && time <= lastTime) return;

        // Sous-echantillonnage progressif : une fois le tampon compacte, on ne
        // garde qu'un pas sur "sampleStride" pour couvrir toute la duree.
        ++sampleCounter;
        if (sampleStride > 1 && (sampleCounter % sampleStride) != 0) return;

        lastTime = time;
        herbivores.push_back(herbivoreCount());
        carnivores.push_back(carnivoreCount());

        if (herbivores.size() > MAX_SAMPLES)
        {
            compactByHalf();
            if (sampleStride <= std::numeric_limits<std::size_t>::max() / 2) sampleStride *= 2;
        }
    }

    void draw(sf::RenderWindow& window)
    {
        const auto size = window.getSize();
        if (size.x == 0 || size.y == 0) return;
        const sf::View saved = window.getView();
        const sf::View screen(sf::FloatRect({0.f,0.f}, {static_cast<float>(size.x),static_cast<float>(size.y)}));
        window.setView(screen);
        rectangle(window, {{0.f,0.f},{static_cast<float>(size.x),static_cast<float>(size.y)}}, color(Color::Background));
        const Range range = automatic ? dataRange() : zoomRange;
        const Layout ui = layout(size,range);
        if (!ui.usable)
        {
            text(window, "Agrandissez la fenetre", 14, {size.x*.5f,size.y*.5f}, .5f,.5f, color(Color::Muted));
            window.setView(saved);
            return;
        }
        const auto& plot = ui.plot;
        const sf::Color ink = color(Color::Text);
        const sf::Color muted = color(Color::Muted);

        // En-tete : titre, sous-titre discret, valeurs actuelles a droite.
        text(window, "Diagramme de phase", ui.compact ? 16 : 18, {24.f,18.f}, 0.f,0.f, ink);
        text(window, "Carnivores en fonction des herbivores", 11, {24.f,46.f}, 0.f,0.f, muted);
        if (!ui.compact && !herbivores.empty())
        {
            std::ostringstream readout;
            readout << "Herbivores : " << std::lround(herbivores.back())
                    << "   Carnivores : " << std::lround(carnivores.back());
            text(window, readout.str(), 12, {static_cast<float>(size.x)-24.f,19.f}, 1.f,0.f, ink);
            if (!automatic)
                text(window, "Zoom manuel  -  R pour reinitialiser", 11,
                     {static_cast<float>(size.x)-24.f,40.f}, 1.f,0.f, muted);
        }

        // Legende : degrade temporel + etat actuel.
        float legendX = 24.f;
        float legendY = 72.f;
        for (int entry = 0; entry < 2; ++entry)
        {
            const std::string label = entry == 0 ? "Debut -> maintenant" : "Etat actuel";
            const float swatch = entry == 0 ? 30.f : 12.f;
            const float width = swatch + 8.f + textWidth(label,11) + 22.f;
            if (legendX + width > static_cast<float>(size.x)-16.f)
            { legendX=24.f; legendY+=20.f; }
            if (entry == 0)
            {
                const int strips = 15;
                for (int s = 0; s < strips; ++s)
                    rectangle(window, {{legendX + s*2.f, legendY-2.f}, {2.f, 4.f}},
                              lerp(color(Color::Start), color(Color::Trajectory),
                                   static_cast<float>(s)/(strips-1)));
            }
            else
                dot(window, {legendX+6.f,legendY}, 4.f, color(Color::Current), sf::Color::White, 1.f);
            text(window,label,11,{legendX+swatch+8.f,legendY},0.f,.5f, ink);
            legendX += width;
        }

        rectangle(window,plot,sf::Color::White);
        drawGrid(window,plot,range);

        // La trajectoire est coupee a la zone de trace.
        sf::View curveView(plot);
        curveView.setViewport(sf::FloatRect(
            {plot.position.x/size.x,plot.position.y/size.y},
            {plot.size.x/size.x,plot.size.y/size.y}));
        window.setView(curveView);
        drawTrajectory(window,plot,range);
        window.setView(screen);

        const float bottom = plot.position.y+plot.size.y;
        line(window,plot.position,{plot.position.x,bottom},color(Color::Axes));
        line(window,{plot.position.x,bottom},{plot.position.x+plot.size.x,bottom},color(Color::Axes));
        text(window,"Herbivores",12,{plot.position.x+plot.size.x*.5f,bottom+30.f},.5f,0.f,muted);
        if (hasFont)
        {
            sf::Text label(font,"Carnivores",12);
            const auto bounds=label.getLocalBounds();
            label.setOrigin({bounds.position.x+bounds.size.x*.5f,bounds.position.y+bounds.size.y*.5f});
            label.setRotation(sf::degrees(-90.f));
            label.setPosition({18.f,std::round(plot.position.y+plot.size.y*.5f)});
            label.setFillColor(muted);
            window.draw(label);
        }
        const std::string help = ui.compact ? "Molette : zoom  |  R : auto  |  Tab : simulation"
            : "Molette : zoom  |  R : cadrage auto  |  Espace : pause  |  Tab : simulation";
        text(window,help,10,{24.f,static_cast<float>(size.y)-14.f},0.f,.5f,muted);
        if (herbivores.empty())
            text(window,"En attente de donnees",14,{plot.position.x+plot.size.x*.5f,plot.position.y+plot.size.y*.5f},.5f,.5f,muted);
        window.setView(saved);
    }
};

#endif // DIAGRAM_H