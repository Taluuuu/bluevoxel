cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_library(h2o_game_framework
    src/game_framework_module.cpp)

target_link_libraries(h2o_game_framework
    PRIVATE
        h2o_scene)

target_include_directories(h2o_game_framework
    PUBLIC
        include/

    PRIVATE
        h2o_scene)