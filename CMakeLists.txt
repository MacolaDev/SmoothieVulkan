cmake_minimum_required(VERSION 3.31.0)

project(SmoothieVulkan)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Vulkan REQUIRED)
set(VULKAN_SDK $ENV{VULKAN_SDK})

find_package(Python3 REQUIRED)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

set(SMOOTHIE_INC_DIR_VMA "${CMAKE_SOURCE_DIR}/include/VulkanMemoryAllocator/include")
set(SMOOTHIE_INC_DIR_STB "${CMAKE_SOURCE_DIR}/include/stb")
set(SMOOTHIE_INC_DIR_GLM "${CMAKE_SOURCE_DIR}/include/glm")
set(SMOOTHIE_INC_DIR_IMGUI "${CMAKE_SOURCE_DIR}/include/imgui")

set(SMOOTHIE_INC_DIR_GLTF "${CMAKE_SOURCE_DIR}/include/tinygltf")
add_subdirectory(include/glfw)

set(SMOOTHIE_INC_DIR_TOML11 "${CMAKE_SOURCE_DIR}/include/toml11")
add_subdirectory(include/toml11)

set(SMOOTHIE_INC_DIR_FMT "${CMAKE_SOURCE_DIR}/include/fmt/include")
add_subdirectory(include/fmt)

add_subdirectory(src/engine)
add_subdirectory(src/apps)
add_subdirectory(src/asset-pipeline)
add_subdirectory(src/editor)

add_dependencies(Demo Smoothie_Engine glfw)
add_dependencies(Smoothie_Editor Smoothie_Engine toml11::toml11)
add_dependencies(smoothie-editor Smoothie_Editor)
add_dependencies(Smoothie_AssetPipeline Smoothie_Engine toml11::toml11)
add_dependencies(asset-pipeline Smoothie_Engine Smoothie_AssetPipeline)

