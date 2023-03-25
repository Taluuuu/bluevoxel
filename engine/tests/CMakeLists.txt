cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_executable(h2o_tests
    core/handle_types_tests.cpp)

target_link_libraries(h2o_tests
    PRIVATE
        h2o_core

        Catch2::Catch2WithMain)

target_include_directories(h2o_tests
    PRIVATE
        h2o_core)