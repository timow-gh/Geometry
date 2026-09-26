#include <Geometry/Cylinder.hpp>
#include <Geometry/Mesh/AddTriangle.hpp>
#include <Geometry/Mesh/MakeTriangleMesh.hpp>
#include <Geometry/Mesh/MeshDelete.hpp>
#include <Geometry/Mesh/MeshEuler.hpp>
#include <Geometry/Mesh/MeshManifold.hpp>
#include <Geometry/Mesh/MeshOrientation.hpp>
#include <Geometry/Mesh/TriangleHalfedgeMesh.hpp>
#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <numbers>
#include <random>
#include <utility>
#include <vector>

using namespace Geometry;

namespace
{

using Mesh = TriangleHalfedgeMesh3d;
using VertexHandle = Mesh::VertexHandle;
using FaceHandle = Mesh::FaceHandle;

// n x n grid in the xy-plane as in MeshCollapseTest.cpp; vertex (i, j) has handle value j * n + i.
Mesh make_grid(std::size_t size)
{
  Mesh mesh;
  std::vector<VertexHandle> vertices;
  for (std::size_t j = 0; j < size; ++j)
  {
    for (std::size_t i = 0; i < size; ++i)
    {
      vertices.push_back(mesh.add_vertex({static_cast<double>(i), static_cast<double>(j), 0.0}));
    }
  }
  const auto at = [&](std::size_t i, std::size_t j) { return vertices[j * size + i]; };
  // Every triangle must attach along an existing edge (add_triangle refuses a second fan at a
  // corner): row 0 grows through each cell's upper triangle first, later rows through the lower.
  for (std::size_t j = 0; j + 1 < size; ++j)
  {
    for (std::size_t i = 0; i + 1 < size; ++i)
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

VertexHandle grid_vertex(std::size_t size, std::uint32_t i, std::uint32_t j)
{
  return VertexHandle{static_cast<std::uint32_t>(j * size + i)};
}

FaceHandle face_with(const Mesh& mesh, VertexHandle first, VertexHandle second, VertexHandle third)
{
  const auto halfedge = mesh.find_halfedge(first, second);
  EXPECT_TRUE(halfedge.is_valid());
  const FaceHandle face = mesh.get_halfedge(halfedge).face;
  EXPECT_EQ(mesh.target_vertex(mesh.get_halfedge(halfedge).next), third);
  return face;
}

// A boundary vertex (handle 0) with an open fan of five triangles over ring vertices 1..6. The
// middle ring vertex 3 touches two fan faces whose far edges at the hub are both interior.
Mesh make_open_fan()
{
  Mesh mesh;
  const VertexHandle hub = mesh.add_vertex({0.0, 0.0, 0.0});
  std::vector<VertexHandle> ring;
  for (int i = 0; i < 6; ++i)
  {
    const double angle = std::numbers::pi * static_cast<double>(i) / 5.0;
    ring.push_back(mesh.add_vertex({std::cos(angle), std::sin(angle), 0.0}));
  }
  for (std::size_t i = 0; i + 1 < ring.size(); ++i)
  {
    EXPECT_TRUE(add_triangle(mesh, hub, ring[i], ring[i + 1]).is_valid());
  }
  return mesh;
}

void expect_structurally_valid(const Mesh& mesh)
{
  EXPECT_TRUE(mesh.has_valid_connectivity());
  EXPECT_TRUE(verify_manifold(mesh));
  EXPECT_TRUE(is_consistently_oriented(mesh));
}

} // namespace

// --- delete_face ------------------------------------------------------------------------------

TEST(MeshDelete, RejectsInvalidAndDeletedFace)
{
  Mesh mesh = make_grid(3);
  EXPECT_EQ(delete_face(mesh, FaceHandle{}), MeshDeleteStatus::InvalidHandle);

  const FaceHandle corner = face_with(mesh, grid_vertex(3, 0, 0), grid_vertex(3, 1, 1), grid_vertex(3, 0, 1));
  ASSERT_EQ(delete_face(mesh, corner), MeshDeleteStatus::Ok);
  EXPECT_EQ(delete_face(mesh, corner), MeshDeleteStatus::InvalidHandle);
}

TEST(MeshDelete, DeletingIsolatedTriangleEmptiesMesh)
{
  Mesh mesh;
  const VertexHandle vertex0 = mesh.add_vertex({0.0, 0.0, 0.0});
  const VertexHandle vertex1 = mesh.add_vertex({1.0, 0.0, 0.0});
  const VertexHandle vertex2 = mesh.add_vertex({0.0, 1.0, 0.0});
  const FaceHandle face = add_triangle(mesh, vertex0, vertex1, vertex2);

  ASSERT_EQ(delete_face(mesh, face), MeshDeleteStatus::Ok);

  EXPECT_EQ(mesh.vertex_count(), 0U);
  EXPECT_EQ(mesh.edge_count(), 0U);
  EXPECT_EQ(mesh.face_count(), 0U);
  EXPECT_TRUE(mesh.empty());
  EXPECT_TRUE(mesh.has_valid_connectivity());
  mesh.garbage_collection();
  EXPECT_EQ(mesh.halfedge_storage_size(), 0U);
}

TEST(MeshDelete, DeletingBoundaryFaceShrinksBoundary)
{
  Mesh mesh = make_grid(3);
  const FaceHandle face = face_with(mesh, grid_vertex(3, 1, 0), grid_vertex(3, 2, 0), grid_vertex(3, 2, 1));

  ASSERT_EQ(delete_face(mesh, face), MeshDeleteStatus::Ok);

  // The corner (2,0) belonged to this face only, so it goes with its two boundary edges.
  EXPECT_TRUE(mesh.is_deleted(grid_vertex(3, 2, 0)));
  EXPECT_EQ(mesh.vertex_count(), 8U);
  EXPECT_EQ(mesh.edge_count(), 14U);
  EXPECT_EQ(mesh.face_count(), 7U);
  EXPECT_EQ(euler_characteristic(mesh), 1);
  EXPECT_EQ(boundary_loops(mesh).size(), 1U);
  expect_structurally_valid(mesh);
}

TEST(MeshDelete, DeletingInteriorFaceOpensHole)
{
  Mesh mesh = make_grid(4);
  const FaceHandle face = face_with(mesh, grid_vertex(4, 1, 1), grid_vertex(4, 2, 1), grid_vertex(4, 2, 2));

  ASSERT_EQ(delete_face(mesh, face), MeshDeleteStatus::Ok);

  EXPECT_EQ(mesh.face_count(), 17U);
  EXPECT_EQ(mesh.edge_count(), 33U);
  EXPECT_EQ(mesh.vertex_count(), 16U);
  EXPECT_EQ(euler_characteristic(mesh), 0);
  EXPECT_EQ(boundary_loops(mesh).size(), 2U);
  expect_structurally_valid(mesh);
}

TEST(MeshDelete, RejectsFaceWhoseDeletionSplitsBoundaryFan)
{
  Mesh mesh = make_open_fan();
  // Fan face (hub, 2, 3) has both edges at the boundary hub interior.
  const FaceHandle middle = face_with(mesh, VertexHandle{0}, VertexHandle{2}, VertexHandle{3});

  EXPECT_EQ(delete_face(mesh, middle), MeshDeleteStatus::NonManifoldVertex);
  EXPECT_FALSE(mesh.has_garbage());

  // Forced through the unchecked kernel, the deletion leaves the hub with two boundary gaps. The
  // splicing chains both fans into one orbit, so only the gap count exposes the bow-tie.
  Mesh bowTie = mesh;
  detail::delete_face_unchecked(bowTie, middle);
  EXPECT_TRUE(bowTie.has_valid_connectivity());
  EXPECT_FALSE(verify_vertex_manifold(bowTie, VertexHandle{0}));
  EXPECT_FALSE(verify_manifold(bowTie));

  // The first face touches the hub's boundary gap, so it can go.
  EXPECT_EQ(delete_face(mesh, face_with(mesh, VertexHandle{0}, VertexHandle{1}, VertexHandle{2})), MeshDeleteStatus::Ok);
  expect_structurally_valid(mesh);
}

TEST(MeshDeleteFuzz, DeletingFacesInAnyLegalOrderStaysValid)
{
  const Cylinder<double> cylinder{Segment3<double>{{0.0, 0.0, 0.0}, {0.0, 0.0, 2.0}}, 1.0};
  for (std::uint32_t seed = 1; seed <= 5; ++seed)
  {
    auto creation = make_triangle_mesh(cylinder, 10);
    ASSERT_TRUE(creation.has_value());
    Mesh mesh = std::move(creation.mesh);
    std::mt19937 generator(seed);
    std::size_t deletedCount = 0;

    // Deletion can get stuck before the mesh is empty: in a band whose vertices are all on the
    // boundary, every face may sit mid-fan at some corner. Delete until then.
    for (bool progress = true; progress && mesh.face_count() > 0;)
    {
      std::vector<FaceHandle> faces(mesh.faces().begin(), mesh.faces().end());
      std::shuffle(faces.begin(), faces.end(), generator);
      progress = false;
      for (const FaceHandle face : faces)
      {
        const std::size_t faceCount = mesh.face_count();
        const MeshDeleteStatus status = delete_face(mesh, face);
        if (status == MeshDeleteStatus::NonManifoldVertex)
        {
          ASSERT_EQ(mesh.face_count(), faceCount);
          continue;
        }
        ASSERT_EQ(status, MeshDeleteStatus::Ok);
        ASSERT_EQ(mesh.face_count(), faceCount - 1);
        ASSERT_TRUE(mesh.has_valid_connectivity()) << "seed " << seed;
        ASSERT_TRUE(verify_manifold(mesh)) << "seed " << seed;
        ASSERT_TRUE(is_consistently_oriented(mesh)) << "seed " << seed;
        ++deletedCount;
        progress = true;
        break;
      }
    }

    EXPECT_GE(deletedCount, 8U) << "seed " << seed;
    mesh.garbage_collection();
    expect_structurally_valid(mesh);
  }
}

// --- delete_vertex ------------------------------------------------------------------------------

TEST(MeshDelete, DeletingInteriorVertexOpensHole)
{
  // The center of a 5 x 5 grid has a one-ring of interior vertices, so its hole stays separate
  // from the outer boundary.
  Mesh mesh = make_grid(5);

  ASSERT_EQ(delete_vertex(mesh, grid_vertex(5, 2, 2)), MeshDeleteStatus::Ok);

  EXPECT_TRUE(mesh.is_deleted(grid_vertex(5, 2, 2)));
  EXPECT_EQ(mesh.vertex_count(), 24U);
  EXPECT_EQ(mesh.face_count(), 26U);
  EXPECT_EQ(boundary_loops(mesh).size(), 2U);
  EXPECT_EQ(euler_characteristic(mesh), 0);
  expect_structurally_valid(mesh);
}

TEST(MeshDelete, DeletingVertexDropsNeighboursLeftIsolated)
{
  Mesh mesh = make_grid(3);

  ASSERT_EQ(delete_vertex(mesh, grid_vertex(3, 1, 1)), MeshDeleteStatus::Ok);

  // Corners (0,0) and (2,2) only had faces around the center; two lone triangles remain.
  EXPECT_TRUE(mesh.is_deleted(grid_vertex(3, 0, 0)));
  EXPECT_TRUE(mesh.is_deleted(grid_vertex(3, 2, 2)));
  EXPECT_EQ(mesh.vertex_count(), 6U);
  EXPECT_EQ(mesh.edge_count(), 6U);
  EXPECT_EQ(mesh.face_count(), 2U);
  EXPECT_EQ(num_connected_components(mesh), 2U);
  expect_structurally_valid(mesh);
}

TEST(MeshDelete, RejectsVertexWhoseDeletionPinchesNeighbour)
{
  Mesh mesh = make_open_fan();
  // Deleting ring vertex 3 removes the two middle fan faces, cutting the hub's fan in two.
  EXPECT_EQ(delete_vertex(mesh, VertexHandle{3}), MeshDeleteStatus::NonManifoldVertex);
  EXPECT_FALSE(mesh.has_garbage());

  // Deleting the hub takes the whole fan with it.
  ASSERT_EQ(delete_vertex(mesh, VertexHandle{0}), MeshDeleteStatus::Ok);
  EXPECT_EQ(mesh.vertex_count(), 0U);
  EXPECT_EQ(mesh.face_count(), 0U);
  EXPECT_TRUE(mesh.has_valid_connectivity());
}

TEST(MeshDelete, DeletesIsolatedVertexAndRejectsInvalid)
{
  Mesh mesh = make_grid(3);
  const VertexHandle isolated = mesh.add_vertex({9.0, 9.0, 0.0});

  EXPECT_EQ(delete_vertex(mesh, isolated), MeshDeleteStatus::Ok);
  EXPECT_TRUE(mesh.is_deleted(isolated));
  EXPECT_EQ(delete_vertex(mesh, isolated), MeshDeleteStatus::InvalidHandle);
  EXPECT_EQ(delete_vertex(mesh, VertexHandle{}), MeshDeleteStatus::InvalidHandle);
}

// Handles to deleted vertices stay in range until garbage_collection(), so add_triangle must reject
// them explicitly; otherwise a live face would reference a tombstoned vertex.
TEST(MeshDelete, AddTriangleRejectsDeletedVertex)
{
  Mesh mesh = make_grid(5);
  const VertexHandle isolated = mesh.add_vertex({9.0, 9.0, 0.0});
  const VertexHandle center = grid_vertex(5, 2, 2);
  ASSERT_EQ(delete_vertex(mesh, isolated), MeshDeleteStatus::Ok);
  ASSERT_EQ(delete_vertex(mesh, center), MeshDeleteStatus::Ok);
  const auto faceCount = mesh.face_count();
  const auto halfedgeCount = mesh.halfedge_count();

  // Refilling one of the hole's original faces through the deleted center.
  EXPECT_FALSE(add_triangle(mesh, grid_vertex(5, 1, 1), grid_vertex(5, 2, 1), center).is_valid());
  const VertexHandle fresh = mesh.add_vertex({9.0, 8.0, 0.0});
  const VertexHandle other = mesh.add_vertex({8.0, 9.0, 0.0});
  EXPECT_FALSE(add_triangle(mesh, isolated, fresh, other).is_valid());

  EXPECT_EQ(mesh.face_count(), faceCount);
  EXPECT_EQ(mesh.halfedge_count(), halfedgeCount);
  expect_structurally_valid(mesh);
  mesh.garbage_collection();
  expect_structurally_valid(mesh);
}

TEST(MeshDelete, DeletingVertexSurvivesIntermediateBowTie)
{
  // Faces go one at a time in fan order, so a boundary neighbour of (1,1) such as (1,0) can lose its
  // face away from the boundary first and pass through a bow-tie; only the final state is checked
  // up front, so the face-by-face splicing must cope.
  Mesh mesh = make_grid(4);
  ASSERT_EQ(delete_vertex(mesh, grid_vertex(4, 1, 1)), MeshDeleteStatus::Ok);
  EXPECT_TRUE(mesh.is_deleted(grid_vertex(4, 0, 0)));
  expect_structurally_valid(mesh);
  mesh.garbage_collection();
  expect_structurally_valid(mesh);
}
