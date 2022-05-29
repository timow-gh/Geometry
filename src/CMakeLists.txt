set(geometry_TARGET_HEADER
        include/Geometry/HalfedgeMesh/Halfedge.hpp
        include/Geometry/HalfedgeMesh/HalfedgeIndices.hpp
        include/Geometry/HalfedgeMesh/Facet.hpp
        include/Geometry/HalfedgeMesh/CalcFaceHalfedges.hpp
        include/Geometry/HalfedgeMesh/MeshPoints.hpp
        include/Geometry/HalfedgeMesh/HalfedgeMesh.hpp
        include/Geometry/HalfedgeMesh/Vertex.hpp
        include/Geometry/HalfedgeMesh/TriangleIndices.hpp
        include/Geometry/HalfedgeMeshBuilder/MeshTriangleAdder.hpp
        include/Geometry/HalfedgeMeshBuilder/CirclePoints.hpp
        include/Geometry/HalfedgeMeshBuilder/ConeMeshBuilder.hpp
        include/Geometry/HalfedgeMeshBuilder/CuboidMeshBuilder.hpp
        include/Geometry/HalfedgeMeshBuilder/CylinderMeshBuilder.hpp
        include/Geometry/HalfedgeMeshBuilder/MeshBuilderBase.hpp
        include/Geometry/HalfedgeMeshBuilder/SphereMeshBuilder.hpp
        include/Geometry/Distance/DistanceLine.hpp
        include/Geometry/Distance/DistanceRay.hpp
        include/Geometry/Distance/DistanceSegment.hpp
        include/Geometry/Distance/DistancePlane.hpp
        include/Geometry/Distance/DistanceSphere.hpp
        include/Geometry/Intersection/IntersectionLine.hpp
        include/Geometry/Intersection/IntersectionPlane.hpp
        include/Geometry/Intersection/IntersectionSegment.hpp
        include/Geometry/Intersection/IntersectionSphere.hpp
        include/Geometry/Transformation/TransformVec.hpp
        include/Geometry/Transformation/TransformSegment.hpp
        include/Geometry/Transformation/TransformCuboid.hpp
        include/Geometry/Transformation/TransformCone.hpp
        include/Geometry/Transformation/TransformCircle.hpp
        include/Geometry/Transformation/TransformCylinder.hpp
        include/Geometry/Transformation/TransformSphere.hpp
        include/Geometry/AABB.hpp
        include/Geometry/Circle.hpp
        include/Geometry/Cone.hpp
        include/Geometry/Cuboid.hpp
        include/Geometry/Segment.hpp
        include/Geometry/Lcs.hpp
        include/Geometry/Sphere.hpp
        include/Geometry/Line.hpp
        include/Geometry/Plane.hpp
        include/Geometry/Polygon.hpp
        include/Geometry/Cylinder.hpp
        include/Geometry/Interval.hpp
        include/Geometry/Triangle.hpp
        include/Geometry/PolyLine.hpp
        include/Geometry/GeometryAssert.hpp include/Geometry/HalfedgeMesh/HalfedgeMeshIndices.hpp include/Geometry/HalfedgeMesh/MeshIndexTraits.hpp include/Geometry/HalfedgeMesh/MeshTraits.hpp)

add_library(tw_geometry INTERFACE ${geometry_TARGET_HEADER})
target_include_directories(tw_geometry
        INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>)
set(TARGET_ALIAS Geometry)
add_library(${PROJECT_NAME}::${TARGET_ALIAS} ALIAS tw_geometry)
set_target_properties(tw_geometry
        PROPERTIES
        LANGUAGE CXX
        LINKER_LANGUAGE CXX
        EXPORT_NAME ${TARGET_ALIAS})
target_link_libraries(tw_geometry
        INTERFACE
        Core::Settings
        Core::Utils
        Core::Types
        Core::Math
        CrossGuid::CrossGuid
        LinAl::LinAl)

install(DIRECTORY "include/Geometry" DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")

install(TARGETS tw_geometry
        EXPORT GeometryTargetsExportSet
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        INCLUDES DESTINATION
        ${CMAKE_INSTALL_INCLUDEDIR}
        ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME})
setupExportSetInstall("${PROJECT_NAME}" "GeometryTargetsExportSet")
