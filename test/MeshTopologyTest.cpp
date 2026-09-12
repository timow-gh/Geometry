#include <Geometry/Mesh/AddTriangle.hpp>
#include <Geometry/Mesh/MeshEuler.hpp>
#include <Geometry/Mesh/MeshManifold.hpp>
#include <Geometry/Mesh/MeshTopology.hpp>
#include <Geometry/Mesh/TriangleHalfedgeMesh.hpp>
#include <gtest/gtest.h>

#include <array>

using namespace Geometry;

namespace
{

using Mesh = TriangleHalfedgeMesh3d;
using VertexHandle = Mesh::VertexHandle;
using FaceHandle = Mesh::FaceHandle;

// Single triangle: 3 vertices, 3 edges, 1 face, all on the boundary.
Mesh make_single_triangle()
{
  Mesh mesh;
  const VertexHandle vertex0 = mesh.add_vertex({0.0, 0.0, 0.0});
  const VertexHandle vertex1 = mesh.add_vertex({1.0, 0.0, 0.0});
  const VertexHandle vertex2 = mesh.add_vertex({0.0, 1.0, 0.0});
  const FaceHandle face = add_triangle(mesh, vertex0, vertex1, vertex2);
  EXPECT_TRUE(face.is_valid());
  return mesh;
}

// Two triangles sharing edge (vertex1 -> vertex2): 4 vertices, 5 edges, 2 faces.
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

// Closed tetrahedron: 4 vertices, 6 edges, 4 faces, no boundary. Outward-oriented faces.
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

// Attempts to build two triangles sharing only vertex0 with otherwise disjoint vertices. Under the
// manifold representation add_triangle rejects the second triangle (it would give vertex0 a second
// fan). The mesh therefore retains just the first triangle and stays manifold.

// Two disjoint triangles (no shared vertices): two connected components.
Mesh make_two_disjoint_triangles()
{
  Mesh mesh;
  const VertexHandle vertex0 = mesh.add_vertex({0.0, 0.0, 0.0});
  const VertexHandle vertex1 = mesh.add_vertex({1.0, 0.0, 0.0});
  const VertexHandle vertex2 = mesh.add_vertex({0.0, 1.0, 0.0});
  const VertexHandle vertex3 = mesh.add_vertex({5.0, 5.0, 0.0});
  const VertexHandle vertex4 = mesh.add_vertex({6.0, 5.0, 0.0});
  const VertexHandle vertex5 = mesh.add_vertex({5.0, 6.0, 0.0});
  EXPECT_TRUE(add_triangle(mesh, vertex0, vertex1, vertex2).is_valid());
  EXPECT_TRUE(add_triangle(mesh, vertex3, vertex4, vertex5).is_valid());
  return mesh;
}

} // namespace

// --- MeshTopology (per-element) -------------------------------------------------------------

TEST(MeshTopologyTest, singleTriangleValenceAndBoundary)
{
  const Mesh mesh = make_single_triangle();
  ASSERT_TRUE(mesh.has_valid_connectivity());

  for (const VertexHandle vertex : mesh.vertices())
  {
    EXPECT_EQ(valence(mesh, vertex), 2U);
    EXPECT_TRUE(is_boundary(mesh, vertex));
    EXPECT_FALSE(is_isolated(mesh, vertex));
  }
}

TEST(MeshTopologyTest, isolatedVertex)
{
  Mesh mesh;
  const VertexHandle vertex = mesh.add_vertex({0.0, 0.0, 0.0});
  EXPECT_TRUE(is_isolated(mesh, vertex));
  EXPECT_FALSE(is_boundary(mesh, vertex));
  EXPECT_EQ(valence(mesh, vertex), 0U);
}

TEST(MeshTopologyTest, sharedEdgeVertexValence)
{
  const Mesh mesh = make_two_adjacent_triangles();
  ASSERT_TRUE(mesh.has_valid_connectivity());

  // vertex1 and vertex2 are the endpoints of the shared interior edge; each touches both faces.
  EXPECT_EQ(valence(mesh, VertexHandle{1}), 3U);
  EXPECT_EQ(valence(mesh, VertexHandle{2}), 3U);
}

TEST(MeshTopologyTest, tetrahedronValenceInterior)
{
  const Mesh mesh = make_tetrahedron();
  ASSERT_TRUE(mesh.has_valid_connectivity());

  for (const VertexHandle vertex : mesh.vertices())
  {
    EXPECT_EQ(valence(mesh, vertex), 3U);
    EXPECT_FALSE(is_boundary(mesh, vertex));
  }
}

// --- MeshManifold ---------------------------------------------------------------------------

TEST(MeshManifoldTest, singleTriangleIsOpenManifold)
{
  const Mesh mesh = make_single_triangle();
  EXPECT_TRUE(is_edge_manifold(mesh));
  EXPECT_TRUE(verify_vertex_manifold(mesh));
  EXPECT_TRUE(verify_manifold(mesh));
  EXPECT_FALSE(is_closed(mesh));
}

TEST(MeshManifoldTest, tetrahedronIsClosedManifold)
{
  const Mesh mesh = make_tetrahedron();
  EXPECT_TRUE(verify_manifold(mesh));
  EXPECT_TRUE(is_closed(mesh));
}

TEST(MeshManifoldTest, nonManifoldVertexRejectedAtConstruction)
{
  Mesh mesh;
  const VertexHandle shared = mesh.add_vertex({0.0, 0.0, 0.0});
  const VertexHandle vertex1 = mesh.add_vertex({1.0, 0.0, 0.0});
  const VertexHandle vertex2 = mesh.add_vertex({0.0, 1.0, 0.0});
  const VertexHandle vertex3 = mesh.add_vertex({-1.0, 0.0, 0.0});
  const VertexHandle vertex4 = mesh.add_vertex({0.0, -1.0, 0.0});

  ASSERT_TRUE(add_triangle(mesh, shared, vertex1, vertex2).is_valid());

  // The second triangle shares only the apex vertex; adding it would make the apex non-manifold, so
  // add_triangle rejects it and leaves the mesh unchanged and manifold.
  EXPECT_FALSE(add_triangle(mesh, shared, vertex3, vertex4).is_valid());
  EXPECT_EQ(mesh.face_count(), 1U);
  ASSERT_TRUE(mesh.has_valid_connectivity());

  EXPECT_TRUE(verify_vertex_manifold(mesh, shared));
  EXPECT_TRUE(verify_vertex_manifold(mesh));
  EXPECT_TRUE(verify_manifold(mesh));
  EXPECT_TRUE(is_edge_manifold(mesh));
}

// The whole-mesh verify_vertex_manifold overload uses a single O(V+H) pass rather than calling the
// per-vertex verifier V times; assert the two agree on every API-built fixture.
TEST(MeshManifoldTest, wholeMeshVerifyMatchesPerVertex)
{
  const std::array<Mesh, 4> meshes = {make_single_triangle(),
                                      make_two_adjacent_triangles(),
                                      make_tetrahedron(),
                                      make_two_disjoint_triangles()};
  for (const Mesh& mesh : meshes)
  {
    bool allVerify = true;
    for (const VertexHandle vertex : mesh.vertices())
    {
      allVerify = allVerify && verify_vertex_manifold(mesh, vertex);
    }
    EXPECT_EQ(verify_vertex_manifold(mesh), allVerify);
    EXPECT_TRUE(allVerify); // add_triangle only builds manifold meshes
  }
}

// verify_vertex_manifold must terminate on a malformed fan whose twin.next chain never closes (only
// reachable by hand-building through the raw connectivity view). Corrupt a halfedge's twin to create
// such a chain and assert the verifier returns rather than looping forever.
TEST(MeshManifoldTest, brokenFanDoesNotHang)
{
  Mesh mesh = make_two_adjacent_triangles();
  ASSERT_TRUE(mesh.has_valid_connectivity());

  // Redirect vertex1's stored outgoing halfedge into a self-twin, so twin.next diverges from the
  // start without ever returning to it. The bounded orbit must give up past halfedge_count.
  const VertexHandle vertex1{1};
  const Mesh::HalfedgeHandle stored = mesh.get_vertex(vertex1).halfedge;
  ASSERT_TRUE(stored.is_valid());
  mesh.connectivity().halfedge(stored).twin = stored;

  EXPECT_FALSE(verify_vertex_manifold(mesh, vertex1));
  EXPECT_FALSE(verify_vertex_manifold(mesh));
  // has_valid_connectivity relies on the same twin.next fan walk; it must terminate (its walks are
  // bounded by the halfedge count) and report the corruption rather than hang.
  EXPECT_FALSE(mesh.has_valid_connectivity());
}

// --- MeshEuler ------------------------------------------------------------------------------

TEST(MeshEulerTest, singleTriangleCharacteristic)
{
  const Mesh mesh = make_single_triangle();
  EXPECT_EQ(euler_characteristic(mesh), 1);
  EXPECT_EQ(boundary_loops(mesh).size(), 1U);
  EXPECT_EQ(num_connected_components(mesh), 1U);
  // A single triangle is a disc: connected, manifold, one boundary loop, chi = 1, so the genus
  // formula g = (2 - b - chi) / 2 yields 0. Genus counts handles and is defined for surfaces with
  // boundary, not only closed ones.
  const auto surfaceGenus = genus(mesh);
  ASSERT_TRUE(surfaceGenus.has_value());
  EXPECT_EQ(*surfaceGenus, 0U);
}

TEST(MeshEulerTest, twoAdjacentTrianglesCharacteristic)
{
  const Mesh mesh = make_two_adjacent_triangles();
  EXPECT_EQ(euler_characteristic(mesh), 1);
  EXPECT_EQ(boundary_loops(mesh).size(), 1U);

  // The single boundary loop visits all four outer boundary halfedges.
  const auto loops = boundary_loops(mesh);
  ASSERT_EQ(loops.size(), 1U);
  EXPECT_EQ(loops.front().size(), 4U);
}

TEST(MeshEulerTest, tetrahedronIsGenusZeroSphere)
{
  const Mesh mesh = make_tetrahedron();
  EXPECT_EQ(euler_characteristic(mesh), 2);
  EXPECT_TRUE(boundary_loops(mesh).empty());
  EXPECT_EQ(num_connected_components(mesh), 1U);

  const auto surfaceGenus = genus(mesh);
  ASSERT_TRUE(surfaceGenus.has_value());
  EXPECT_EQ(*surfaceGenus, 0U);
}

// An isolated vertex is manifold-legal (add_vertex creates one before it is wired into a face) and is
// ignored by num_connected_components and verify_manifold; genus must ignore it too rather than let
// it inflate the Euler characteristic and skew the result.
TEST(MeshEulerTest, genusIgnoresIsolatedVertex)
{
  Mesh mesh = make_tetrahedron();
  (void)mesh.add_vertex({9.0, 9.0, 9.0}); // isolated, unused vertex

  EXPECT_EQ(num_connected_components(mesh), 1U);
  EXPECT_TRUE(verify_manifold(mesh));

  const auto surfaceGenus = genus(mesh);
  ASSERT_TRUE(surfaceGenus.has_value());
  EXPECT_EQ(*surfaceGenus, 0U);
}

TEST(MeshEulerTest, disjointTrianglesHaveTwoComponents)
{
  const Mesh mesh = make_two_disjoint_triangles();
  ASSERT_TRUE(mesh.has_valid_connectivity());
  EXPECT_EQ(num_connected_components(mesh), 2U);
  EXPECT_EQ(boundary_loops(mesh).size(), 2U);
  // genus assumes a connected mesh as a precondition and no longer guards against a disconnected one,
  // so the user must gate on num_connected_components themselves; genus is not called here.
}
