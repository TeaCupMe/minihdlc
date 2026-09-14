include(FetchContent)

# Fetch GoogleTest dynamically at configure time
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.15.2 # Or your preferred version
)
# Forcing GTest to fit neatly inside your project setup
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(minihdlc_crc_tests test_crc_ccitt.cpp)
target_link_libraries(minihdlc_crc_tests 
    PRIVATE 
        minihdlc              # The lib you want to test
        gtest_main            # Automatic GTest main() function
        gtest                 # Core framework
)

add_executable(minihdlc_tests test_main.cpp)
target_link_libraries(minihdlc_tests 
    PRIVATE 
        minihdlc              # The lib you want to test
        gtest_main            # Automatic GTest main() function
        gtest                 # Core framework
        GTest::gmock_main
)

# Automatically register individual tests with CTest
gtest_discover_tests(minihdlc_crc_tests)
gtest_discover_tests(minihdlc_tests)
