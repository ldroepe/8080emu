
cmake_minimum_required(VERSION 3.12)
project(8080emu 
    VERSION 1.0.0
    DESCRIPTION "8080 Space Invadors Emulator"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(EMU_SOURCES
    ${PROJECT_SOURCE_DIR}/disassembler.cpp
)
add_executable(${PROJECT_NAME} ${EMU_SOURCES})
