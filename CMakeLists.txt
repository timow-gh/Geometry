cmake_minimum_required(VERSION 3.16)

project("Geometry"
        VERSION 0.1.0
        LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS NO)
set(BUILD_SHARED_LIBS ON)
option(${PROJECT_NAME}_INSTALL "Enable install rule" ON)
if ((CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR))
    option(${PROJECT_NAME}_BUILD_TESTS "Enable tests" ON)
    option(${PROJECT_NAME}_BUILD_BENCHMARKS "Enable Core tests" ON)
else ()
    option(${PROJECT_NAME}_BUILD_TESTS "Enable tests" OFF)
    option(${PROJECT_NAME}_BUILD_BENCHMARKS "Enable Core tests" OFF)
endif ()

if (NOT CMAKE_BUILD_TYPE)
    message(STATUS "No build type selected, default to Release")
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
endif ()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

if (${PROJECT_NAME}_INSTALL)
    include(GNUInstallDirs)
    include(CMakePackageConfigHelpers)

    if (NOT CMAKE_GENERATOR STREQUAL "Xcode")
        file(RELATIVE_PATH relDir
             ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}
             ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
        set(CMAKE_INSTALL_RPATH $ORIGIN $ORIGIN/${relDir})
    endif ()
endif ()

add_subdirectory(depenencies)

add_subdirectory(src)

if (${PROJECT_NAME}_BUILD_BENCHMARKS)
    add_subdirectory(benchmark)
endif ()

if (${PROJECT_NAME}_BUILD_TESTS)
    include(CTest)
    add_subdirectory(tests)
endif ()
