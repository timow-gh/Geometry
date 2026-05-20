#include <Geometry/TriangleHalfedgeMesh.hpp>
#include <gtest/gtest.h>

using namespace Geometry;

namespace
{

using Mesh = TriangleHalfedgeMesh3d;
using HalfedgeHandle = Mesh::HalfedgeHandle;
using VertexHandle = Mesh::VertexHandle;

HalfedgeHandle find_halfedge(const Mesh& mesh, Mesh::FaceHandle face, VertexHandle source, VertexHandle target)
{
  for (HalfedgeHandle halfedge : mesh.halfedges_around_face(face))
  {
    if (mesh.source_vertex(halfedge) == source && mesh.target_vertex(halfedge) == target)
    {
      return halfedge;
    }
  }

  return HalfedgeHandle{};
}

} // namespace

TEST(TriangleHalfedgeMeshTest, defaultConstructorCreatesEmptyValidMesh)
{
  Mesh const mesh;

  EXPECT_TRUE(mesh.empty());
  EXPECT_TRUE(mesh.is_valid());
  EXPECT_EQ(mesh.vertex_count(), 0U);
  EXPECT_EQ(mesh.halfedge_count(), 0U);
  EXPECT_EQ(mesh.edge_count(), 0U);
  EXPECT_EQ(mesh.face_count(), 0U);
}

TEST(TriangleHalfedgeMeshTest, addVertexStoresPosition)
{
  Mesh mesh;

  Mesh::VertexHandle const vertex = mesh.add_vertex({1.0, 2.0, 3.0});

  EXPECT_TRUE(vertex.is_valid());
  EXPECT_TRUE(mesh.contains(vertex));
  EXPECT_EQ(mesh.vertex_count(), 1U);
  EXPECT_EQ(mesh.get_position(vertex), (linal::double3{1.0, 2.0, 3.0}));
}

TEST(TriangleHalfedgeMeshTest, addTriangleCreatesFaceCycle)
{
  Mesh mesh;
  Mesh::VertexHandle const v0 = mesh.add_vertex({0.0, 0.0, 0.0});
  Mesh::VertexHandle const v1 = mesh.add_vertex({1.0, 0.0, 0.0});
  Mesh::VertexHandle const v2 = mesh.add_vertex({0.0, 1.0, 0.0});

  Mesh::FaceHandle const face = mesh.add_triangle(v0, v1, v2);

  ASSERT_TRUE(face.is_valid());
  ASSERT_TRUE(mesh.is_valid());
  EXPECT_EQ(mesh.vertex_count(), 3U);
  EXPECT_EQ(mesh.halfedge_count(), 3U);
  EXPECT_EQ(mesh.edge_count(), 3U);
  EXPECT_EQ(mesh.face_count(), 1U);

  std::array<HalfedgeHandle, 3> const halfedges = mesh.halfedges_around_face(face);
  EXPECT_EQ(mesh.source_vertex(halfedges[0]), v0);
  EXPECT_EQ(mesh.target_vertex(halfedges[0]), v1);
  EXPECT_EQ(mesh.source_vertex(halfedges[1]), v1);
  EXPECT_EQ(mesh.target_vertex(halfedges[1]), v2);
  EXPECT_EQ(mesh.source_vertex(halfedges[2]), v2);
  EXPECT_EQ(mesh.target_vertex(halfedges[2]), v0);

  EXPECT_TRUE(mesh.is_boundary(halfedges[0]));
  EXPECT_TRUE(mesh.is_boundary(halfedges[1]));
  EXPECT_TRUE(mesh.is_boundary(halfedges[2]));
}

TEST(TriangleHalfedgeMeshTest, adjacentTrianglesShareTwinHalfedges)
{
  Mesh mesh;
  Mesh::VertexHandle const v0 = mesh.add_vertex({0.0, 0.0, 0.0});
  Mesh::VertexHandle const v1 = mesh.add_vertex({1.0, 0.0, 0.0});
  Mesh::VertexHandle const v2 = mesh.add_vertex({0.0, 1.0, 0.0});
  Mesh::VertexHandle const v3 = mesh.add_vertex({1.0, 1.0, 0.0});

  Mesh::FaceHandle const first = mesh.add_triangle(v0, v1, v2);
  Mesh::FaceHandle const second = mesh.add_triangle(v2, v1, v3);

  ASSERT_TRUE(first.is_valid());
  ASSERT_TRUE(second.is_valid());
  ASSERT_TRUE(mesh.is_valid());
  EXPECT_EQ(mesh.face_count(), 2U);
  EXPECT_EQ(mesh.halfedge_count(), 6U);
  EXPECT_EQ(mesh.edge_count(), 5U);

  HalfedgeHandle const firstShared = find_halfedge(mesh, first, v1, v2);
  HalfedgeHandle const secondShared = find_halfedge(mesh, second, v2, v1);

  ASSERT_TRUE(firstShared.is_valid());
  ASSERT_TRUE(secondShared.is_valid());
  EXPECT_EQ(mesh.get_halfedge(firstShared).twin, secondShared);
  EXPECT_EQ(mesh.get_halfedge(secondShared).twin, firstShared);
  EXPECT_FALSE(mesh.is_boundary(firstShared));
  EXPECT_FALSE(mesh.is_boundary(secondShared));
}

TEST(TriangleHalfedgeMeshTest, rejectsInvalidTrianglesWithoutChangingMesh)
{
  Mesh mesh;
  Mesh::VertexHandle const v0 = mesh.add_vertex({0.0, 0.0, 0.0});
  Mesh::VertexHandle const v1 = mesh.add_vertex({1.0, 0.0, 0.0});
  Mesh::VertexHandle const v2 = mesh.add_vertex({0.0, 1.0, 0.0});

  EXPECT_FALSE(mesh.add_triangle(v0, v1, v1).is_valid());
  EXPECT_FALSE(mesh.add_triangle(v0, v1, Mesh::VertexHandle{}).is_valid());
  EXPECT_EQ(mesh.face_count(), 0U);
  EXPECT_EQ(mesh.halfedge_count(), 0U);
  EXPECT_EQ(mesh.edge_count(), 0U);

  ASSERT_TRUE(mesh.add_triangle(v0, v1, v2).is_valid());
  EXPECT_FALSE(mesh.add_triangle(v0, v1, v2).is_valid());
  EXPECT_EQ(mesh.face_count(), 1U);
  EXPECT_EQ(mesh.halfedge_count(), 3U);
  EXPECT_EQ(mesh.edge_count(), 3U);
  EXPECT_TRUE(mesh.is_valid());
}

TEST(TriangleHalfedgeMeshTest, findsIncidentFacesAndHalfedges)
{
  Mesh mesh;
  Mesh::VertexHandle const v0 = mesh.add_vertex({0.0, 0.0, 0.0});
  Mesh::VertexHandle const v1 = mesh.add_vertex({1.0, 0.0, 0.0});
  Mesh::VertexHandle const v2 = mesh.add_vertex({0.0, 1.0, 0.0});
  Mesh::VertexHandle const v3 = mesh.add_vertex({1.0, 1.0, 0.0});

  ASSERT_TRUE(mesh.add_triangle(v0, v1, v2).is_valid());
  ASSERT_TRUE(mesh.add_triangle(v2, v1, v3).is_valid());

  std::vector<HalfedgeHandle> const halfedges = mesh.halfedges_around_vertex(v1);
  std::vector<Mesh::FaceHandle> const faces = mesh.faces_around_vertex(v1);

  EXPECT_EQ(halfedges.size(), 2U);
  EXPECT_EQ(faces.size(), 2U);
}
