set(geometry_TARGET_HEADER
        include/Geometry/HalfedgeMesh/CalcFaceHalfedges.hpp
        include/Geometry/HalfedgeMesh/CalcFacetNormal.hpp
        include/Geometry/HalfedgeMesh/CalcSegmentIndicesHalfedgeIndices.hpp
        include/Geometry/HalfedgeMesh/CalcTriangleIndices.hpp
        include/Geometry/HalfedgeMesh/CalcVertexFacets.hpp
        include/Geometry/HalfedgeMesh/CalcVertexNormals.hpp
        include/Geometry/HalfedgeMesh/ConversionHelper.hpp
        include/Geometry/HalfedgeMesh/DebugHelper.hpp
        include/Geometry/HalfedgeMesh/Facet.hpp
        include/Geometry/HalfedgeMesh/Halfedge.hpp
        include/Geometry/HalfedgeMesh/MeshPoints.hpp
        include/Geometry/HalfedgeMesh/HalfedgeMesh.hpp
        include/Geometry/HalfedgeMesh/Vertex.hpp
        include/Geometry/HalfedgeMesh/MeshIndices.hpp
        include/Geometry/HalfedgeMesh/MeshTraits.hpp
        include/Geometry/HalfedgeMeshBuilder/MeshTriangleAdder.hpp
        include/Geometry/HalfedgeMeshBuilder/DiscretizeCircle.hpp
        include/Geometry/HalfedgeMeshBuilder/ConeMeshBuilder.hpp
        include/Geometry/HalfedgeMeshBuilder/CuboidMeshBuilder.hpp
        include/Geometry/HalfedgeMeshBuilder/CylinderMeshBuilder.hpp
        include/Geometry/HalfedgeMeshBuilder/MeshBuilderBase.hpp
        include/Geometry/HalfedgeMeshBuilder/SphereMeshBuilder.hpp
        include/Geometry/HalfedgeMeshBuilder/MeshBuilder.hpp
        include/Geometry/HalfedgeMeshBuilder/MeshBuilderConfig.hpp
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
        include/Geometry/Sphere.hpp
        include/Geometry/Triangle.hpp
)

set(TARGET_NAME tw_geometry)
set(TARGET_EXPORT_NAME Geometry)
set(${TARGET_NAME}_MAJOR 1)
set(${TARGET_NAME}_MINOR 0)
set(${TARGET_NAME}_PATCH 0)

add_library(${TARGET_NAME} INTERFACE ${geometry_TARGET_HEADER})

target_include_directories(${TARGET_NAME}
        INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/export_header_includes>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
)

set_target_properties(${TARGET_NAME}
        PROPERTIES
        LANGUAGE CXX
        LINKER_LANGUAGE CXX
        DEBUG_POSTFIX d
        VERSION ${${TARGET_NAME}_MAJOR}.${${TARGET_NAME}_MINOR}.${${TARGET_NAME}_PATCH}
        SOVERSION ${PROJECT_VERSION_MAJOR}
        OUTPUT_NAME "tw_${TARGET_EXPORT_NAME}"
        EXPORT_NAME ${TARGET_EXPORT_NAME}
)

add_library(${PROJECT_NAME}::${TARGET_EXPORT_NAME} ALIAS ${TARGET_NAME})

target_link_libraries(${TARGET_NAME} INTERFACE linal::linal)

if (${PROJECT_NAME}_USE_WARNINGS)
    add_warnings_and_compile_options(${TARGET_NAME} "${${PROJECT_NAME}_WARNINGS_AS_ERRORS}")
endif ()

if (${PROJECT_NAME}_STATIC_ANALYSIS)
    enable_static_analysis(${TARGET_NAME} "${${PROJECT_NAME}_WARNINGS_AS_ERRORS}")
endif ()

if (${PROJECT_NAME}_INSTALL)
    include(setup_export_set_installation)

    install(DIRECTORY "include/${TARGET_EXPORT_NAME}" DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")

    install(TARGETS ${TARGET_NAME}
            EXPORT GeometryTargetsExportSet
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            INCLUDES DESTINATION
            ${CMAKE_INSTALL_INCLUDEDIR}
            ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME})

    setup_export_set_installation(
            ${PROJECT_NAME}
            "GeometryTargetsExportSet"
            "Config.cmake.in"
            ${PROJECT_VERSION_MAJOR} ${PROJECT_VERSION_MINOR} ${PROJECT_VERSION_PATCH}
    )
endif ()
