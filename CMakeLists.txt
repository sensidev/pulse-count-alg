cmake_minimum_required(VERSION 3.20)
project(pulse_counter_algorithm)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_VERBOSE_MAKEFILE ON)

add_subdirectory(libs)

if(NOT DEFINED TARGET_GROUP)
    set(TARGET_GROUP release)
endif()


if(TARGET_GROUP STREQUAL release)
    add_subdirectory(src)
elseif(TARGET_GROUP STREQUAL test)
    include(CTest)
    add_subdirectory(test)
endif()


