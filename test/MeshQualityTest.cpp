#include <Geometry/Mesh/AddTriangle.hpp>
#include <Geometry/Mesh/MeshQuality.hpp>
#include <Geometry/Mesh/TriangleHalfedgeMesh.hpp>
#include <gtest/gtest.h>

#include <cmath>
#include <numbers>

using namespace Geometry;

namespace
{

using Mesh         = TriangleHalfedgeMesh3d;
using VertexHandle = Mesh::VertexHandle;
using FaceHandle   = Mesh::FaceHandle;

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

} // namespace

// --- Degenerate faces -----------------------------------------------------------------------

TEST(MeshQualityTest, degenerateFaceDetected)
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

TEST(MeshQualityTest, validMeshHasNoDegenerateFaces)
{
  const Mesh tetrahedron = make_tetrahedron();
  EXPECT_FALSE(has_degenerate_faces(tetrahedron));
  for (const FaceHandle face : tetrahedron.faces())
  {
    EXPECT_FALSE(is_degenerate(tetrahedron, face));
  }
}

// --- Folded edges ---------------------------------------------------------------------------

namespace
{

// Two triangles hinged on the edge (0,0,0)-(1,0,0): the first lies in the xy-plane facing +z, the
// second is rotated about the hinge so their normals are `normalAngleDegrees` apart. At 0 the
// patch is flat; towards 180 the second triangle closes onto the first.
struct Hinge
{
  Mesh mesh;
  Mesh::EdgeHandle hinge;
  Mesh::EdgeHandle boundary;
};

Hinge make_hinge(double normalAngleDegrees)
{
  const double angle = normalAngleDegrees * std::numbers::pi / 180.0;
  Hinge result;
  Mesh& mesh = result.mesh;
  const VertexHandle vertex0 = mesh.add_vertex({0.0, 0.0, 0.0});
  const VertexHandle vertex1 = mesh.add_vertex({1.0, 0.0, 0.0});
  const VertexHandle flatApex = mesh.add_vertex({0.5, 1.0, 0.0});
  const VertexHandle hingedApex = mesh.add_vertex({0.5, -std::cos(angle), std::sin(angle)});
  EXPECT_TRUE(add_triangle(mesh, vertex0, vertex1, flatApex).is_valid());
  EXPECT_TRUE(add_triangle(mesh, vertex1, vertex0, hingedApex).is_valid());
  result.hinge = mesh.get_halfedge(mesh.find_halfedge(vertex0, vertex1)).edge;
  result.boundary = mesh.get_halfedge(mesh.find_halfedge(vertex1, flatApex)).edge;
  return result;
}

} // namespace

TEST(MeshQualityTest, flatAndCreasedEdgesAreNotFolded)
{
  for (const double normalAngle : {0.0, 90.0, 178.0})
  {
    const Hinge hinge = make_hinge(normalAngle);
    EXPECT_FALSE(is_folded_edge(hinge.mesh, hinge.hinge)) << normalAngle << " deg";
    EXPECT_FALSE(has_folded_edges(hinge.mesh)) << normalAngle << " deg";
  }
}

TEST(MeshQualityTest, almostClosedHingeIsFolded)
{
  const Hinge hinge = make_hinge(179.5);
  EXPECT_TRUE(is_folded_edge(hinge.mesh, hinge.hinge));
  EXPECT_TRUE(has_folded_edges(hinge.mesh));
}

TEST(MeshQualityTest, foldAngleIsConfigurable)
{
  const Hinge hinge = make_hinge(178.0);
  EXPECT_TRUE(is_folded_edge(hinge.mesh, hinge.hinge, 170.0 * std::numbers::pi / 180.0));
  // Below 90 degrees the limit bounds acute angles too, e.g. to flag every sharp crease.
  const Hinge crease = make_hinge(60.0);
  EXPECT_TRUE(is_folded_edge(crease.mesh, crease.hinge, 45.0 * std::numbers::pi / 180.0));
  EXPECT_FALSE(is_folded_edge(crease.mesh, crease.hinge, 75.0 * std::numbers::pi / 180.0));
}

TEST(MeshQualityTest, boundaryAndDegenerateEdgesAreNotFolded)
{
  const Hinge hinge = make_hinge(179.5);
  EXPECT_FALSE(is_folded_edge(hinge.mesh, hinge.boundary));

  // A face with collinear corners has no normal to compare.
  Mesh mesh;
  const VertexHandle vertex0 = mesh.add_vertex({0.0, 0.0, 0.0});
  const VertexHandle vertex1 = mesh.add_vertex({1.0, 0.0, 0.0});
  const VertexHandle flatApex = mesh.add_vertex({0.5, 1.0, 0.0});
  const VertexHandle collinearApex = mesh.add_vertex({2.0, 0.0, 0.0});
  ASSERT_TRUE(add_triangle(mesh, vertex0, vertex1, flatApex).is_valid());
  ASSERT_TRUE(add_triangle(mesh, vertex1, vertex0, collinearApex).is_valid());
  EXPECT_FALSE(has_folded_edges(mesh));
}
