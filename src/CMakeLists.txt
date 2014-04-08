cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

project (shiny)

add_definitions(
    -std=c++11
    -pthread
)

#
# This is needed due to a bug in libstdc++ which causes std::promise to throw "Unknown error -1" 
#

set (CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -Wl,--no-as-needed -pthread")

#include_directories ( )

#link_directories ( )

add_executable (test_dm
    test.cpp
)

#target_link_libraries (test_dm
#
#)
