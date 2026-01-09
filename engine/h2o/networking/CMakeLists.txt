cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_library(h2o_networking STATIC
    src/client.cpp
    src/net_peer_local.cpp
    src/net_peer_online.cpp
    src/networking_module.cpp
    src/server.cpp
)

target_link_libraries(h2o_networking
    PUBLIC
        bitsery
        GameNetworkingSockets::shared

    PRIVATE
        h2o_core
)

target_include_directories(h2o_networking
    PUBLIC
        include/

        bitsery
        GameNetworkingSockets::shared

    PRIVATE
        h2o_core
)