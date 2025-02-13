cmake_minimum_required(VERSION 3.31.0)

project(Demo)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(Demo 
"main.cpp" 
"${CMAKE_SOURCE_DIR}/include/GLFW/src/win32_platform.h"
"${CMAKE_SOURCE_DIR}/include/GLFW/src/win32_joystick.h"
"${CMAKE_SOURCE_DIR}/include/GLFW/src/win32_init.c"
"${CMAKE_SOURCE_DIR}/include/GLFW/src/win32_joystick.c"
"${CMAKE_SOURCE_DIR}/include/GLFW/src/win32_monitor.c"
"${CMAKE_SOURCE_DIR}/include/GLFW/src/win32_window.c"
"${CMAKE_SOURCE_DIR}/include/GLFW/src/win32_module.c"
"${CMAKE_SOURCE_DIR}/include/GLFW/src/win32_thread.c"
"${CMAKE_SOURCE_DIR}/include/GLFW/src/win32_time.c"
"${CMAKE_SOURCE_DIR}/include/GLFW/src/wgl_context.c"

"${CMAKE_SOURCE_DIR}/include/GLFW/src/internal.h"
"${CMAKE_SOURCE_DIR}/include/GLFW/src/platform.h"
"${CMAKE_SOURCE_DIR}/include/GLFW/src/mappings.h"

"${CMAKE_SOURCE_DIR}/include/GLFW/src/context.c"
"${CMAKE_SOURCE_DIR}/include/GLFW/src/init.c"
"${CMAKE_SOURCE_DIR}/include/GLFW/src/input.c"
"${CMAKE_SOURCE_DIR}/include/GLFW/src/monitor.c"
"${CMAKE_SOURCE_DIR}/include/GLFW/src/platform.c"
"${CMAKE_SOURCE_DIR}/include/GLFW/src/vulkan.c"
"${CMAKE_SOURCE_DIR}/include/GLFW/src/window.c"

"${CMAKE_SOURCE_DIR}/include/GLFW/src/egl_context.c"
"${CMAKE_SOURCE_DIR}/include/GLFW/src/osmesa_context.c"
"${CMAKE_SOURCE_DIR}/include/GLFW/src/null_platform.h"
"${CMAKE_SOURCE_DIR}/include/GLFW/src/null_joystick.h"

"${CMAKE_SOURCE_DIR}/include/GLFW/src/null_init.c"
"${CMAKE_SOURCE_DIR}/include/GLFW/src/null_monitor.c"
"${CMAKE_SOURCE_DIR}/include/GLFW/src/null_window.c"
"${CMAKE_SOURCE_DIR}/include/GLFW/src/null_joystick.c"
)
set(CXXFLAGS "${CXXFLAGS} /permissive-")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /permissive-")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /permissive-")

target_compile_definitions(Demo PRIVATE _GLFW_WIN32)

#Include directories
target_include_directories(Demo PRIVATE ${CMAKE_SOURCE_DIR}/include/GLFW/include)
target_include_directories(Demo PRIVATE ${CMAKE_SOURCE_DIR}/Engine)
target_include_directories(Demo PRIVATE ${CMAKE_SOURCE_DIR}/include/VulkanMemoryAllocator/include)
target_include_directories(Demo PRIVATE ${CMAKE_SOURCE_DIR}/include/stb)
target_include_directories(Demo PRIVATE ${Vulkan_INCLUDE_DIR})

# Get the Vulkan SDK path
set(VULKAN_SDK_PATH $ENV{VULKAN_SDK})

target_link_libraries(Demo PRIVATE  
debug ${CMAKE_SOURCE_DIR}/lib/Debug/SmoothieEngine.lib 
optimized ${CMAKE_SOURCE_DIR}/lib/Release/SmoothieEngine.lib 
Vulkan::Vulkan
)


link_libraries()
set_target_properties(
    Demo PROPERTIES

    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/Demo
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/Demo
)
