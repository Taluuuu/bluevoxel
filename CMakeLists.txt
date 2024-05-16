cmake_minimum_required(VERSION 3.20)

project(bluevoxel)

function(ADD_BLUEVOXEL_EXECUTABLE SUBFOLDER)
    add_subdirectory(${SUBFOLDER})
endfunction()

add_subdirectory(engine)
ADD_BLUEVOXEL_EXECUTABLE(bluevoxel_client)
ADD_BLUEVOXEL_EXECUTABLE(bluevoxel_editor)
ADD_BLUEVOXEL_EXECUTABLE(bluevoxel_server)

# Copy res folder to output directory and rename to "Resources"
file(REMOVE_RECURSE ${CMAKE_CURRENT_BINARY_DIR}/Resources)
file(COPY res DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(RENAME ${CMAKE_CURRENT_BINARY_DIR}/res ${CMAKE_CURRENT_BINARY_DIR}/Resources)