cmake_minimum_required(VERSION 3.16)

project(TP_RTS VERSION 0.1 LANGUAGES CXX)
include_directories(".")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(PROJECT_SOURCES
    main.cpp
    Map.h
    Map.cpp
)

qt_add_executable(TP_RTS ${PROJECT_SOURCES} )

target_link_libraries(TP_RTS PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)
