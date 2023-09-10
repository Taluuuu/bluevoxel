cmake_minimum_required(VERSION 3.20)
project(enet)
add_library(enet src/enet.cpp)
target_include_directories(enet PUBLIC include/)