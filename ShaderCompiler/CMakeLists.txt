cmake_minimum_required(VERSION 3.31.0)

project(ShaderCompiler)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(ShaderCompiler "main.cpp" "Parser.h" "Parser.cpp")


target_include_directories(ShaderCompiler PRIVATE ${Vulkan_INCLUDE_DIR})

set(VULKAN_SDK_PATH $ENV{VULKAN_SDK})
target_link_libraries(ShaderCompiler PRIVATE  

${VULKAN_SDK_PATH}/Lib/glslangd.lib
${VULKAN_SDK_PATH}/Lib/SPIRV-Toolsd.lib
${VULKAN_SDK_PATH}/Lib/SPIRV-Tools-optd.lib
)


set_target_properties(
    ShaderCompiler PROPERTIES

    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/Demo
)