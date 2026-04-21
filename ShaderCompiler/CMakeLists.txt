cmake_minimum_required(VERSION 3.31.0)

project(ShaderCompiler)
add_executable(ShaderCompiler "main.cpp"
        Compiler.cpp
        Compiler.h)

target_include_directories(ShaderCompiler PRIVATE 
${Vulkan_INCLUDE_DIR}
${CMAKE_SOURCE_DIR}/Engine
${CMAKE_SOURCE_DIR}/include/VulkanMemoryAllocator/include
${CMAKE_SOURCE_DIR}/include
${VULKAN_SDK}
)

target_link_directories(ShaderCompiler PRIVATE ${VULKAN_SDK}/lib)
target_link_libraries(ShaderCompiler PRIVATE slang SmoothieEngine vulkan)