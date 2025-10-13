cmake_minimum_required(VERSION 3.31.0)

project(SmoothieVulkan)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Vulkan REQUIRED)
find_package(Python3 REQUIRED)

add_subdirectory(include/glfw)


set(ENABLE_OPT 0)
#add_subdirectory(include/SPIRV-Tools)
#set(ENABLE_OPT 1)
#set(ALLOW_EXTERNAL_SPIRV_TOOLS 1)
add_subdirectory(include/glslang)

set(CMAKE_DEBUG_POSTFIX "_d")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

add_subdirectory(Engine)
add_subdirectory(Demo)
add_subdirectory(ShaderCompiler)
add_subdirectory(Exporter)
add_subdirectory(Editor)

add_dependencies(Demo glfw)
add_dependencies(Demo SmoothieEngine)
add_dependencies(Exporter Demo)
add_dependencies(ShaderCompiler glslang)
add_dependencies(ShaderCompiler SmoothieEngine)
