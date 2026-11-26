cmake_minimum_required(VERSION 3.24)
project(GARUDA VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(GARUDA_PROJECT_NAME "GarudaAppProject" CACHE STRING "GarudaApp")
set(GARUDA_PACKAGE_NAME "com.sehajavastha.garuda" CACHE STRING "GarudaApp")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

include(cmake/Dependencies.cmake)
include(cmake/Shaders.cmake)

add_subdirectory(Engine)
add_subdirectory(Project)
if (NOT ANDROID)
    add_subdirectory(Editor)
endif ()