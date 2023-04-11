cmake_minimum_required(VERSION 3.20)
project(stb)
add_library(stb src/stb.cpp)
target_include_directories(stb PUBLIC include/)