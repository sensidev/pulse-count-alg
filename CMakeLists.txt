project("PulseCountAlgorithm" C CXX)
cmake_minimum_required(VERSION 3.0)


set(CMAKE_CXX_STANDARD 14)
set(CMAKE_VERBOSE_MAKEFILE ON)

add_subdirectory(libs)

include(CTest)

add_subdirectory(test)


