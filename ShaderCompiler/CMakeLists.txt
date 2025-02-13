cmake_minimum_required(VERSION 3.31.0)

project(ShaderCompiler)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(ShaderCompiler "main.cpp" "Parser.h" "Parser.cpp")


target_include_directories(ShaderCompiler PRIVATE ${Vulkan_INCLUDE_DIR})

set(VULKAN_SDK_PATH $ENV{VULKAN_SDK})
target_link_libraries(ShaderCompiler PRIVATE  

debug ${VULKAN_SDK_PATH}/Lib/glslangd.lib
debug ${VULKAN_SDK_PATH}/Lib/SPIRV-Toolsd.lib
debug ${VULKAN_SDK_PATH}/Lib/SPIRV-Tools-optd.lib
debug ${VULKAN_SDK_PATH}/Lib/glslang-default-resource-limitsd.lib

optimized  ${VULKAN_SDK_PATH}/Lib/glslang.lib
optimized  ${VULKAN_SDK_PATH}/Lib/SPIRV-Tools.lib
optimized  ${VULKAN_SDK_PATH}/Lib/SPIRV-Tools-opt.lib
optimized  ${VULKAN_SDK_PATH}/Lib/glslang-default-resource-limits.lib
)


set_target_properties(
    ShaderCompiler PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/Demo
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/Demo
)