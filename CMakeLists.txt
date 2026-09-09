cmake_minimum_required(VERSION 3.16)

project(TP_RTS VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

qt_add_executable(TP_RTS
    main.cpp
    Map.h
)

target_link_libraries(TP_RTS PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)