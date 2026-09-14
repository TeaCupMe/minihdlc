cmake_minimum_required(VERSION 3.20)

project(MiniHDLC
        VERSION 0.1.0
        LANGUAGES CXX
)

option(MINIHDLC_TINY_MODE "Build the library in tiny mode" OFF)
option(MINIHDLC_USE_CALLBACK "Build the library with frame-handler callback API" OFF)
option(MINIHDLC_EMBEDDED_FLAGS "Add bare-metal C++ flags (-fno-exceptions/-fno-rtti/...)" OFF)
set(MINIHDLC_MAX_FRAME_LENGTH "64" CACHE STRING "Maximum HDLC frame payload length")

add_library(minihdlc
    src/minihdlc.cpp
    src/crc_ccitt.cpp
)
add_library(minihdlc::minihdlc ALIAS minihdlc)

target_compile_features(minihdlc PUBLIC cxx_std_17)

target_include_directories(minihdlc PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

# These macros change the public header API/ABI, so they must be PUBLIC.
target_compile_definitions(minihdlc PUBLIC
    MINIHDLC_MAX_FRAME_LENGTH=${MINIHDLC_MAX_FRAME_LENGTH}
)

if(MINIHDLC_TINY_MODE)
    message(STATUS "MINIHDLC_TINY_MODE is ON: pointer/length checks disabled.")
    target_compile_definitions(minihdlc PUBLIC MINIHDLC_TINY)
endif()

if(MINIHDLC_USE_CALLBACK)
    message(STATUS "MINIHDLC_USE_CALLBACK is ON: frame-handler callback API enabled.")
    target_compile_definitions(minihdlc PUBLIC MINIHDLC_USE_CALLBACK)
endif()

if(MINIHDLC_EMBEDDED_FLAGS OR (CMAKE_SYSTEM_NAME STREQUAL "Generic"))
    message(STATUS "Applying embedded C++ flags to minihdlc.")
    target_compile_options(minihdlc PRIVATE
        $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>
        $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
        $<$<COMPILE_LANGUAGE:CXX>:-fno-threadsafe-statics>
    )
endif()

# Tests only when this repo is the top-level project (not an add_subdirectory consumer).
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    option(BUILD_TESTING "Build the test suite" ON)
    if(BUILD_TESTING)
        enable_testing()
        include(GoogleTest)
        add_subdirectory(tests)
    endif()
endif()
