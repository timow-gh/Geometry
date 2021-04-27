cmake_minimum_required(VERSION 3.16)

project("Geometry"
        VERSION 0.1.0
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT CMAKE_BUILD_TYPE)
    message(STATUS "No build type selected, default to Release")
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
endif ()

list(APPEND CMAKE_MODULE_PATH
     ${CMAKE_CURRENT_LIST_DIR}/CMake
     ${CMAKE_CURRENT_LIST_DIR}/cmake)

include(CTest)

option(${PROJECT_NAME}_INSTALL "Enable install rule" ON)
option(${PROJECT_NAME}_BUILD_TESTS "Enable Core tests" ON)
option(${PROJECT_NAME}_BUILD_BENCHMARKS "Enable Core tests" OFF)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

if (${PROJECT_NAME}_INSTALL)
    include(GNUInstallDirs)
    include(CMakePackageConfigHelpers)

    if (NOT CMAKE_GENERATOR STREQUAL "Xcode")
        file(RELATIVE_PATH relDir
             ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}
             ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}
             )
        set(CMAKE_INSTALL_RPATH $ORIGIN $ORIGIN/${relDir})
    endif ()
endif ()

find_package(CrossGuid CONFIG REQUIRED)
find_package(Core CONFIG REQUIRED)
find_package(LinAl CONFIG REQUIRED)

set(TARGET_NAME ${PROJECT_NAME})

set(geometry_TARGET_HEADER
    include/Geometry/HalfedgeMesh/Vertex.hpp
    include/Geometry/HalfedgeMesh/Halfedge.hpp
    include/Geometry/HalfedgeMesh/Facet.hpp
    include/Geometry/HalfedgeMesh/FacetUtils.hpp
    include/Geometry/HalfedgeMesh/MeshPoints.hpp
    include/Geometry/HalfedgeMesh/HalfedgeMesh.hpp
    include/Geometry/HalfedgeMesh/HalfedgeMeshBuilder.hpp
    include/Geometry/AABB.hpp
    include/Geometry/Cube.hpp
    include/Geometry/Segment.hpp
    include/Geometry/Lcs.hpp
    include/Geometry/Sphere.hpp
    include/Geometry/Line.hpp
    include/Geometry/Plane.hpp
    include/Geometry/Polygon.hpp
    include/Geometry/Cylinder.hpp
    include/Geometry/Interval.hpp
    include/Geometry/DistanceLine.hpp
    include/Geometry/DistanceRay.hpp
    include/Geometry/DistanceSegment.hpp
    include/Geometry/DistancePlane.hpp
    include/Geometry/IntersectionLine.hpp
    include/Geometry/IntersectionPlane.hpp
    include/Geometry/IntersectionSegment.hpp
    include/Geometry/IntersectionSphere.hpp
    include/Geometry/HalfedgeMesh/CubeMeshBuilder.hpp
    include/Geometry/HalfedgeMesh/SphereMeshBuilder.hpp
    include/Geometry/Triangle.hpp
    include/Geometry/InvalidIndex.hpp
    )

add_library(tw_geometry INTERFACE)
target_include_directories(tw_geometry
                           INTERFACE
                           $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>)
set(TARGET_ALIAS Geometry)
add_library(${PROJECT_NAME}::${TARGET_ALIAS} ALIAS tw_geometry)
set_target_properties(tw_geometry
                      PROPERTIES
                      EXPORT_NAME ${TARGET_ALIAS})
target_link_libraries(tw_geometry INTERFACE Core::Types CrossGuid::CrossGuid LinAl::LinAl)
if (MSVC)
    include(HelperFunctions)
    setMSVCOutputDirectories(tw_geometry)
endif ()

if (${PROJECT_NAME}_BUILD_BENCHMARKS)
    add_subdirectory(benchmark)
endif ()

if (${PROJECT_NAME}_BUILD_TESTS)
    add_subdirectory(tests)
endif ()

if (${PROJECT_NAME}_INSTALL)
    install(DIRECTORY
            "${PROJECT_SOURCE_DIR}/include"
            DESTINATION
            "${CMAKE_INSTALL_INCLUDEDIR}")
    install(TARGETS
            tw_geometry
            EXPORT
            GeometryTargetsExportSet
            RUNTIME
            DESTINATION ${CMAKE_INSTALL_BINDIR}
            LIBRARY
            DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE
            DESTINATION ${CMAKE_INSTALL_LIBDIR}
            INCLUDES
            DESTINATION
            ${CMAKE_INSTALL_INCLUDEDIR}
            ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME})

    set(${PROJECT_NAME}_INSTALL_CMAKEDIR
        "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
        CACHE STRING "Path to install ${PROJECT_NAME} Config*.cmake files to.")
    set(${PROJECT_NAME}_MODULE_INSTALL_DIR
        "${CMAKE_INSTALL_LIBDIR}/cmake"
        CACHE STRING "Path to install ${PROJECT_NAME}'s .cmake  module files to.")

    install(EXPORT GeometryTargetsExportSet
            NAMESPACE Geometry::
            FILE GeometryTargets.cmake
            DESTINATION ${${PROJECT_NAME}_INSTALL_CMAKEDIR})

    write_basic_package_version_file(
            ${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
            VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}
            COMPATIBILITY SameMajorVersion)

    configure_package_config_file(
            ${PROJECT_SOURCE_DIR}/cmake/Config.cmake.in
            ${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
            INSTALL_DESTINATION ${${PROJECT_NAME}_INSTALL_CMAKEDIR}
            PATH_VARS ${PROJECT_NAME}_MODULE_INSTALL_DIR
            NO_SET_AND_CHECK_MACRO
            NO_CHECK_REQUIRED_COMPONENTS_MACRO)

    install(FILES
            "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
            "${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
            DESTINATION ${${PROJECT_NAME}_INSTALL_CMAKEDIR})
endif ()
