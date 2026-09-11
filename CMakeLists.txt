cmake_minimum_required(VERSION 3.21)   # 4.0 est trop récent, ça cassera sur Linux si CMake < 4.0
project(RTS-PROJECT CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

# On ne construit que ce dont on a besoin (graphique), ça évite
# de tirer les dépendances système d'audio/réseau (openal, vorbis, flac)
set(SFML_BUILD_AUDIO OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_NETWORK OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 2.6.1
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(SFML)

add_executable(RTS-PROJECT src/main.cpp)

target_link_libraries(RTS-PROJECT PRIVATE
    sfml-graphics
    sfml-window
    sfml-system
)

if(WIN32)
    add_custom_command(TARGET RTS-PROJECT POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_RUNTIME_DLLS:RTS-PROJECT>
            $<TARGET_FILE_DIR:RTS-PROJECT>
        COMMAND_EXPAND_LISTS
    )
endif()