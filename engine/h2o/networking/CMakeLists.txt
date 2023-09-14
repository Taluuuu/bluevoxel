cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_library(h2o_networking
    src/client.cpp
    src/networking_module.cpp
    src/server.cpp
)

target_link_libraries(h2o_networking
    PUBLIC
        GameNetworkingSockets::shared

    PRIVATE
        h2o_core
)

target_include_directories(h2o_networking
    PUBLIC
        include/

        GameNetworkingSockets::shared

    PRIVATE
        h2o_core
)