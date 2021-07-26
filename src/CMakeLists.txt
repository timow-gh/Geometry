set(geometry_TARGET_HEADER
        include/Geometry/HalfedgeMesh/Vertex.hpp
        include/Geometry/HalfedgeMesh/Halfedge.hpp
        include/Geometry/HalfedgeMesh/Facet.hpp
        include/Geometry/HalfedgeMesh/FacetUtils.hpp
        include/Geometry/HalfedgeMesh/MeshPoints.hpp
        include/Geometry/HalfedgeMesh/HalfedgeMesh.hpp
        include/Geometry/HalfedgeMeshBuilder/HalfedgeMeshBuilder.hpp
        include/Geometry/HalfedgeMeshBuilder/CuboidMeshBuilder.hpp
        include/Geometry/HalfedgeMeshBuilder/SphereMeshBuilder.hpp
        include/Geometry/Distance/DistanceLine.hpp
        include/Geometry/Distance/DistanceRay.hpp
        include/Geometry/Distance/DistanceSegment.hpp
        include/Geometry/Distance/DistancePlane.hpp
        include/Geometry/Intersection/IntersectionLine.hpp
        include/Geometry/Intersection/IntersectionPlane.hpp
        include/Geometry/Intersection/IntersectionSegment.hpp
        include/Geometry/Intersection/IntersectionSphere.hpp
        include/Geometry/AABB.hpp
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
        include/Geometry/InvalidIndex.hpp
        include/Geometry/PolyLine.hpp
        )

add_library(tw_geometry INTERFACE)
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
        Core::Types
        Core::Math
        CrossGuid::CrossGuid
        LinAl::LinAl)
target_compile_features(tw_geometry INTERFACE cxx_std_17)

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
