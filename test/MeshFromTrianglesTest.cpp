#include <Geometry/Cuboid.hpp>
#include <Geometry/Cylinder.hpp>
#include <Geometry/Mesh/MakeTriangleMesh.hpp>
#include <Geometry/Mesh/MeshBuffers.hpp>
#include <Geometry/Mesh/MeshFromTriangles.hpp>
#include <Geometry/Mesh/MeshGlobalTopology.hpp>
#include <Geometry/Mesh/MeshOrientation.hpp>
#include <Geometry/Mesh/MeshTopology.hpp>
#include <Geometry/Mesh/MeshVerify.hpp>
#include <Geometry/Mesh/TriangleHalfedgeMesh.hpp>
#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <random>
#include <span>
#include <utility>
#include <vector>

using namespace Geometry;

// A named namespace rather than an anonymous one: this file also instantiates the library for
// float, 8-bit-index and 2D meshes, whose local Mesh/handle aliases would otherwise hide the ones
// below (MSVC C4459).
namespace MeshFromTrianglesTesting
{

using Mesh = TriangleHalfedgeMesh3d;
using VertexHandle = Mesh::VertexHandle;
using HalfedgeHandle = Mesh::HalfedgeHandle;
using EdgeHandle = Mesh::EdgeHandle;
using FaceHandle = Mesh::FaceHandle;
using Vec3 = Mesh::vec_t;
using Triangle = std::array<std::uint32_t, 3>;
using Status = MeshFromTrianglesStatus;

struct TriangleSoup
{
  std::vector<Vec3> positions;
  std::vector<Triangle> triangles;
};

MeshFromTrianglesResult<double, 3, std::uint32_t> build(const TriangleSoup& soup)
{
  return make_mesh_from_triangles(std::span<const Vec3>{soup.positions}, std::span<const Triangle>{soup.triangles});
}

// Positions in handle order and one triangle per face; requires a mesh without garbage, so that
// handle values are the indices.
TriangleSoup soup_of(const Mesh& mesh)
{
  EXPECT_FALSE(mesh.has_garbage());
  TriangleSoup soup;
  for (const VertexHandle vertex : mesh.vertices())
  {
    soup.positions.push_back(mesh.get_position(vertex));
  }
  for (const FaceHandle face : mesh.faces())
  {
    const auto corners = mesh.vertices_around_face(face);
    soup.triangles.push_back({corners[0].get_value(), corners[1].get_value(), corners[2].get_value()});
  }
  return soup;
}

// Shuffles the triangle order and rotates each triangle's corners, which keeps its winding.
void shuffle_soup(TriangleSoup& soup, std::uint32_t seed)
{
  std::mt19937 generator{seed};
  std::ranges::shuffle(soup.triangles, generator);
  std::uniform_int_distribution<int> rotation{0, 2};
  for (Triangle& triangle : soup.triangles)
  {
    std::ranges::rotate(triangle, triangle.begin() + rotation(generator));
  }
}

// Rotates each triangle to start at its smallest index, then sorts: equal results describe the
// same faces with the same windings.
std::vector<Triangle> canonical_triangles(std::vector<Triangle> triangles)
{
  for (Triangle& triangle : triangles)
  {
    std::ranges::rotate(triangle, std::ranges::min_element(triangle));
  }
  std::ranges::sort(triangles);
  return triangles;
}

std::vector<Triangle> canonical_triangles(const Mesh& mesh)
{
  return canonical_triangles(soup_of(mesh).triangles);
}

// n x n vertex grid in the xy-plane, vertex (i, j) at index j * n + i, each cell split along its
// (i, j)-(i+1, j+1) diagonal; the cell at hole, if given, is left out.
TriangleSoup make_grid_soup(std::uint32_t size, std::optional<std::array<std::uint32_t, 2>> hole = std::nullopt)
{
  TriangleSoup soup;
  for (std::uint32_t j = 0; j < size; ++j)
  {
    for (std::uint32_t i = 0; i < size; ++i)
    {
      soup.positions.push_back({static_cast<double>(i), static_cast<double>(j), 0.0});
    }
  }
  const auto at = [size](std::uint32_t i, std::uint32_t j) { return j * size + i; };
  for (std::uint32_t j = 0; j + 1 < size; ++j)
  {
    for (std::uint32_t i = 0; i + 1 < size; ++i)
    {
      if (hole == std::array{i, j})
      {
        continue;
      }
      soup.triangles.push_back({at(i, j), at(i + 1, j), at(i + 1, j + 1)});
      soup.triangles.push_back({at(i, j), at(i + 1, j + 1), at(i, j + 1)});
    }
  }
  return soup;
}

// Positions for the hand-written soups below; the builder never looks at them.
std::vector<Vec3> make_positions(std::size_t count)
{
  std::vector<Vec3> positions;
  for (std::size_t i = 0; i < count; ++i)
  {
    positions.push_back({static_cast<double>(i), static_cast<double>(i * i), 1.0});
  }
  return positions;
}

std::vector<std::size_t> sorted_boundary_loop_sizes(const Mesh& mesh)
{
  std::vector<std::size_t> sizes;
  for (const auto& loop : boundary_loops(mesh))
  {
    sizes.push_back(loop.size());
  }
  std::ranges::sort(sizes);
  return sizes;
}

void expect_valid_manifold(const Mesh& mesh)
{
  EXPECT_TRUE(mesh.has_valid_connectivity());
  EXPECT_TRUE(verify_manifold(mesh));
  EXPECT_TRUE(is_consistently_oriented(mesh));
}

void expect_failure(const TriangleSoup& soup, Status expected)
{
  const auto result = build(soup);
  EXPECT_EQ(result.error, expected);
  EXPECT_FALSE(result.has_value());
  EXPECT_TRUE(result.mesh.empty());
  EXPECT_EQ(result.mesh.halfedge_count(), 0U);
}

Mesh make_cube()
{
  auto creation = make_triangle_mesh(Cuboid<double>{{1.0, 2.0, 3.0}});
  EXPECT_TRUE(creation.has_value());
  return std::move(creation.mesh);
}

Mesh make_cylinder()
{
  auto creation = make_triangle_mesh(Cylinder<double>{Segment3<double>{{0.0, 0.0, 0.0}, {0.0, 0.0, 2.0}}, 1.0}, 12);
  EXPECT_TRUE(creation.has_value());
  return std::move(creation.mesh);
}

// Rebuilds a closed operand from a shuffled soup and checks it is the same surface.
void expect_shuffled_round_trip(const Mesh& original)
{
  const std::vector<Triangle> expected = canonical_triangles(original);
  for (std::uint32_t seed = 0; seed < 8; ++seed)
  {
    TriangleSoup soup = soup_of(original);
    shuffle_soup(soup, seed);

    const auto result = build(soup);
    ASSERT_TRUE(result.has_value()) << "seed " << seed;
    const Mesh& mesh = result.mesh;
    expect_valid_manifold(mesh);
    EXPECT_TRUE(verify_closed(mesh));
    EXPECT_EQ(is_outward_oriented(mesh), std::optional<bool>{true});
    EXPECT_EQ(mesh.vertex_count(), original.vertex_count());
    EXPECT_EQ(mesh.edge_count(), original.edge_count());
    EXPECT_EQ(mesh.face_count(), original.face_count());
    EXPECT_EQ(euler_characteristic(mesh), 2);
    EXPECT_EQ(canonical_triangles(mesh), expected);
  }
}

TEST(MeshFromTrianglesTest, EmptyInputGivesEmptyMesh)
{
  const auto result = build({});
  ASSERT_TRUE(result.has_value());
  EXPECT_TRUE(result.mesh.empty());
  EXPECT_EQ(result.mesh.halfedge_count(), 0U);
}

TEST(MeshFromTrianglesTest, UnreferencedPositionsBecomeIsolatedVertices)
{
  const TriangleSoup soup{make_positions(4), {{0, 1, 2}}};
  const auto result = build(soup);
  ASSERT_TRUE(result.has_value());
  const Mesh& mesh = result.mesh;
  EXPECT_EQ(mesh.vertex_count(), 4U);
  EXPECT_TRUE(is_isolated(mesh, VertexHandle{3}));
  EXPECT_EQ(mesh.get_position(VertexHandle{3}), soup.positions[3]);
  expect_valid_manifold(mesh);
}

TEST(MeshFromTrianglesTest, SingleTriangleGetsBoundaryLoop)
{
  const TriangleSoup soup{make_positions(3), {{2, 0, 1}}};
  const auto result = build(soup);
  ASSERT_TRUE(result.has_value());
  const Mesh& mesh = result.mesh;
  expect_valid_manifold(mesh);
  EXPECT_EQ(mesh.edge_count(), 3U);
  EXPECT_EQ(mesh.face_count(), 1U);
  EXPECT_EQ(sorted_boundary_loop_sizes(mesh), (std::vector<std::size_t>{3}));
  for (const VertexHandle vertex : mesh.vertices())
  {
    EXPECT_TRUE(is_boundary(mesh, vertex));
  }
  // A boundary edge stores its face-side halfedge, as add_triangle does.
  for (const EdgeHandle edge : mesh.edges())
  {
    EXPECT_FALSE(mesh.is_boundary(mesh.get_edge(edge).halfedge));
  }
}

TEST(MeshFromTrianglesTest, HandlesFollowTheInput)
{
  TriangleSoup soup = soup_of(make_cube());
  shuffle_soup(soup, 7);
  const auto result = build(soup);
  ASSERT_TRUE(result.has_value());
  const Mesh& mesh = result.mesh;

  for (std::uint32_t i = 0; i < soup.positions.size(); ++i)
  {
    EXPECT_EQ(mesh.get_position(VertexHandle{i}), soup.positions[i]);
  }
  for (std::uint32_t face = 0; face < soup.triangles.size(); ++face)
  {
    const HalfedgeHandle stored = mesh.get_face(FaceHandle{face}).get_halfedgehandle();
    EXPECT_EQ(stored, HalfedgeHandle{3 * face});
    const auto corners = mesh.vertices_around_face(FaceHandle{face});
    const Triangle triangle = soup.triangles[face];
    EXPECT_EQ(corners, (std::array{VertexHandle{triangle[0]}, VertexHandle{triangle[1]}, VertexHandle{triangle[2]}}));
  }
}

TEST(MeshFromTrianglesTest, ShuffledCubeSoupRoundTrips)
{
  expect_shuffled_round_trip(make_cube());
}

TEST(MeshFromTrianglesTest, ShuffledCylinderSoupRoundTrips)
{
  expect_shuffled_round_trip(make_cylinder());
}

TEST(MeshFromTrianglesTest, RebuildsMeshFromRenderBuffers)
{
  using MeshF = TriangleHalfedgeMesh3f;
  using Vec3F = MeshF::vec_t;

  auto creation = make_triangle_mesh(Cuboid<float>{{1.0F, 1.0F, 1.0F}});
  ASSERT_TRUE(creation.has_value());
  const MeshF& original = creation.mesh;
  const auto vertexBuffer = make_vertex_buffer(original);
  const auto indexBuffer = make_triangle_index_buffer(original);
  ASSERT_TRUE(vertexBuffer.has_value());
  ASSERT_TRUE(indexBuffer.has_value());

  std::vector<Vec3F> positions;
  for (std::size_t i = 0; i < vertexBuffer.values.size(); i += 3)
  {
    positions.push_back({vertexBuffer.values[i], vertexBuffer.values[i + 1], vertexBuffer.values[i + 2]});
  }
  std::vector<Triangle> triangles;
  for (std::size_t i = 0; i < indexBuffer.values.size(); i += 3)
  {
    triangles.push_back({indexBuffer.values[i], indexBuffer.values[i + 1], indexBuffer.values[i + 2]});
  }

  const auto result = make_mesh_from_triangles(std::span<const Vec3F>{positions}, std::span<const Triangle>{triangles});
  ASSERT_TRUE(result.has_value());
  EXPECT_TRUE(result.mesh.has_valid_connectivity());
  EXPECT_TRUE(verify_closed(result.mesh));
  EXPECT_EQ(is_outward_oriented(result.mesh), std::optional<bool>{true});
  EXPECT_EQ(make_triangle_index_buffer(result.mesh).values.size(), indexBuffer.values.size());
  EXPECT_EQ(make_vertex_buffer(result.mesh).values, vertexBuffer.values);
}

TEST(MeshFromTrianglesTest, ShuffledOpenGridGetsOneBoundaryLoop)
{
  TriangleSoup soup = make_grid_soup(4);
  shuffle_soup(soup, 3);
  const auto result = build(soup);
  ASSERT_TRUE(result.has_value());
  const Mesh& mesh = result.mesh;
  expect_valid_manifold(mesh);
  EXPECT_FALSE(verify_closed(mesh));
  EXPECT_EQ(euler_characteristic(mesh), 1);
  EXPECT_EQ(sorted_boundary_loop_sizes(mesh), (std::vector<std::size_t>{12}));
  // Only the center vertices (1, 1), (2, 1), (1, 2), (2, 2) are interior.
  for (const VertexHandle vertex : mesh.vertices())
  {
    const std::uint32_t i = vertex.get_value() % 4;
    const std::uint32_t j = vertex.get_value() / 4;
    const bool isInterior = i > 0 && i < 3 && j > 0 && j < 3;
    EXPECT_EQ(is_boundary(mesh, vertex), !isInterior);
  }
}

TEST(MeshFromTrianglesTest, GridWithHoleGetsTwoBoundaryLoops)
{
  TriangleSoup soup = make_grid_soup(4, std::array<std::uint32_t, 2>{1, 1});
  shuffle_soup(soup, 5);
  const auto result = build(soup);
  ASSERT_TRUE(result.has_value());
  const Mesh& mesh = result.mesh;
  expect_valid_manifold(mesh);
  EXPECT_EQ(euler_characteristic(mesh), 0);
  EXPECT_EQ(sorted_boundary_loop_sizes(mesh), (std::vector<std::size_t>{4, 12}));
}

TEST(MeshFromTrianglesTest, OppositeCopiesOfATriangleFormAClosedSurface)
{
  const TriangleSoup soup{make_positions(3), {{0, 1, 2}, {0, 2, 1}}};
  const auto result = build(soup);
  ASSERT_TRUE(result.has_value());
  expect_valid_manifold(result.mesh);
  EXPECT_TRUE(verify_closed(result.mesh));
  EXPECT_EQ(result.mesh.edge_count(), 3U);
  EXPECT_EQ(euler_characteristic(result.mesh), 2);
}

TEST(MeshFromTrianglesTest, BowTieIsNonManifoldVertex)
{
  // Two triangles that share only vertex 0: two open fans at one vertex.
  expect_failure({make_positions(5), {{0, 1, 2}, {0, 3, 4}}}, Status::NonManifoldVertex);
}

TEST(MeshFromTrianglesTest, ClosedSurfacesSharingAVertexAreNonManifoldVertex)
{
  // Two tetrahedra with the common apex 0: two closed fans, so no boundary gap gives it away.
  const std::vector<Triangle> tetrahedron{{0, 2, 1}, {0, 1, 3}, {0, 3, 2}, {1, 2, 3}};
  TriangleSoup soup{make_positions(7), tetrahedron};
  for (Triangle triangle : tetrahedron)
  {
    for (std::uint32_t& vertex : triangle)
    {
      vertex = vertex == 0 ? 0 : vertex + 3;
    }
    soup.triangles.push_back(triangle);
  }
  expect_failure(soup, Status::NonManifoldVertex);
}

TEST(MeshFromTrianglesTest, ThreeFacesOnAnEdgeAreNonManifoldEdge)
{
  expect_failure({make_positions(5), {{0, 1, 2}, {1, 0, 3}, {1, 0, 4}}}, Status::NonManifoldEdge);
}

TEST(MeshFromTrianglesTest, FourFacesOnAnEdgeAreNonManifoldEdge)
{
  // Two consistently oriented pairs around edge (0, 1), as where two solids touch along an edge.
  expect_failure({make_positions(6), {{0, 1, 2}, {1, 0, 3}, {0, 1, 4}, {1, 0, 5}}}, Status::NonManifoldEdge);
}

TEST(MeshFromTrianglesTest, FlippedTriangleIsInconsistentOrientation)
{
  TriangleSoup soup = soup_of(make_cube());
  std::swap(soup.triangles[5][1], soup.triangles[5][2]);
  expect_failure(soup, Status::InconsistentOrientation);
}

TEST(MeshFromTrianglesTest, DuplicateTriangleIsInconsistentOrientation)
{
  expect_failure({make_positions(3), {{0, 1, 2}, {1, 2, 0}}}, Status::InconsistentOrientation);
}

TEST(MeshFromTrianglesTest, RepeatedVertexIsDegenerateTriangle)
{
  expect_failure({make_positions(4), {{0, 1, 2}, {1, 3, 1}}}, Status::DegenerateTriangle);
}

TEST(MeshFromTrianglesTest, IndexPastPositionsIsOutOfRange)
{
  expect_failure({make_positions(3), {{0, 1, 3}}}, Status::VertexIndexOutOfRange);
}

TEST(MeshFromTrianglesTest, ReportsHandleCapacity)
{
  using SmallMesh = TriangleHalfedgeMesh<double, 3, std::uint8_t>;
  using SmallTriangle = std::array<std::uint8_t, 3>;
  using SmallVec3 = SmallMesh::vec_t;

  // A strip of n triangles over n + 2 vertices has n + 2 boundary edges, so 3n + n + 2 halfedges.
  const auto build_strip = [](std::size_t triangleCount, std::size_t vertexCount) {
    const std::vector<SmallVec3> positions(vertexCount);
    std::vector<SmallTriangle> triangles;
    for (std::size_t k = 0; k < triangleCount; ++k)
    {
      const auto first = static_cast<std::uint8_t>(k);
      const auto second = static_cast<std::uint8_t>(k + 1);
      const auto third = static_cast<std::uint8_t>(k + 2);
      triangles.push_back(k % 2 == 0 ? SmallTriangle{first, second, third} : SmallTriangle{second, first, third});
    }
    return make_mesh_from_triangles(std::span<const SmallVec3>{positions}, std::span<const SmallTriangle>{triangles});
  };

  // 162 halfedges fit below the invalid handle value 255.
  const auto fitting = build_strip(40, 42);
  ASSERT_TRUE(fitting.has_value());
  EXPECT_TRUE(fitting.mesh.has_valid_connectivity());
  EXPECT_TRUE(verify_manifold(fitting.mesh));
  EXPECT_EQ(fitting.mesh.halfedge_count(), 162U);

  // 210 face halfedges fit, but the 72 boundary halfedges do not.
  EXPECT_EQ(build_strip(70, 72).error, Status::IndexCapacityExceeded);
  // Vertex handles must stay below 255.
  EXPECT_TRUE(build_strip(0, 254).has_value());
  EXPECT_EQ(build_strip(0, 255).error, Status::IndexCapacityExceeded);
  // 85 triangles would need halfedge 255.
  EXPECT_EQ(build_strip(85, 87).error, Status::IndexCapacityExceeded);
}

TEST(MeshFromTrianglesTest, BuildsPlanarMesh)
{
  using Mesh2 = TriangleHalfedgeMesh2d;
  using Vec2 = Mesh2::vec_t;

  const std::vector<Vec2> positions{{0.0, 0.0}, {1.0, 0.0}, {1.0, 1.0}, {0.0, 1.0}};
  const std::vector<Triangle> triangles{{0, 2, 3}, {0, 1, 2}};
  const auto result = make_mesh_from_triangles(std::span<const Vec2>{positions}, std::span<const Triangle>{triangles});
  ASSERT_TRUE(result.has_value());
  EXPECT_TRUE(result.mesh.has_valid_connectivity());
  EXPECT_TRUE(verify_manifold(result.mesh));
  EXPECT_EQ(result.mesh.edge_count(), 5U);
  EXPECT_EQ(boundary_loop_count(result.mesh), 1U);
}

} // namespace MeshFromTrianglesTesting
