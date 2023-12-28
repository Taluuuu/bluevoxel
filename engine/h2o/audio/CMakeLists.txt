cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

set(AUDIO_SOURCES
        src/audio_module.cpp)

add_library(h2o_audio ${AUDIO_SOURCES})

target_link_libraries(h2o_audio
    PRIVATE
        h2o_core)

target_include_directories(h2o_audio
    PUBLIC
        include/

    PRIVATE
        h2o_core)
