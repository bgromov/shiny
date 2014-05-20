cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

project (shiny)


find_package (PkgConfig REQUIRED)
pkg_check_modules (Log4cxx liblog4cxx)

add_definitions (
    -std=c++11
    -pthread
)

#
# This is needed due to a bug in libstdc++ which causes std::promise to throw "Unknown error -1" 
#
set (CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -Wl,--no-as-needed -pthread")

include_directories (${Log4cxx_INCLUDE_DIRS})
link_directories (${Log4cxx_LIBRARY_DIRS})

add_custom_target (config
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/config ${CMAKE_BINARY_DIR}/config)

add_executable (test_dm
    test.cpp
)
add_dependencies (test_dm config)
target_link_libraries (test_dm
    pthread
    ${Log4cxx_LIBRARIES}
)
