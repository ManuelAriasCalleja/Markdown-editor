# Banco de pruebas de iconos: NO forma parte de la build del editor.
#   cmake -S . -B build && cmake --build build && ./build/iconpreview salida/
cmake_minimum_required(VERSION 3.16)
project(iconpreview LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(iconpreview preview.cpp iconvariants.cpp iconvariants.h)
target_link_libraries(iconpreview PRIVATE Qt6::Widgets)
