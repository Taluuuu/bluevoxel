cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_executable(bluevoxel_client
        src/bluevoxel_client_module.cpp
    src/main.cpp
)

set_target_properties(bluevoxel_client PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_BINARY_DIR}/..)
set_target_properties(bluevoxel_client PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_BINARY_DIR}/..)
set_target_properties(bluevoxel_client PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_CURRENT_BINARY_DIR}/..)
set_target_properties(bluevoxel_client PROPERTIES RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${CMAKE_CURRENT_BINARY_DIR}/..)

target_link_libraries(bluevoxel_client
    PRIVATE
        h2o_core
        h2o_voxel
        h2o_voxel_client
)

target_include_directories(bluevoxel_client
    PRIVATE
        h2o_core
        h2o_voxel
        h2o_voxel_client
)