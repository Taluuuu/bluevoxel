cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

set(INVENTORY_SOURCES
    src/inventory_module.cpp)

add_library(h2o_inventory STATIC ${INVENTORY_SOURCES})

target_link_libraries(h2o_inventory
    PRIVATE
        h2o_core
        h2o_scene
        h2o_ui)

target_include_directories(h2o_inventory
    PUBLIC
        include/

    PRIVATE
        h2o_core
        h2o_scene
        h2o_ui)
