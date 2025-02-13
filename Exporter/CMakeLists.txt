cmake_minimum_required(VERSION 3.31.0)

project(Exporter)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
add_library(Exporter SHARED ${SOURCES})


find_package(Python REQUIRED COMPONENTS Interpreter Development)

target_include_directories(Exporter PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_include_directories(Exporter PRIVATE ${Python_INCLUDE_DIRS})
target_link_libraries(Exporter PRIVATE ${Python_LIBRARIES})

set_target_properties(Exporter PROPERTIES SUFFIX ".pyd")
set_target_properties(Exporter PROPERTIES OUTPUT_NAME "SmoothieExporter")

set(CXXFLAGS "${CXXFLAGS} /permissive-")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /permissive-")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /permissive-")

link_libraries()
set_target_properties( Exporter PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/Exporter
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/Exporter
)