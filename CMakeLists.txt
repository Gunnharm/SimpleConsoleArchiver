cmake_minimum_required(VERSION 3.23)
project(SimpleConsoleArchiver)

set(CMAKE_CXX_STANDARD 20)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} "-static-libgcc -static-libstdc++")

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/tests/)
