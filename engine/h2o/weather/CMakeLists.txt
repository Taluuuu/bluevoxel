cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_library(h2o_weather STATIC
    src/weather.cpp
    src/weather_system.cpp
)

target_link_libraries(h2o_weather
    PRIVATE
        h2o_core
        h2o_networking
        h2o_scene
)

target_include_directories(h2o_weather
    PUBLIC
        include/

    PRIVATE
        h2o_core
        h2o_networking
        h2o_scene
)