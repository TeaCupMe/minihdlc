cmake_minimum_required(VERSION 3.20)

project(MiniHDLC
        VERSION 0.1.0
        LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)



option(BUILD_TESTING "Build the test suite" ON)
option(MINIHDLC_TINY_MODE "Build the library in tiny mode" OFF)

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

if(MINIHDLC_TINY_MODE)
    message("MINIHDLC_TINY_MODE is set, building the library in tiny mode. Pointer/length checks will be disabled.")
    target_compile_definitions(minihdlc PRIVATE "MINIHDLC_TINY")
endif()