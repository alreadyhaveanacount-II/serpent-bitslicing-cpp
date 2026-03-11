cmake_minimum_required(VERSION 3.15)
project(Serpent-Bitslice VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/O2 /Oi /Ot /Gy /arch:AVX512 /Wall)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
else()
    add_compile_options(-O3 -march=native -mprefer-vector-width=512 -fno-plt)
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

include_directories(include)

file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(${PROJECT_NAME} ${SOURCES})

include(CheckIPOSupported)
check_ipo_supported(RESULT ipo_supported OUTPUT error)
if(ipo_supported)
    set_property(TARGET ${PROJECT_NAME} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()


set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_NAME "serpent_bench"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID}")
