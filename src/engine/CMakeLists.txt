cmake_minimum_required(VERSION 3.31.0)

project(Smoothie_Engine)

file(GLOB_RECURSE Smoothie_Engine_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.h" "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_library(Smoothie_Engine STATIC ${Smoothie_Engine_SOURCES})
target_precompile_headers(Smoothie_Engine PRIVATE Common.h)

add_library(Smoothie_Engine_inc INTERFACE)
target_include_directories(Smoothie_Engine_inc INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${SMOOTHIE_INC_DIR_GLM}
        ${SMOOTHIE_INC_DIR_STB}
        ${SMOOTHIE_INC_DIR_VMA}
        ${SMOOTHIE_INC_DIR_FMT}
)

target_link_libraries(Smoothie_Engine_inc INTERFACE Smoothie_Engine fmt::fmt)
add_library(Smoothie::Engine ALIAS Smoothie_Engine_inc)

target_link_libraries(Smoothie_Engine PRIVATE Smoothie_Engine_inc)