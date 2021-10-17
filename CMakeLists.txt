cmake_minimum_required(VERSION 3.19)

include(FetchContent)
FetchContent_Declare(twCMakeUtils
        GIT_REPOSITORY https://github.com/timow-gh/twCMakeUtils.git
        GIT_TAG main)
FetchContent_GetProperties(twCMakeUtils)
set(lcs_twCMakeUtils "")
string(TOLOWER "twCMakeUtils" lcs_twCMakeUtils)
if (NOT ${lcs_twCMakeUtils}_POPULATED)
    FetchContent_Populate(twCMakeUtils)
    list(APPEND CMAKE_MODULE_PATH ${${lcs_twCMakeUtils}_SOURCE_DIR}/modules)
    include(CMakeHelperFunctions)
endif ()

project("Geometry"
        VERSION 0.1.0
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

option(${PROJECT_NAME}_BUILD_TESTS "Geometry, enable tests" ON)
option(${PROJECT_NAME}_BUILD_BENCHMARKS "Geometry, enable benchmarks" ON)

preamble()

add_subdirectory(dependencies)

add_subdirectory(src)

if (${PROJECT_NAME}_BUILD_BENCHMARKS)
    add_subdirectory(benchmark)
endif ()

if (${PROJECT_NAME}_BUILD_TESTS)
    include(CTest)
    enable_testing()
    add_subdirectory(tests)
endif ()
