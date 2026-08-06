cmake_minimum_required(VERSION 3.31.0)

project(app-utility)
file(GLOB_RECURSE APP_UTILITY_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/Utility/*.h" "${CMAKE_CURRENT_SOURCE_DIR}/Utility/*.cpp")

project(Demo)
file(GLOB_RECURSE SOURCES_DEMO "${CMAKE_CURRENT_SOURCE_DIR}/demo/*.h" "${CMAKE_CURRENT_SOURCE_DIR}/demo/*.cpp")
add_executable(Demo ${SOURCES_DEMO} ${APP_UTILITY_SOURCES})
target_include_directories(Demo PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(Demo PRIVATE Vulkan::Vulkan)
target_link_libraries(Demo PRIVATE Smoothie::Engine)
target_link_libraries(Demo PRIVATE glfw)

project(asset-pipeline)
file(GLOB_RECURSE SOURCES_ASSET_PIPELINE "${CMAKE_CURRENT_SOURCE_DIR}/asset-pipeline/*.h" "${CMAKE_CURRENT_SOURCE_DIR}/asset-pipeline/*.cpp")
add_executable(asset-pipeline ${SOURCES_ASSET_PIPELINE})
target_link_libraries(asset-pipeline PRIVATE Vulkan::Vulkan)
target_link_libraries(asset-pipeline PRIVATE Smoothie::Asset-Pipeline)

project(smoothie-editor)
file(GLOB_RECURSE SOURCES_EDITOR "${CMAKE_CURRENT_SOURCE_DIR}/editor/*.h" "${CMAKE_CURRENT_SOURCE_DIR}/editor/*.cpp")
add_executable(smoothie-editor
        ${SOURCES_EDITOR}
        ${APP_UTILITY_SOURCES}
        "${SMOOTHIE_INC_DIR_IMGUI}/backends/imgui_impl_glfw.h"
        "${SMOOTHIE_INC_DIR_IMGUI}/backends/imgui_impl_glfw.cpp" #Smoothie::Editor compiles without any windowing library
)

target_include_directories(smoothie-editor PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(smoothie-editor PRIVATE Vulkan::Vulkan)
target_link_libraries(smoothie-editor PRIVATE glfw)
target_link_libraries(smoothie-editor PRIVATE Smoothie::Editor)
target_link_libraries(smoothie-editor PRIVATE Smoothie::Engine)
if (UNIX AND NOT APPLE)
    find_package(X11 REQUIRED)
    target_link_libraries(smoothie-editor PRIVATE ${X11_LIBRARIES})
endif()
