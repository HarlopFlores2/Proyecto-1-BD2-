cmake_minimum_required(VERSION 3.22)
project(Proyecto_1_BD2_)

set(CMAKE_CXX_STANDARD 14)

add_executable(Proyecto_1_BD2_
        main.cpp
        extendibleHash.h
        sequentialFile.h )

set(SFML_STATIC_LIBRARIES TRUE)
set(SFML_DIR C:/SFML/lib/cmake/SFML)
find_package(SFML COMPONENTS system window graphics audio network REQUIRED)

include_directories(C:/SFML/include)
target_link_libraries(Proyecto_1_BD2_ sfml-system sfml-window sfml-graphics sfml-audio)