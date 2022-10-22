cmake_minimum_required(VERSION 3.18)
project(application)

set(CMAKE_CXX_STANDARD 20)

add_executable(application 
    src/main.cpp

    src/application.cpp
)

set_target_properties(application PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_BINARY_DIR})
set_target_properties(application PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_BINARY_DIR})
set_target_properties(application PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_CURRENT_BINARY_DIR})
set_target_properties(application PROPERTIES RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(application
    PRIVATE
        core
        rendering
        windowing
)

target_include_directories(application
    PRIVATE
        core
        rendering
        windowing
        
        include/
)