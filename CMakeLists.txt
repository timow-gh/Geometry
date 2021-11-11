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

set(CMAKE_CXX_STANDARD 20)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

if ((CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR))
    option(Geometry_BUILD_TESTS "Geometry: enable tests" ON)
    option(Geometry_BUILD_BENCHMARKS "Geometry: enable benchmarks" ON)
else ()
    option(Geometry_BUILD_TESTS "Geometry: enable tests" OFF)
    option(Geometry_BUILD_BENCHMARKS "Geometry: enable benchmarks" OFF)
endif ()

preamble()

add_subdirectory(dependencies)

add_subdirectory(src)

if (Geometry_BUILD_BENCHMARKS)
    add_subdirectory(benchmark)
endif ()

if (Geometry_BUILD_TESTS)
    include(CTest)
    enable_testing()
    add_subdirectory(tests)
endif ()
