include(GNUInstallDirs)

set(geometry_TARGET_HEADER
        include/Geometry/Distance/DistanceLine.hpp
        include/Geometry/Distance/DistanceRay.hpp
        include/Geometry/Distance/DistanceSegment.hpp
        include/Geometry/Distance/DistancePlane.hpp
        include/Geometry/Distance/DistanceSphere.hpp
        include/Geometry/Intersect/BentleyOttmann.hpp
        include/Geometry/Intersect/IntersectAABB.hpp
        include/Geometry/Intersect/IntersectAABBHelper.hpp
        include/Geometry/Intersect/IntersectInterval.hpp
        include/Geometry/Intersect/IntersectLine.hpp
        include/Geometry/Intersect/IntersectPlane.hpp
        include/Geometry/Intersect/IntersectSegment.hpp
        include/Geometry/Intersect/IntersectSphere.hpp
        include/Geometry/Intersect/IntersectTriangle.hpp
        include/Geometry/Transform/TransformVec.hpp
        include/Geometry/Transform/TransformSegment.hpp
        include/Geometry/Transform/TransformCuboid.hpp
        include/Geometry/Transform/TransformCone.hpp
        include/Geometry/Transform/TransformCylinder.hpp
        include/Geometry/Transform/TransformSphere.hpp
        include/Geometry/Transform/TransformAABB.hpp
        include/Geometry/Utils/Assert.hpp
        include/Geometry/Utils/Compiler.hpp
        include/Geometry/Utils/DefaultValues.hpp
        include/Geometry/Utils/GeometryAssert.hpp
        include/Geometry/Utils/Warnings.hpp
        include/Geometry/AABB.hpp
        include/Geometry/Circle.hpp
        include/Geometry/ClosetPointOnLine.hpp
        include/Geometry/Cone.hpp
        include/Geometry/Cuboid.hpp
        include/Geometry/Cylinder.hpp
        include/Geometry/ExtremePointsInDirection.hpp
        include/Geometry/Handle.hpp
        include/Geometry/Interval.hpp
        include/Geometry/KdTree.hpp
        include/Geometry/Segment.hpp
        include/Geometry/Line.hpp
        include/Geometry/Plane.hpp
        include/Geometry/PointQueries.hpp
        include/Geometry/Sphere.hpp
        include/Geometry/Triangle.hpp
        include/Geometry/TriangleHalfedgeMesh.hpp
)

set(TARGET_NAME tw_geometry)
set(TARGET_EXPORT_NAME Geometry)

add_library(${TARGET_NAME} INTERFACE)

target_sources(${TARGET_NAME}
        INTERFACE
        FILE_SET HEADERS
        BASE_DIRS include
        FILES ${geometry_TARGET_HEADER}
)

target_include_directories(${TARGET_NAME}
        INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

set_target_properties(${TARGET_NAME}
        PROPERTIES
        EXPORT_NAME ${TARGET_EXPORT_NAME})

add_library(${PROJECT_NAME}::${TARGET_EXPORT_NAME} ALIAS ${TARGET_NAME})

target_compile_features(${TARGET_NAME} INTERFACE cxx_std_20)

target_link_libraries(${TARGET_NAME} INTERFACE linal::linal)

if (DEFINED ${PROJECT_NAME}_USE_WARNINGS AND ${PROJECT_NAME}_USE_WARNINGS)
    add_warnings_and_compile_options(${TARGET_NAME} "${${PROJECT_NAME}_WARNINGS_AS_ERRORS}")
endif ()

if (${PROJECT_NAME}_INSTALL)
    include(CMakePackageConfigHelpers)

    set(project_export_set_name "${PROJECT_NAME}TargetsExportSet")
    set(config_install_dir "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")

    install(TARGETS ${TARGET_NAME}
            EXPORT ${project_export_set_name}
            FILE_SET HEADERS
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
            COMPONENT dev)

    write_basic_package_version_file(
            "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
            VERSION ${PROJECT_VERSION}
            COMPATIBILITY SameMajorVersion)

    configure_package_config_file(
            "${PROJECT_SOURCE_DIR}/cmake/Config.cmake.in"
            "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
            INSTALL_DESTINATION "${config_install_dir}")

    install(EXPORT ${project_export_set_name}
            NAMESPACE ${PROJECT_NAME}::
            FILE ${PROJECT_NAME}Targets.cmake
            DESTINATION "${config_install_dir}"
            COMPONENT dev)

    export(EXPORT ${project_export_set_name}
            FILE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake"
            NAMESPACE ${PROJECT_NAME}::)

    install(FILES
            "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
            "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
            DESTINATION "${config_install_dir}"
            COMPONENT dev)
endif ()
