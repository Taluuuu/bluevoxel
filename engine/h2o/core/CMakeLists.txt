cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_library(h2o_core
    src/engine.cpp
    src/utils.cpp
)

target_link_libraries(h2o_core
    PUBLIC
        glm
        fmt
        magic_enum
        oup
)

target_include_directories(h2o_core
    PUBLIC
        include/
        
        glm
        fmt
        magic_enum
        oup
)