cmake_minimum_required(VERSION 3.28)
project(RTS-PROJECT LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

set(SFML_BUILD_AUDIO OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_NETWORK OFF CACHE BOOL "" FORCE)

FetchContent_Declare(SFML
        GIT_REPOSITORY https://github.com/SFML/SFML.git
        GIT_TAG 3.1.0
        GIT_SHALLOW ON
        EXCLUDE_FROM_ALL
        SYSTEM)
FetchContent_MakeAvailable(SFML)

add_executable(main src/main.cpp)

target_compile_features(main PRIVATE cxx_std_17)

target_link_libraries(main PRIVATE
    sfml-graphics
    sfml-window
    sfml-system
)

# Seule police du projet : asset/Lato-Bold.ttf (aucune police systeme).
# 1) Chemin absolu du dossier asset/ integre au binaire (repli pour Graph.h)
target_compile_definitions(main PRIVATE ASSET_DIR="${CMAKE_CURRENT_SOURCE_DIR}/asset")

# 2) Copie asset/ a cote de l'executable apres chaque build (Linux, Windows, multi-config)
add_custom_command(TARGET main POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
            ${CMAKE_CURRENT_SOURCE_DIR}/asset
            $<TARGET_FILE_DIR:main>/asset
    COMMENT "Copie des assets vers $<TARGET_FILE_DIR:main>/asset")

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT main)