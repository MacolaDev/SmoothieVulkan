cmake_minimum_required(VERSION 3.31.0)

project(Exporter)


file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

file(GLOB ImGuiSources 
"${CMAKE_SOURCE_DIR}/include/imgui/*.cpp" 
"${CMAKE_SOURCE_DIR}/include/imgui/*.h"
"${CMAKE_SOURCE_DIR}/include/imgui/backends/imgui_impl_opengl3.h" 
"${CMAKE_SOURCE_DIR}/include/imgui/backends/imgui_impl_opengl3.cpp"
"${CMAKE_SOURCE_DIR}/include/imgui/backends/imgui_impl_glfw.h"
"${CMAKE_SOURCE_DIR}/include/imgui/backends/imgui_impl_glfw.cpp"
)

add_library(Exporter SHARED ${SOURCES} ${ImGuiSources})


find_package(Python REQUIRED COMPONENTS Interpreter Development)

target_include_directories(Exporter PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_include_directories(Exporter PRIVATE ${Python_INCLUDE_DIRS})
target_include_directories(Exporter PRIVATE ${CMAKE_SOURCE_DIR}/Engine)
target_include_directories(Exporter PRIVATE ${CMAKE_SOURCE_DIR}/include/glfw/include)
target_include_directories(Exporter PRIVATE ${CMAKE_SOURCE_DIR}/include/imgui)
target_include_directories(Exporter PRIVATE ${Vulkan_INCLUDE_DIR})
target_include_directories(Exporter PRIVATE ${CMAKE_SOURCE_DIR}/include/VulkanMemoryAllocator/include)

target_link_libraries(Exporter PRIVATE ${Python_LIBRARIES})
target_link_libraries(Exporter PRIVATE SmoothieEngine)
target_link_libraries(Exporter PRIVATE glfw)
target_link_libraries(Exporter PRIVATE opengl32)
target_link_libraries(Exporter PRIVATE Vulkan::Vulkan)

set_target_properties(Exporter PROPERTIES SUFFIX ".pyd")
set_target_properties(Exporter PROPERTIES OUTPUT_NAME "SmoothieExporter")


set_target_properties( Exporter PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/bin/Exporter

    #This build is useful for debugging of python extension module in release mode
    #Blender does not use debug version of python (python_d.exe) so standard debug build cant be used for debugging
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_SOURCE_DIR}/bin/Exporter
)