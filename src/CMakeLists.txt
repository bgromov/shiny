cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

project (shiny)

add_definitions(
    -std=c++11
)

#include_directories ( )

#link_directories ( )

add_executable (test_dm
    test.cpp
)

#target_link_libraries (test_dm
#
#)
