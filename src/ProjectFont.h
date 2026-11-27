#ifndef PROJECT_FONT_H
#define PROJECT_FONT_H

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

#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <system_error>
#include <vector>

#include <SFML/Graphics/Font.hpp>

// Meme recherche de police pour les graphiques et le panneau de statistiques.
namespace ProjectFont
{
inline constexpr const char* FONT_FILE = "Lato-Bold.ttf";

// Police du projet, cherchee depuis l'executable et le repertoire courant.
inline std::filesystem::path executableDir()
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

inline std::vector<std::filesystem::path> fontCandidates(const std::string& explicitPath)
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

inline bool load(sf::Font& font, const std::string& fontPath = "")
{
    std::error_code error;
    for (const auto& path : fontCandidates(fontPath))
    {
        if (std::filesystem::is_regular_file(path, error) && font.openFromFile(path))
            return true;
    }
    std::cerr << "[Font] Police introuvable : asset/" << FONT_FILE
              << ". Indiquez son chemin via GRAPH_FONT.\n";
    return false;
}
}

#endif // PROJECT_FONT_H
