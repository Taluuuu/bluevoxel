cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

set(HEALTH_SOURCES
    src/health_component.cpp
)

add_library(h2o_health STATIC ${HEALTH_SOURCES})

target_link_libraries(h2o_health
    PRIVATE
        h2o_core
        h2o_networking
        h2o_scene
        h2o_ui

        yaml-cpp
)

target_include_directories(h2o_health
    PUBLIC
        include/

    PRIVATE
        h2o_core
        h2o_networking
        h2o_scene
        h2o_ui

        yaml-cpp
)
