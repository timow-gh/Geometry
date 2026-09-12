#include <Geometry/Mesh/AddTriangle.hpp>
#include <Geometry/Mesh/MeshEuler.hpp>
#include <Geometry/Mesh/MeshManifold.hpp>
#include <Geometry/Mesh/MeshTopology.hpp>
#include <Geometry/Mesh/TriangleHalfedgeMesh.hpp>
#include <gtest/gtest.h>

#include <algorithm>
#include <cstddef>
#include <random>
#include <vector>

using namespace Geometry;

namespace
{

using Mesh = TriangleHalfedgeMesh3d;
using VertexHandle = Mesh::VertexHandle;
using HalfedgeHandle = Mesh::HalfedgeHandle;
using FaceHandle = Mesh::FaceHandle;

// Independent structural properties that TriangleHalfedgeMesh::has_valid_connectivity() does not, by
// itself, establish. These cross-check the connectivity against counts/formulas the writer did not
// produce, which is what catches a boundary loop that is internally consistent but globally mis-threaded.
void check_invariants(const Mesh& mesh)
{
  ASSERT_TRUE(mesh.has_valid_connectivity());
  ASSERT_TRUE(is_edge_manifold(mesh));
  // Exhaustively verify vertex-manifoldness: this fuzz test exists to prove add_triangle maintains the
  // manifold invariant, so it must independently confirm no detached second fan was produced.
  ASSERT_TRUE(verify_vertex_manifold(mesh));

  // Every halfedge has a valid twin that is an involution.
  for (const HalfedgeHandle halfedge : mesh.halfedges())
  {
    const HalfedgeHandle twin = mesh.get_halfedge(halfedge).twin;
    ASSERT_TRUE(twin.is_valid());
    EXPECT_EQ(mesh.get_halfedge(twin).twin, halfedge);
  }

  // Boundary halfedges chain into closed loops of only no-face halfedges; total boundary halfedges
  // equal the sum of loop lengths (independent count).
  std::size_t boundaryCount = 0;
  for (const HalfedgeHandle halfedge : mesh.halfedges())
  {
    const auto& meshHalfedge = mesh.get_halfedge(halfedge);
    if (meshHalfedge.is_boundary())
    {
      ++boundaryCount;
      // next and prev of a boundary halfedge are boundary halfedges.
      EXPECT_TRUE(mesh.get_halfedge(meshHalfedge.next).is_boundary());
      EXPECT_TRUE(mesh.get_halfedge(meshHalfedge.prev).is_boundary());
    }
  }

  std::size_t loopHalfedgeSum = 0;
  for (const auto& loop : boundary_loops(mesh))
  {
    loopHalfedgeSum += loop.size();
    ASSERT_FALSE(loop.empty());
    // Walk the loop by next and confirm it closes with exactly loop.size() steps.
    HalfedgeHandle current = loop.front();
    for (std::size_t i = 0; i < loop.size(); ++i)
    {
      EXPECT_TRUE(mesh.get_halfedge(current).is_boundary());
      current = mesh.get_halfedge(current).next;
    }
    EXPECT_EQ(current, loop.front());
  }
  EXPECT_EQ(loopHalfedgeSum, boundaryCount);

  // Every vertex fan orbit (twin.next) closes and its outgoing count matches an independent scan.
  for (const VertexHandle vertex : mesh.vertices())
  {
    if (is_isolated(mesh, vertex))
    {
      continue;
    }
    const HalfedgeHandle start = mesh.get_vertex(vertex).halfedge;
    std::size_t orbit = 0;
    HalfedgeHandle current = start;
    bool completedOrbit = false;
    while (!completedOrbit)
    {
      ++orbit;
      ASSERT_LE(orbit, mesh.halfedge_count()); // guard against an open/broken fan
      current = mesh.get_halfedge(mesh.get_halfedge(current).twin).next;
      completedOrbit = current == start;
    }

    std::size_t scan = 0;
    for (const HalfedgeHandle halfedge : mesh.halfedges())
    {
      if (mesh.source_vertex(halfedge) == vertex)
      {
        ++scan;
      }
    }
    EXPECT_EQ(orbit, scan);

    // A boundary vertex references a boundary outgoing halfedge.
    if (is_boundary(mesh, vertex))
    {
      EXPECT_TRUE(mesh.get_halfedge(start).is_boundary());
    }
  }
}

// A single connected component built purely by add_triangle is a manifold triangulated disk (genus 0
// with one boundary loop) or closed sphere: euler_characteristic = V - E + F. We assert the identity
// holds after every insertion (V,E,F are counts, so this is an independent check of edge_count).
void check_euler(const Mesh& mesh)
{
  const auto chi = static_cast<long long>(mesh.vertex_count()) - static_cast<long long>(mesh.edge_count())
                   + static_cast<long long>(mesh.face_count());
  EXPECT_EQ(euler_characteristic(mesh), chi);
}

// Builds a (rows x cols) grid of vertices and triangulates each cell into two triangles, inserting the
// triangles in the given order. Asserts invariants after each successful insertion.
void build_grid(std::size_t rows, std::size_t cols, const std::vector<std::size_t>& order)
{
  Mesh mesh;
  std::vector<std::vector<VertexHandle>> grid(rows, std::vector<VertexHandle>(cols));
  for (std::size_t row = 0; row < rows; ++row)
  {
    for (std::size_t column = 0; column < cols; ++column)
    {
      grid[row][column] = mesh.add_vertex({static_cast<double>(column), static_cast<double>(row), 0.0});
    }
  }

  // Each cell (r,c) with r<rows-1, c<cols-1 yields two triangles with consistent (CCW) winding.
  struct Tri
  {
    VertexHandle a, b, c;
  };
  std::vector<Tri> tris;
  for (std::size_t row = 0; row + 1 < rows; ++row)
  {
    for (std::size_t column = 0; column + 1 < cols; ++column)
    {
      const VertexHandle v00 = grid[row][column];
      const VertexHandle v10 = grid[row + 1][column];
      const VertexHandle v01 = grid[row][column + 1];
      const VertexHandle v11 = grid[row + 1][column + 1];
      tris.push_back({v00, v01, v11});
      tris.push_back({v00, v11, v10});
    }
  }

  for (const std::size_t idx : order)
  {
    const Tri& triangle = tris[idx];
    const FaceHandle face = add_triangle(mesh, triangle.a, triangle.b, triangle.c);
    // Some insertion orders make a triangle temporarily non-addable (would create a non-manifold
    // vertex before its neighbours exist). Those legitimately return invalid; skip them.
    if (!face.is_valid())
    {
      continue;
    }
    check_invariants(mesh);
    check_euler(mesh);
  }
}

TEST(AddTriangleFuzzTest, SingleTriangleInvariants)
{
  Mesh mesh;
  const VertexHandle vertex0 = mesh.add_vertex({0.0, 0.0, 0.0});
  const VertexHandle vertex1 = mesh.add_vertex({1.0, 0.0, 0.0});
  const VertexHandle vertex2 = mesh.add_vertex({0.0, 1.0, 0.0});
  ASSERT_TRUE(add_triangle(mesh, vertex0, vertex1, vertex2).is_valid());

  check_invariants(mesh);
  EXPECT_EQ(mesh.halfedge_count(), 6U); // 3 interior + 3 boundary
  EXPECT_EQ(mesh.edge_count(), 3U);
  ASSERT_EQ(boundary_loops(mesh).size(), 1U);
  EXPECT_EQ(boundary_loops(mesh).front().size(), 3U);
}

TEST(AddTriangleFuzzTest, TwoAdjacentTrianglesInvariants)
{
  Mesh mesh;
  const VertexHandle vertex0 = mesh.add_vertex({0.0, 0.0, 0.0});
  const VertexHandle vertex1 = mesh.add_vertex({1.0, 0.0, 0.0});
  const VertexHandle vertex2 = mesh.add_vertex({0.0, 1.0, 0.0});
  const VertexHandle vertex3 = mesh.add_vertex({1.0, 1.0, 0.0});
  ASSERT_TRUE(add_triangle(mesh, vertex0, vertex1, vertex2).is_valid());
  ASSERT_TRUE(add_triangle(mesh, vertex2, vertex1, vertex3).is_valid());

  check_invariants(mesh);
  EXPECT_EQ(mesh.edge_count(), 5U);
  ASSERT_EQ(boundary_loops(mesh).size(), 1U);
  EXPECT_EQ(boundary_loops(mesh).front().size(), 4U);
}

TEST(AddTriangleFuzzTest, GridInOrder)
{
  for (std::size_t dimension = 2; dimension <= 5; ++dimension)
  {
    std::vector<std::size_t> order(2 * (dimension - 1) * (dimension - 1));
    for (std::size_t i = 0; i < order.size(); ++i)
    {
      order[i] = i;
    }
    build_grid(dimension, dimension, order);
  }
}

TEST(AddTriangleFuzzTest, GridRandomOrders)
{
  std::mt19937 rng(12345);
  for (std::size_t trial = 0; trial < 400; ++trial)
  {
    const std::size_t dimension = 2 + (rng() % 6); // 2..7
    std::vector<std::size_t> order(2 * (dimension - 1) * (dimension - 1));
    for (std::size_t i = 0; i < order.size(); ++i)
    {
      order[i] = i;
    }
    std::shuffle(order.begin(), order.end(), rng);
    build_grid(dimension, dimension, order);
  }
}

TEST(AddTriangleFuzzTest, ClosedTetrahedronHasNoBoundary)
{
  Mesh mesh;
  const VertexHandle vertex0 = mesh.add_vertex({0.0, 0.0, 0.0});
  const VertexHandle vertex1 = mesh.add_vertex({1.0, 0.0, 0.0});
  const VertexHandle vertex2 = mesh.add_vertex({0.0, 1.0, 0.0});
  const VertexHandle vertex3 = mesh.add_vertex({0.0, 0.0, 1.0});
  ASSERT_TRUE(add_triangle(mesh, vertex0, vertex2, vertex1).is_valid());
  ASSERT_TRUE(add_triangle(mesh, vertex0, vertex1, vertex3).is_valid());
  ASSERT_TRUE(add_triangle(mesh, vertex1, vertex2, vertex3).is_valid());
  ASSERT_TRUE(add_triangle(mesh, vertex2, vertex0, vertex3).is_valid());

  check_invariants(mesh);
  EXPECT_TRUE(is_closed(mesh));
  EXPECT_EQ(boundary_loops(mesh).size(), 0U);
  EXPECT_EQ(mesh.halfedge_count(), 12U); // 6 edges x 2, all interior
}

TEST(AddTriangleFuzzTest, RejectsNonManifoldVertex)
{
  Mesh mesh;
  const VertexHandle shared = mesh.add_vertex({0.0, 0.0, 0.0});
  const VertexHandle vertex1 = mesh.add_vertex({1.0, 0.0, 0.0});
  const VertexHandle vertex2 = mesh.add_vertex({0.0, 1.0, 0.0});
  const VertexHandle vertex3 = mesh.add_vertex({-1.0, 0.0, 0.0});
  const VertexHandle vertex4 = mesh.add_vertex({0.0, -1.0, 0.0});

  ASSERT_TRUE(add_triangle(mesh, shared, vertex1, vertex2).is_valid());
  const FaceHandle second = add_triangle(mesh, shared, vertex3, vertex4);
  EXPECT_FALSE(second.is_valid());
  check_invariants(mesh);
  EXPECT_EQ(mesh.face_count(), 1U);
}

} // namespace
