cmake_minimum_required(VERSION 3.1)

##############################################################
# Some common stuff
##############################################################
option(USE_AVX2 "Use AVX2 intrinsics" ON)
option(USE_GCC "Use gcc instead of clang" OFF)

if(USE_GCC)
    set(CMAKE_CXX_COMPILER g++ CACHE STRING "CXX Compiler")
    set(CMAKE_C_COMPILER gcc CACHE STRING "C Compiler")
else()
    set(CMAKE_CXX_COMPILER clang++ CACHE STRING "CXX Compiler")
    set(CMAKE_C_COMPILER clang CACHE STRING "C Compiler")
endif()

set(CMAKE_CXX_STANDARD 14 CACHE STRING "CXX Standard")
set(CMAKE_VERBOSE_MAKEFILE true CACHE BOOL "verbose make output")
set(THREADS_PREFER_PTHREAD_FLAG ON CACHE STRING "use pthread")

##############################################################
# Define the project
##############################################################

project(intset LANGUAGES CXX) 

# Use -O3 instead of -O2
string(REPLACE "-O2" "-O3" newFlags ${CMAKE_CXX_FLAGS_RELEASE})
set(CMAKE_CXX_FLAGS_RELEASE "${newFlags}")

if(USE_AVX2)
    message("Using AVX2 instructions")
    #Assume that the build/target machines are the same arch and have an AVX2 capable CPU
    add_compile_options("-march=native")
    add_definitions(-DUSE_AVX2)
endif()

add_library(${PROJECT_NAME} STATIC "")

target_sources(${PROJECT_NAME}
        PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/kset/kset.cpp
        ${CMAKE_CURRENT_LIST_DIR}/kset/kset.h
        ${CMAKE_CURRENT_LIST_DIR}/kset/kset_node.h
        ${CMAKE_CURRENT_LIST_DIR}/kset/kset_node.cpp
        ${CMAKE_CURRENT_LIST_DIR}/kset/errors.h
        ${CMAKE_CURRENT_LIST_DIR}/kset/packed_ptr.h
)

find_package(gtest REQUIRED HINTS ${CMAKE_CURRENT_LIST_DIR}/cmake)
target_include_directories(${PROJECT_NAME}
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/kset
    ${GTEST_INCLUDE_DIR}
    )


##############################################################
# Add tests and benchmarks
##############################################################

message("Adding test dir")
add_subdirectory("test")

message("Adding bench dir")
add_subdirectory("bench")

