cmake_minimum_required(VERSION 3.31.0)

project(SmoothieEditor)

file(GLOB ImGuiSources 
"${CMAKE_SOURCE_DIR}/include/imgui/*.cpp" 
"${CMAKE_SOURCE_DIR}/include/imgui/*.h"
"${CMAKE_SOURCE_DIR}/include/imgui/backends/imgui_impl_vulkan.h" 
"${CMAKE_SOURCE_DIR}/include/imgui/backends/imgui_impl_vulkan.cpp"
"${CMAKE_SOURCE_DIR}/include/imgui/backends/imgui_impl_glfw.h"
"${CMAKE_SOURCE_DIR}/include/imgui/backends/imgui_impl_glfw.cpp"
)

file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

add_executable(SmoothieEditor ${SOURCES} ${ImGuiSources} "${CMAKE_SOURCE_DIR}/Demo/Demo.cpp"
        Tools/Tool_Camera.cpp
        Tools/Tool_Camera.h
        Tools/Tool_SceneGraph.cpp
        Tools/Tool_SceneGraph.h)

target_compile_definitions(SmoothieEditor PRIVATE SMOOTHIE_VULKAN_EDITOR)

target_include_directories(SmoothieEditor PRIVATE ${CMAKE_SOURCE_DIR}/include/GLFW/include)
target_include_directories(SmoothieEditor PRIVATE ${CMAKE_SOURCE_DIR}/Engine)
target_include_directories(SmoothieEditor PRIVATE ${CMAKE_SOURCE_DIR}/include/VulkanMemoryAllocator/include)
target_include_directories(SmoothieEditor PRIVATE ${CMAKE_SOURCE_DIR}/include/stb)
target_include_directories(SmoothieEditor PRIVATE ${Vulkan_INCLUDE_DIR})
target_include_directories(SmoothieEditor PRIVATE ${CMAKE_SOURCE_DIR}/Demo)
target_include_directories(SmoothieEditor PRIVATE ${CMAKE_SOURCE_DIR}/include/imgui)

target_link_libraries(SmoothieEditor PRIVATE Vulkan::Vulkan)
target_link_libraries(SmoothieEditor PRIVATE glfw)
target_link_libraries(SmoothieEditor PRIVATE SmoothieEngine)

if (UNIX AND NOT APPLE)
    find_package(X11 REQUIRED)
    target_link_libraries(SmoothieEditor PRIVATE ${X11_LIBRARIES})
endif()

#set_target_properties(
#    SmoothieEditor PROPERTIES
#
#    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/Demo
#    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/Demo
#)
