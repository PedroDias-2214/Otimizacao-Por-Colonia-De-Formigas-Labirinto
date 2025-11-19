cmake_minimum_required(VERSION 4.0)
project(ACO_Labirinto)

set(CMAKE_CXX_STANDARD 20)

add_executable(ACO_Labirinto main.cpp
        Labirinto.cpp
        Labirinto.h
        Formiga.cpp
        Formiga.h)
