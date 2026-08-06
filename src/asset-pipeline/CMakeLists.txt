cmake_minimum_required(VERSION 3.31.0)

project(Smoothie_AssetPipeline)

file(GLOB_RECURSE Smoothie_AssetPipeline_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/Pipe/*.h" "${CMAKE_CURRENT_SOURCE_DIR}/Pipe/*.cpp")

if (NOT DEFINED SMOOTHIE_ASSET_PIPELINE_INTERFACE_IMPL)
    set(SMOOTHIE_ASSET_PIPELINE_INTERFACE_IMPL "Interface.cpp")
endif ()


add_library(Smoothie_AssetPipeline STATIC ${Smoothie_AssetPipeline_SOURCES} ${SMOOTHIE_ASSET_PIPELINE_INTERFACE_IMPL})
target_precompile_headers(Smoothie_AssetPipeline PRIVATE
        Common.h)


add_library(Smoothie_AssetPipeline_inc INTERFACE)
target_include_directories(Smoothie_AssetPipeline_inc INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${VULKAN_SDK}
        ${Vulkan_INCLUDE_DIR}
        ${SMOOTHIE_INC_DIR_VMA}
        ${SMOOTHIE_INC_DIR_TOML11}
        ${SMOOTHIE_INC_DIR_GLTF}
        ${SMOOTHIE_INC_DIR_FMT}
)

target_link_directories(Smoothie_AssetPipeline_inc INTERFACE ${VULKAN_SDK}/lib)
target_link_libraries(Smoothie_AssetPipeline_inc INTERFACE slang vulkan Smoothie_AssetPipeline toml11::toml11)

target_link_libraries(Smoothie_AssetPipeline PRIVATE Smoothie_AssetPipeline_inc Smoothie::Engine)
add_library(Smoothie::Asset-Pipeline ALIAS Smoothie_AssetPipeline_inc)


