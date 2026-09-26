#include <Geometry/Mesh/AddTriangle.hpp>
#include <Geometry/Mesh/MeshCollapse.hpp>
#include <Geometry/Mesh/MeshEuler.hpp>
#include <Geometry/Mesh/MeshManifold.hpp>
#include <Geometry/Mesh/TriangleHalfedgeMesh.hpp>
#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <vector>

using namespace Geometry;

namespace
{

using Mesh = TriangleHalfedgeMesh2d;
using VertexHandle = Mesh::VertexHandle;
using HalfedgeHandle = Mesh::HalfedgeHandle;

// 3 x 3 planar grid, counter-clockwise; see make_grid in MeshCollapseTest.cpp. Vertex (i, j) has
// handle value 3 * j + i, so the center (1, 1) is vertex 4 and (2, 1) is vertex 5.
Mesh make_grid3()
{
  Mesh mesh;
  std::vector<VertexHandle> vertices;
  for (std::size_t j = 0; j < 3; ++j)
  {
    for (std::size_t i = 0; i < 3; ++i)
    {
      vertices.push_back(mesh.add_vertex({static_cast<double>(i), static_cast<double>(j)}));
    }
  }
  const auto at = [&](std::size_t i, std::size_t j) { return vertices[j * 3 + i]; };
  // add_triangle rejects a triangle touching the mesh at a used corner through two new edges (a
  // second fan), so every triangle must attach along an existing edge. Row 0 grows through each
  // cell's upper triangle first (it shares the previous cell's right edge); later rows grow through
  // the lower triangle first (it shares the row below).
  for (std::size_t j = 0; j + 1 < 3; ++j)
  {
    for (std::size_t i = 0; i + 1 < 3; ++i)
    {
      const std::array<VertexHandle, 3> lower{at(i, j), at(i + 1, j), at(i + 1, j + 1)};
      const std::array<VertexHandle, 3> upper{at(i, j), at(i + 1, j + 1), at(i, j + 1)};
      for (const auto& triangle : j == 0 ? std::array{upper, lower} : std::array{lower, upper})
      {
        EXPECT_TRUE(add_triangle(mesh, triangle).is_valid());
      }
    }
  }
  return mesh;
}

constexpr VertexHandle center{4};
constexpr VertexHandle rightMiddle{5};

} // namespace

TEST(MeshCollapse2D, InversionCheckUsesSignedArea)
{
  const Mesh mesh = make_grid3();
  const HalfedgeHandle halfedge = mesh.find_halfedge(center, rightMiddle);

  EXPECT_FALSE(collapse_inverts_faces(mesh, halfedge, mesh.get_position(rightMiddle)));
  EXPECT_TRUE(collapse_inverts_faces(mesh, halfedge, Mesh::vec_t{-10.0, -10.0}));
  // Merging onto a neighbour's position flattens the faces shared with that neighbour.
  EXPECT_TRUE(collapse_inverts_faces(mesh, halfedge, mesh.get_position(VertexHandle{3})));
}

TEST(MeshCollapse2D, CollapsesPlanarMesh)
{
  Mesh mesh = make_grid3();

  ASSERT_EQ(collapse_halfedge(mesh, mesh.find_halfedge(center, rightMiddle)), CollapseStatus::Ok);

  EXPECT_EQ(mesh.vertex_count(), 8U);
  EXPECT_EQ(mesh.edge_count(), 13U);
  EXPECT_EQ(mesh.face_count(), 6U);
  EXPECT_EQ(euler_characteristic(mesh), 1);
  EXPECT_TRUE(mesh.has_valid_connectivity());
  EXPECT_TRUE(verify_manifold(mesh));
}
