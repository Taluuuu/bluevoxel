cmake_minimum_required(VERSION 3.18)
project(core)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_library(core
    src/module.cpp
)

target_link_libraries(core
    PUBLIC
        glm
)

target_include_directories(core
    PUBLIC
        include/
        
        glm
)