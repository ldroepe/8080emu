
cmake_minimum_required(VERSION 3.12)
project(8080emu 
    VERSION 1.0.0
    DESCRIPTION "8080 Space Invadors Emulator"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(PROJECT_TEST_DIR ${PROJECT_SOURCE_DIR}/../test)

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

set(EMU_INCLUDES
    ${PROJECT_SOURCE_DIR}/disassembler.h
)
set(EMU_SOURCES
    ${EMU_INCLUDES}
    ${PROJECT_SOURCE_DIR}/disassembler.cpp
)
set(EMU_TESTS
    ${EMU_INCLUDES}
    ${PROJECT_TEST_DIR}/decode_test.cpp
)

add_executable(${PROJECT_NAME} ${EMU_SOURCES})

set(PROJECT_TEST_NAME ${PROJECT_NAME}_test)
enable_testing()
add_executable(
    ${PROJECT_TEST_NAME}
    ${EMU_TESTS}
)

target_link_libraries(
    ${PROJECT_TEST_NAME}
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(${PROJECT_TEST_NAME})
