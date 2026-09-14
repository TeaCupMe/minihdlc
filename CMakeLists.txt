cmake_minimum_required(VERSION 3.20)

project(MiniHDLC
        VERSION 0.1.0
        LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_TESTING "Build the test suite" ON)
option(MINIHDLC_TINY_MODE "Build the library in tiny mode" OFF)
option(MINIHDLC_USE_CALLBACK "Build the library with frame-handler callback API" OFF)

if(BUILD_TESTING)
    enable_testing()       # Enables CTest
    include(GoogleTest)    # Brings in gtest_discover_tests
    add_subdirectory(tests)
endif()

message(STATUS "The CMake executable is located at: ${CMAKE_COMMAND}")

add_library(minihdlc
    ./src/minihdlc.cpp
    ./src/crc_ccitt.cpp
)

target_include_directories(minihdlc PUBLIC
    include/
)

# These macros change the public header API/ABI, so they must be PUBLIC.
if(MINIHDLC_TINY_MODE)
    message(STATUS "MINIHDLC_TINY_MODE is ON: pointer/length checks disabled.")
    target_compile_definitions(minihdlc PUBLIC MINIHDLC_TINY)
endif()

if(MINIHDLC_USE_CALLBACK)
    message(STATUS "MINIHDLC_USE_CALLBACK is ON: frame-handler callback API enabled.")
    target_compile_definitions(minihdlc PUBLIC MINIHDLC_USE_CALLBACK)
endif()
