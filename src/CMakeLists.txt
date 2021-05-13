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
    include/Geometry/Distance/DistanceLine.hpp
    include/Geometry/Distance/DistanceRay.hpp
    include/Geometry/Distance/DistanceSegment.hpp
    include/Geometry/Distance/DistancePlane.hpp
    include/Geometry/Intersection/IntersectionLine.hpp
    include/Geometry/Intersection/IntersectionPlane.hpp
    include/Geometry/Intersection/IntersectionSegment.hpp
    include/Geometry/Intersection/IntersectionSphere.hpp
    include/Geometry/HalfedgeMesh/CubeMeshBuilder.hpp
    include/Geometry/HalfedgeMesh/SphereMeshBuilder.hpp
    include/Geometry/Triangle.hpp
    include/Geometry/InvalidIndex.hpp
    )

add_library(tw_geometry INTERFACE)
target_include_directories(tw_geometry
                           INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>)
set(TARGET_ALIAS Geometry)
add_library(${PROJECT_NAME}::${TARGET_ALIAS} ALIAS tw_geometry)
set_target_properties(tw_geometry
                      PROPERTIES
                      EXPORT_NAME ${TARGET_ALIAS})
target_link_libraries(tw_geometry INTERFACE Core::Types Core::Math CrossGuid::CrossGuid LinAl::LinAl)
if (MSVC)
    include(HelperFunctions)
    setMSVCOutputDirectories(tw_geometry)
endif ()

if (${PROJECT_NAME}_INSTALL)
    install(DIRECTORY "include/Geometry" DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")

    install(TARGETS tw_geometry
            EXPORT GeometryTargetsExportSet
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            INCLUDES DESTINATION
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