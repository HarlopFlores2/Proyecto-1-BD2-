cmake_minimum_required(VERSION 3.22)
project(Proyecto_1_BD2_)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(Proyecto_1_BD2_
        main.cpp
        sequentialFile.h
        sequentialFile.cpp)
