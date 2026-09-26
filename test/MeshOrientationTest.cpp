#include <Geometry/Mesh/AddTriangle.hpp>
#include <Geometry/Mesh/MakeTriangleMesh.hpp>
#include <Geometry/Mesh/MeshOrientation.hpp>
#include <Geometry/Mesh/TriangleHalfedgeMesh.hpp>
#include <gtest/gtest.h>

#include <array>

using namespace Geometry;

namespace
{

using Mesh         = TriangleHalfedgeMesh3d;
using VertexHandle = Mesh::VertexHandle;
using FaceHandle   = Mesh::FaceHandle;

// Single open triangle: 3 boundary edges, no enclosed volume.
Mesh make_single_triangle()
{
  Mesh mesh;
  const VertexHandle vertex0 = mesh.add_vertex({0.0, 0.0, 0.0});
  const VertexHandle vertex1 = mesh.add_vertex({1.0, 0.0, 0.0});
  const VertexHandle vertex2 = mesh.add_vertex({0.0, 1.0, 0.0});
  EXPECT_TRUE(add_triangle(mesh, vertex0, vertex1, vertex2).is_valid());
  return mesh;
}

// Two triangles sharing edge (vertex1 -> vertex2): an open patch, consistently wound.
Mesh make_two_adjacent_triangles()
{
  Mesh mesh;
  const VertexHandle vertex0 = mesh.add_vertex({0.0, 0.0, 0.0});
  const VertexHandle vertex1 = mesh.add_vertex({1.0, 0.0, 0.0});
  const VertexHandle vertex2 = mesh.add_vertex({0.0, 1.0, 0.0});
  const VertexHandle vertex3 = mesh.add_vertex({1.0, 1.0, 0.0});
  EXPECT_TRUE(add_triangle(mesh, vertex0, vertex1, vertex2).is_valid());
  EXPECT_TRUE(add_triangle(mesh, vertex2, vertex1, vertex3).is_valid());
  return mesh;
}

// Closed tetrahedron with outward winding (same construction as MeshTopologyTest).
Mesh make_tetrahedron()
{
  Mesh mesh;
  const VertexHandle vertex0 = mesh.add_vertex({0.0, 0.0, 0.0});
  const VertexHandle vertex1 = mesh.add_vertex({1.0, 0.0, 0.0});
  const VertexHandle vertex2 = mesh.add_vertex({0.0, 1.0, 0.0});
  const VertexHandle vertex3 = mesh.add_vertex({0.0, 0.0, 1.0});
  EXPECT_TRUE(add_triangle(mesh, vertex0, vertex2, vertex1).is_valid());
  EXPECT_TRUE(add_triangle(mesh, vertex0, vertex1, vertex3).is_valid());
  EXPECT_TRUE(add_triangle(mesh, vertex1, vertex2, vertex3).is_valid());
  EXPECT_TRUE(add_triangle(mesh, vertex2, vertex0, vertex3).is_valid());
  return mesh;
}

// Same tetrahedron vertices but every triangle wound the opposite way: inward-facing normals.
Mesh make_inward_tetrahedron()
{
  Mesh mesh;
  const VertexHandle vertex0 = mesh.add_vertex({0.0, 0.0, 0.0});
  const VertexHandle vertex1 = mesh.add_vertex({1.0, 0.0, 0.0});
  const VertexHandle vertex2 = mesh.add_vertex({0.0, 1.0, 0.0});
  const VertexHandle vertex3 = mesh.add_vertex({0.0, 0.0, 1.0});
  EXPECT_TRUE(add_triangle(mesh, vertex0, vertex1, vertex2).is_valid());
  EXPECT_TRUE(add_triangle(mesh, vertex0, vertex3, vertex1).is_valid());
  EXPECT_TRUE(add_triangle(mesh, vertex1, vertex3, vertex2).is_valid());
  EXPECT_TRUE(add_triangle(mesh, vertex2, vertex3, vertex0).is_valid());
  return mesh;
}

} // namespace

// --- Consistency (combinatorial) ------------------------------------------------------------

TEST(MeshOrientationTest, consistencyHoldsForMeshesBuiltByAddTriangle)
{
  // add_triangle preserves consistent winding by construction, so every mesh it builds -- open or
  // closed -- must pass.
  EXPECT_TRUE(is_consistently_oriented(make_single_triangle()));
  EXPECT_TRUE(is_consistently_oriented(make_two_adjacent_triangles()));
  EXPECT_TRUE(is_consistently_oriented(make_tetrahedron()));
  EXPECT_TRUE(is_consistently_oriented(make_inward_tetrahedron()));
  EXPECT_TRUE(is_consistently_oriented(make_triangle_mesh(Cuboid<double>{{1, 1, 1}}).mesh));
}

// --- Global orientation (outward vs inward) -------------------------------------------------

TEST(MeshOrientationTest, closedMeshClassifiesOutwardOrInward)
{
  EXPECT_EQ(mesh_orientation(make_tetrahedron()), MeshOrientation::Outward);
  EXPECT_EQ(is_outward_oriented(make_tetrahedron()), std::optional<bool>{true});

  EXPECT_EQ(mesh_orientation(make_inward_tetrahedron()), MeshOrientation::Inward);
  EXPECT_EQ(is_outward_oriented(make_inward_tetrahedron()), std::optional<bool>{false});
}

TEST(MeshOrientationTest, primitivesAreOutward)
{
  const Segment3d axis{{0, 0, 0}, {0, 0, 2}};
  EXPECT_EQ(mesh_orientation(make_triangle_mesh(Cuboid<double>{{1, 1, 1}}).mesh), MeshOrientation::Outward);
  EXPECT_EQ(mesh_orientation(make_triangle_mesh(Cone<double>{axis, 1}).mesh), MeshOrientation::Outward);
  EXPECT_EQ(mesh_orientation(make_triangle_mesh(Cylinder<double>{axis, 1}).mesh), MeshOrientation::Outward);
}

TEST(MeshOrientationTest, openOrEmptyMeshIsUndefined)
{
  EXPECT_EQ(mesh_orientation(make_single_triangle()), MeshOrientation::Undefined);
  EXPECT_EQ(mesh_orientation(make_two_adjacent_triangles()), MeshOrientation::Undefined);
  EXPECT_EQ(is_outward_oriented(make_single_triangle()), std::nullopt);

  const Mesh empty;
  EXPECT_EQ(mesh_orientation(empty), MeshOrientation::Undefined);
  EXPECT_EQ(is_outward_oriented(empty), std::nullopt);
}

// --- Degenerate faces -----------------------------------------------------------------------

TEST(MeshOrientationTest, degenerateFaceDetected)
{
  Mesh mesh;
  const VertexHandle vertex0 = mesh.add_vertex({0.0, 0.0, 0.0});
  const VertexHandle vertex1 = mesh.add_vertex({1.0, 0.0, 0.0});
  const VertexHandle vertex2 = mesh.add_vertex({2.0, 0.0, 0.0}); // collinear with the other two
  const FaceHandle face = add_triangle(mesh, vertex0, vertex1, vertex2);
  ASSERT_TRUE(face.is_valid());

  EXPECT_TRUE(is_degenerate(mesh, face));
  EXPECT_TRUE(has_degenerate_faces(mesh));
}

TEST(MeshOrientationTest, validMeshHasNoDegenerateFaces)
{
  const Mesh tetrahedron = make_tetrahedron();
  EXPECT_FALSE(has_degenerate_faces(tetrahedron));
  for (const FaceHandle face : tetrahedron.faces())
  {
    EXPECT_FALSE(is_degenerate(tetrahedron, face));
  }
}
