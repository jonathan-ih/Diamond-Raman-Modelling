cmake_minimum_required(VERSION 3.16)
project(Diamond_Raman_Modelling)

set(CMAKE_CXX_STANDARD 14)
SET(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -fexceptions")
find_library(GSL REQUIRED)

add_executable(Diamond_Raman_Modelling
        main.cpp diamond.cpp diamond.h laser.cpp laser.h raman.cpp raman.h
        fitting.cpp fitting.h settings.cpp settings.h)

target_link_libraries(Diamond_Raman_Modelling gsl)