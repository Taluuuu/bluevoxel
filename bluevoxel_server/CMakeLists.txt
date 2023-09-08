cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_executable(bluevoxel_server
    src/server_module.cpp
    src/main.cpp
)

set_target_properties(bluevoxel_server PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_BINARY_DIR}/..)
set_target_properties(bluevoxel_server PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_BINARY_DIR}/..)
set_target_properties(bluevoxel_server PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_CURRENT_BINARY_DIR}/..)
set_target_properties(bluevoxel_server PROPERTIES RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${CMAKE_CURRENT_BINARY_DIR}/..)

target_link_libraries(bluevoxel_server
    PRIVATE
        h2o_core
        h2o_voxel
        h2o_voxel_server
)

target_include_directories(bluevoxel_server
    PRIVATE
        h2o_core
        h2o_voxel
        h2o_voxel_server
)

# Add this resource path
set(RESOURCE_PATHS ${RESOURCE_PATHS};${CMAKE_CURRENT_SOURCE_DIR}/res PARENT_SCOPE)