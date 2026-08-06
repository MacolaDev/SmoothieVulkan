cmake_minimum_required(VERSION 3.31.0)

project(Smoothie_Editor)

file(GLOB Smoothie_Editor_SOURCES_ImGui
        "${SMOOTHIE_INC_DIR_IMGUI}/*.cpp"
        "${SMOOTHIE_INC_DIR_IMGUI}/*.h"
        "${SMOOTHIE_INC_DIR_IMGUI}/backends/imgui_impl_vulkan.h"
        "${SMOOTHIE_INC_DIR_IMGUI}/backends/imgui_impl_vulkan.cpp")

file(GLOB_RECURSE Smoothie_Editor_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

add_library(Smoothie_Editor STATIC ${Smoothie_Editor_SOURCES} ${Smoothie_Editor_SOURCES_ImGui})
target_precompile_headers(Smoothie_Editor PRIVATE Common.h)

add_library(Smoothie_Editor_inc INTERFACE)
target_include_directories(Smoothie_Editor_inc INTERFACE
        ${CMAKE_SOURCE_DIR}/src/engine
        ${CMAKE_SOURCE_DIR}/src/editor
        ${SMOOTHIE_INC_DIR_GLM}
        ${SMOOTHIE_INC_DIR_STB}
        ${SMOOTHIE_INC_DIR_VMA}
        ${SMOOTHIE_INC_DIR_FMT}
        ${SMOOTHIE_INC_DIR_IMGUI}
)

target_link_libraries(Smoothie_Editor_inc INTERFACE Smoothie_Engine Smoothie_Editor)
add_library(Smoothie::Editor ALIAS Smoothie_Editor_inc)
target_link_libraries(Smoothie_Editor PRIVATE Smoothie_Editor_inc)