cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_library(h2o_ui
#    src/nuklear/ui_renderer_nuklear.cpp
    src/ui_module.cpp
)

target_link_libraries(h2o_ui
    PRIVATE
        h2o_core
        h2o_input
        h2o_rendering
        h2o_windowing

        glad
        glfw
)

target_include_directories(h2o_ui
    PUBLIC
        include/

    PRIVATE
        h2o_core
        h2o_input
        h2o_rendering
        h2o_windowing

        ../../lib/nuklear
        glad
        glfw
)