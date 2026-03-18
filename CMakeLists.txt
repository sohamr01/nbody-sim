cmake_minimum_required(VERSION 3.10)

project(NBodySim)

set(CMAKE_CXX_STANDARD 17)

find_package(glfw3 REQUIRED)

include_directories(external/glad/include)

add_executable(nbody
    src/main.cpp
    external/glad/src/glad.c
)

target_include_directories(nbody PRIVATE
    include
    external/glad/include
)

target_link_libraries(nbody glfw "-framework OpenGL")