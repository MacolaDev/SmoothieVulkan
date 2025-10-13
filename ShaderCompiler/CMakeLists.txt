cmake_minimum_required(VERSION 3.31.0)

project(ShaderCompiler)
add_executable(ShaderCompiler "main.cpp" "Parser.h" "Parser.cpp")

target_include_directories(ShaderCompiler PRIVATE 
${Vulkan_INCLUDE_DIR}
${CMAKE_SOURCE_DIR}/Engine
)

target_link_libraries(ShaderCompiler PRIVATE  
glslang::glslang 
glslang::SPIRV 
glslang::glslang-default-resource-limits
SmoothieEngine
)


#set_target_properties(
#    ShaderCompiler PROPERTIES
#    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/Demo
#    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/Demo
#)