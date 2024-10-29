cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

set(GRAPH_SOURCES
    src/graph.cpp
    src/graph_ui.cpp
)

add_library(h2o_graph STATIC ${GRAPH_SOURCES})

target_link_libraries(h2o_graph
    PRIVATE
        h2o_core
        h2o_ui

        yaml-cpp
)

target_include_directories(h2o_graph
    PUBLIC
        include/

    PRIVATE
        h2o_core
        h2o_ui

        yaml-cpp
)
