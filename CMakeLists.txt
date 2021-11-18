cmake_minimum_required(VERSION 3.5)
project(thread_pool)

option(BUILD_THREAD_POOL_TESTS "Build ${PROJECT_NAME} tests" OFF)
option(BUILD_THREAD_POOL_EXAMPLES "Build ${PROJECT_NAME} examples" OFF)

set(CMAKE_CXX_STANDARD 14)

add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE include)

if (BUILD_THREAD_POOL_EXAMPLES)
    add_subdirectory(examples)
endif ()

if (BUILD_THREAD_POOL_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif ()

