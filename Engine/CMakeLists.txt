cmake_minimum_required(VERSION 3.31.0)

project(SmoothieEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.h" "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_library(SmoothieEngine STATIC ${SOURCES})

target_precompile_headers(SmoothieEngine PRIVATE Core/Common.h)

target_include_directories(SmoothieEngine PRIVATE ${CMAKE_SOURCE_DIR}/include/VulkanMemoryAllocator/include)
target_include_directories(SmoothieEngine PRIVATE ${CMAKE_SOURCE_DIR}/include/glm)
target_include_directories(SmoothieEngine PRIVATE ${CMAKE_SOURCE_DIR}/include/stb)
target_include_directories(SmoothieEngine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(SmoothieEngine PRIVATE ${Vulkan_INCLUDE_DIR})
