cmake_minimum_required(VERSION 3.20)
project(game)

set(CMAKE_CXX_STANDARD 20)

add_executable(game 
    src/game_module.cpp
    src/main.cpp
)

set_target_properties(game PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_BINARY_DIR}/..)
set_target_properties(game PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_BINARY_DIR}/..)
set_target_properties(game PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_CURRENT_BINARY_DIR}/..)
set_target_properties(game PROPERTIES RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${CMAKE_CURRENT_BINARY_DIR}/..)

target_link_libraries(game
    PRIVATE
        h2o_core
        h2o_rendering
        h2o_windowing
        h2o_input
        h2o_voxels
)

target_include_directories(game
    PRIVATE
        h2o_core
        h2o_rendering
        h2o_windowing
        h2o_input
        h2o_voxels
        
        include/
)

# Add this resource path
set(RESOURCE_PATHS ${RESOURCE_PATHS};${CMAKE_CURRENT_SOURCE_DIR}/res PARENT_SCOPE)