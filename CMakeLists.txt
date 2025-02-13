cmake_minimum_required(VERSION 3.31.0)

project(SmoothieVulkan)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Vulkan REQUIRED)
find_package(Python3 REQUIRED)

add_subdirectory(Engine)
add_subdirectory(Demo)
add_subdirectory(ShaderCompiler)
add_subdirectory(Exporter)
add_dependencies(Demo SmoothieEngine)

link_libraries()