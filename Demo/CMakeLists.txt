cmake_minimum_required(VERSION 3.31.0)

project(Demo)

add_executable(Demo 
"main.cpp"
"Demo.h"
"Demo.cpp"
)

#Include directories
target_include_directories(Demo PRIVATE ${CMAKE_SOURCE_DIR}/include/GLFW/include)
target_include_directories(Demo PRIVATE ${CMAKE_SOURCE_DIR}/Engine)
target_include_directories(Demo PRIVATE ${CMAKE_SOURCE_DIR}/include/VulkanMemoryAllocator/include)
target_include_directories(Demo PRIVATE ${CMAKE_SOURCE_DIR}/include/stb)
target_include_directories(Demo PRIVATE ${Vulkan_INCLUDE_DIR})
target_link_libraries(Demo PRIVATE Vulkan::Vulkan)
target_link_libraries(Demo PRIVATE glfw)
target_link_libraries(Demo PRIVATE SmoothieEngine)

#set_target_properties(
#    Demo PROPERTIES
#
#    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/Demo
#    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/Demo
#)
