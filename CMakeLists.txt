cmake_minimum_required(VERSION 3.11...3.20 FATAL_ERROR)

project(Buglab VERSION 1 LANGUAGES CXX)

set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})

file(GLOB Paths_sources_cpp ./src/*.cpp)
file(GLOB Paths_sources_h ./src/*.h)

list(APPEND Paths_sources ${Paths_sources_cpp})
list(APPEND Paths_sources ${Paths_sources_h})

add_executable(Buglab ${Paths_sources})

target_include_directories(Buglab PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}> $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)
set_target_properties(Buglab PROPERTIES 
                        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
                        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
                        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
                        )

set_property(TARGET Buglab PROPERTY CXX_STANDARD 17)

install(TARGETS Buglab DESTINATION ".")

get_target_property(buglab-tests-src Buglab SOURCES)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/. FILES ${buglab-tests-src})
set_target_properties(Buglab PROPERTIES FOLDER Buglab)

