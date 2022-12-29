cmake_minimum_required(VERSION 3.21)

project(VulkanRenderer)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

# Enable c++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug" OR ${CMAKE_BUILD_TYPE} STREQUAL RelWithDebInfo)
  add_compile_definitions("_GLIBCXX_DEBUG")
endif()

set(EXTERNAL_FOLDER "${VulkanRenderer_SOURCE_DIR}/external")
set(MAIN_FOLDER "${VulkanRenderer_SOURCE_DIR}/src")

if(MSVC)
  add_definitions(/MP)
endif()

# set(CMAKE_CXX_FLAGS "-Wall")
if(WIN32)
set(CMAKE_CXX_FLAGS_DEBUG "/ZI /Od")
set(CMAKE_CXX_FLAGS_RELEASE "/O2")
else(WIN32)
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
endif(WIN32)

add_subdirectory("src")