cmake_minimum_required(VERSION 3.1)
include($ENV{HOME}/GlobalCMakeLists.txt)

project(intset LANGUAGES CXX) 

string(REPLACE "-O2" "-O3" newFlags ${CMAKE_CXX_FLAGS_RELEASE})
set(CMAKE_CXX_FLAGS_RELEASE "${newFlags}")
message("CMAKE_CXX_FLAGS_RELEASE=${CMAKE_CXX_FLAGS_RELEASE}")

#if(DEFINED ENV{USE_AVX2})
if(false)
    message("Using AVX2 instructions")
    add_compile_options("-march=native")
    add_definitions(-DUSE_AVX2)
endif()

add_library(${PROJECT_NAME} STATIC "")

target_sources(${PROJECT_NAME}
        PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/kset/kset.cpp
        ${CMAKE_CURRENT_LIST_DIR}/kset/kset.h
)

message("project name: ${PROJECT_NAME}")

target_include_directories(${PROJECT_NAME}
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/kset
    )

message("Adding test dir")
add_subdirectory("test")
add_subdirectory("bench")

