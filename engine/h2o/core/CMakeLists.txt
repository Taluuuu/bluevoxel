cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_library(h2o_core STATIC
    src/app_config.cpp
    src/debug_infos.cpp
    src/engine.cpp
    src/layers.cpp
    src/profiling/scope_timer.cpp
    src/resources.cpp
    src/text_file.cpp
    src/thread_pool.cpp
    src/tickable.cpp
    src/utils.cpp
)

target_link_libraries(h2o_core
    PUBLIC
        glm
        fmt
        magic_enum
        oup

    PRIVATE
        yaml-cpp
)

target_include_directories(h2o_core
    PUBLIC
        include/
        
        glm
        fmt
        magic_enum
        oup

    PRIVATE
        yaml-cpp
)