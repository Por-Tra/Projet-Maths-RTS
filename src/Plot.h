#ifndef PLOT_H
#define PLOT_H

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
#include <locale>
#include <sstream>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

#include <SFML/Graphics.hpp>

// Fonctions communes aux graphiques : police, axes, echelle, legende et zoom.
class Plot
{
public:
    struct Point { double x; double y; };
    struct Range
    {
        double x0{0.0};
        double x1{1.0};
        double y0{0.0};
        double y1{1.0};
    };
    struct LegendEntry { std::string label; sf::Color color; };

    explicit Plot(const std::string& fontPath = "")
    {
        std::error_code error;
        for (const auto& path : fontCandidates(fontPath))
        {
            if (std::filesystem::is_regular_file(path, error) && font.openFromFile(path))
            {
                hasFont = true;
                break;
            }
        }
        if (!hasFont)
            std::cerr << "[Plot] Police introuvable : asset/" << FONT_FILE
                      << ". Indiquez son chemin via GRAPH_FONT.\n";
    }

    void resetZoom() noexcept { automatic = true; }

    void handleEvent(const sf::Event& event, sf::Vector2u size, const Range& fullRange,
                     const std::vector<LegendEntry>& legend)
    {
        if (const auto* key = event.getIf<sf::Event::KeyPressed>())
        {
            if (key->code == sf::Keyboard::Key::R || key->code == sf::Keyboard::Key::Home)
                resetZoom();
        }

        const auto* wheel = event.getIf<sf::Event::MouseWheelScrolled>();
        if (!wheel || wheel->wheel != sf::Mouse::Wheel::Vertical ||
            !std::isfinite(wheel->delta) || wheel->delta == 0.f) return;

        const Range current = automatic ? fullRange : zoomRange;
        const sf::FloatRect bounds = layout(size, current, legend.size());
        const sf::Vector2f mouse{static_cast<float>(wheel->position.x),
                                 static_cast<float>(wheel->position.y)};
        if (!usable(bounds) || !bounds.contains(mouse)) return;

        const double anchorX = (mouse.x - bounds.position.x) / bounds.size.x;
        const double anchorY = 1.0 - (mouse.y - bounds.position.y) / bounds.size.y;
        const double factor = std::pow(1.2, -std::clamp(static_cast<double>(wheel->delta), -20.0, 20.0));
        const auto x = zoomAxis(current.x0, current.x1, fullRange.x1, anchorX, factor);
        const auto y = zoomAxis(current.y0, current.y1, fullRange.y1, anchorY, factor);
        zoomRange = {x.first, x.second, y.first, y.second};
        automatic = x.second - x.first >= fullRange.x1 * (1.0 - 1e-10) &&
                    y.second - y.first >= fullRange.y1 * (1.0 - 1e-10);
    }

    // Le rappel dessine uniquement les courbes, dans une vue limitee aux axes.
    template <typename DrawCurves>
    void draw(sf::RenderTarget& target, const Range& fullRange,
              const std::vector<LegendEntry>& legend, const std::string& xLabel,
              const std::string& yLabel, DrawCurves drawCurves) const
    {
        const sf::Vector2u size = target.getSize();
        if (size.x == 0 || size.y == 0) return;

        target.clear(sf::Color::White);
        const Range range = automatic ? fullRange : zoomRange;
        const sf::FloatRect bounds = layout(size, range, legend.size());
        if (!usable(bounds)) return;

        const sf::View saved = target.getView();
        const sf::Vector2f screenSize{static_cast<float>(size.x), static_cast<float>(size.y)};
        const sf::View screen(sf::FloatRect({0.f, 0.f}, screenSize));
        target.setView(screen);
        drawLegend(target, bounds, legend);

        sf::View curveView(bounds);
        curveView.setViewport(sf::FloatRect(
            {bounds.position.x / screenSize.x, bounds.position.y / screenSize.y},
            {bounds.size.x / screenSize.x, bounds.size.y / screenSize.y}));
        target.setView(curveView);
        drawCurves(bounds, range);

        target.setView(screen);
        drawAxes(target, bounds, range, xLabel, yLabel);
        target.setView(saved);
    }

    static double axisMaximum(double peak, double minimum = 1.0)
    {
        const double padded = std::max(peak * 1.1, minimum);
        const double step = niceStep(padded, 5);
        return std::ceil(padded / step) * step;
    }

    static void segment(sf::RenderTarget& target, const sf::FloatRect& bounds,
                        const Range& range, Point start, Point end, sf::Color color)
    {
        if (!std::isfinite(start.x) || !std::isfinite(start.y) ||
            !std::isfinite(end.x) || !std::isfinite(end.y)) return;

        start = {(start.x - range.x0) / (range.x1 - range.x0),
                 (start.y - range.y0) / (range.y1 - range.y0)};
        end = {(end.x - range.x0) / (range.x1 - range.x0),
               (end.y - range.y0) / (range.y1 - range.y0)};
        if (clip(start, end)) line(target, toScreen(bounds, start), toScreen(bounds, end), color);
    }

private:
    static constexpr const char* FONT_FILE = "Lato-Bold.ttf";
    static constexpr unsigned TICK_SIZE = 11;
    static constexpr float LEGEND_SPACING = 20.f;

    sf::Font font;
    bool hasFont{false};
    bool automatic{true};
    Range zoomRange;

    // Police du projet, cherchee depuis l'executable et le repertoire courant.
    static std::filesystem::path executableDir()
    {
        namespace fs = std::filesystem;
#if defined(_WIN32)
        std::wstring buffer(MAX_PATH, L'\0');
        for (;;)
        {
            const DWORD length = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
            if (length == 0) return {};
            if (length < buffer.size())
            {
                buffer.resize(length);
                return fs::path(buffer).parent_path();
            }
            buffer.resize(buffer.size() * 2);
        }
#elif defined(__APPLE__)
        std::uint32_t size = 0;
        _NSGetExecutablePath(nullptr, &size);
        std::string buffer(size, '\0');
        if (_NSGetExecutablePath(buffer.data(), &size) != 0) return {};
        std::error_code error;
        return fs::weakly_canonical(fs::path(buffer.c_str()), error).parent_path();
#else
        std::error_code error;
        return fs::read_symlink("/proc/self/exe", error).parent_path();
#endif
    }

    static std::vector<std::filesystem::path> fontCandidates(const std::string& explicitPath)
    {
        namespace fs = std::filesystem;
        std::vector<fs::path> paths;
        if (!explicitPath.empty()) paths.emplace_back(explicitPath);
        if (const char* path = std::getenv("GRAPH_FONT")) paths.emplace_back(path);
#ifdef ASSET_DIR
        paths.push_back(fs::path(ASSET_DIR) / FONT_FILE);
#endif
        std::error_code error;
        for (const fs::path& root : {executableDir(), fs::current_path(error)})
        {
            fs::path directory = root;
            for (int depth = 0; depth < 5 && !directory.empty(); ++depth)
            {
                paths.push_back(directory / "asset" / FONT_FILE);
                const fs::path parent = directory.parent_path();
                if (parent == directory) break;
                directory = parent;
            }
        }
        return paths;
    }

    // Echelle et cadrage.
    static double niceStep(double span, int target)
    {
        const double raw = span / std::max(1, target);
        if (!(raw > 0.0) || !std::isfinite(raw)) return 1.0;
        const double power = std::pow(10.0, std::floor(std::log10(raw)));
        const double fraction = raw / power;
        for (double multiplier : {1.0, 2.0, 2.5, 5.0, 10.0})
            if (fraction <= multiplier) return multiplier * power;
        return 10.0 * power;
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

        const std::string result = out.str();
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
        std::vector<double> values;
        const double first = std::ceil(low / step - 1e-9) * step;
        // Boucle bornee pour eviter les erreurs d'arrondi a fort zoom.
        for (int i = 0; i < 64; ++i)
        {
            const double value = first + i * step;
            if (value > high + step * 1e-8) break;
            if (value >= low) values.push_back(value);
        }
        return values;
    }

    static std::pair<double, double> zoomAxis(double low, double high, double maximum,
                                             double anchor, double factor)
    {
        const double span = std::clamp((high - low) * factor, maximum / 100000.0, maximum);
        const double start = std::clamp(low + anchor * (high - low) - anchor * span, 0.0, maximum - span);
        return {start, start + span};
    }

    sf::FloatRect layout(sf::Vector2u size, const Range& range, std::size_t legendCount) const
    {
        const float top = 24.f + LEGEND_SPACING * static_cast<float>(legendCount);
        const float height = static_cast<float>(size.y) - top - 56.f;
        const double step = niceStep(range.y1 - range.y0, std::max(2, static_cast<int>(height / 65.f)));
        float labelWidth = 0.f;
        for (double value : ticks(range.y0, range.y1, step))
            labelWidth = std::max(labelWidth, textWidth(formatValue(value, step)));
        const float left = std::ceil(std::max(64.f, labelWidth + 40.f));
        const double xStep = niceStep(range.x1 - range.x0, 5);
        const float right = std::max(24.f, textWidth(formatValue(range.x1, xStep)) * .5f + 8.f);
        return {{left, top}, {static_cast<float>(size.x) - left - right, height}};
    }

    static bool usable(const sf::FloatRect& bounds)
    {
        return bounds.size.x >= 100.f && bounds.size.y >= 60.f;
    }

    // Coupe les segments aux limites du graphique (algorithme de Liang-Barsky).
    static bool clip(Point& start, Point& end)
    {
        const double dx = end.x - start.x;
        const double dy = end.y - start.y;
        double enter = 0.0;
        double leave = 1.0;
        const double p[] = {-dx, dx, -dy, dy};
        const double q[] = {start.x, 1.0 - start.x, start.y, 1.0 - start.y};
        for (int i = 0; i < 4; ++i)
        {
            if (p[i] == 0.0)
            {
                if (q[i] < 0.0) return false;
                continue;
            }
            const double ratio = q[i] / p[i];
            if (p[i] < 0.0) enter = std::max(enter, ratio);
            else leave = std::min(leave, ratio);
            if (enter > leave) return false;
        }
        end = {start.x + leave * dx, start.y + leave * dy};
        start = {start.x + enter * dx, start.y + enter * dy};
        return true;
    }

    static sf::Vector2f toScreen(const sf::FloatRect& bounds, Point point)
    {
        return {bounds.position.x + static_cast<float>(point.x) * bounds.size.x,
                bounds.position.y + (1.f - static_cast<float>(point.y)) * bounds.size.y};
    }

    // Dessin sans habillage : lignes simples et texte noir.
    float textWidth(const std::string& value) const
    {
        return hasFont ? sf::Text(font, value, TICK_SIZE).getLocalBounds().size.x : 0.f;
    }

    void text(sf::RenderTarget& target, const std::string& value, sf::Vector2f position,
              float alignX = 0.f, float alignY = 0.f, unsigned size = TICK_SIZE,
              float rotation = 0.f) const
    {
        if (!hasFont) return;
        sf::Text label(font, value, size);
        const auto bounds = label.getLocalBounds();
        label.setOrigin({bounds.position.x + bounds.size.x * alignX,
                         bounds.position.y + bounds.size.y * alignY});
        label.setPosition({std::round(position.x), std::round(position.y)});
        label.setRotation(sf::degrees(rotation));
        label.setFillColor(sf::Color::Black);
        target.draw(label);
    }

    static void line(sf::RenderTarget& target, sf::Vector2f start, sf::Vector2f end,
                     sf::Color color = sf::Color::Black)
    {
        const sf::Vertex vertices[] = {{start, color}, {end, color}};
        target.draw(vertices, 2, sf::PrimitiveType::Lines);
    }

    void drawLegend(sf::RenderTarget& target, const sf::FloatRect& bounds,
                    const std::vector<LegendEntry>& legend) const
    {
        float y = 20.f;
        for (const LegendEntry& entry : legend)
        {
            line(target, {bounds.position.x, y}, {bounds.position.x + 16.f, y}, entry.color);
            text(target, entry.label, {bounds.position.x + 24.f, y}, 0.f, .5f);
            y += LEGEND_SPACING;
        }
    }

    double horizontalStep(const sf::FloatRect& bounds, const Range& range) const
    {
        int target = std::max(2, static_cast<int>(bounds.size.x / 100.f));
        for (; target > 1; --target)
        {
            const double step = niceStep(range.x1 - range.x0, target);
            float widest = 0.f;
            for (double value : ticks(range.x0, range.x1, step))
                widest = std::max(widest, textWidth(formatValue(value, step)));
            if (bounds.size.x * step / (range.x1 - range.x0) >= widest + 20.f) break;
        }
        return niceStep(range.x1 - range.x0, target);
    }

    void drawAxes(sf::RenderTarget& target, const sf::FloatRect& bounds, const Range& range,
                  const std::string& xLabel, const std::string& yLabel) const
    {
        const float left = bounds.position.x;
        const float bottom = bounds.position.y + bounds.size.y;
        line(target, bounds.position, {left, bottom});
        line(target, {left, bottom}, {left + bounds.size.x, bottom});

        const double yStep = niceStep(range.y1 - range.y0, std::max(2, static_cast<int>(bounds.size.y / 65.f)));
        for (double value : ticks(range.y0, range.y1, yStep))
        {
            const float y = toScreen(bounds, {0.0, (value - range.y0) / (range.y1 - range.y0)}).y;
            line(target, {left - 4.f, y}, {left, y});
            text(target, formatValue(value, yStep), {left - 8.f, y}, 1.f, .5f);
        }

        const double xStep = horizontalStep(bounds, range);
        for (double value : ticks(range.x0, range.x1, xStep))
        {
            const float x = toScreen(bounds, {(value - range.x0) / (range.x1 - range.x0), 0.0}).x;
            line(target, {x, bottom}, {x, bottom + 4.f});
            text(target, formatValue(value, xStep), {x, bottom + 10.f}, .5f);
        }
        text(target, xLabel, {left + bounds.size.x * .5f, bottom + 30.f}, .5f, 0.f, 12);
        text(target, yLabel, {18.f, bounds.position.y + bounds.size.y * .5f}, .5f, .5f, 12, -90.f);
    }
};

#endif // PLOT_H
