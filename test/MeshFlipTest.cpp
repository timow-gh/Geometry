#include <Geometry/Cuboid.hpp>
#include <Geometry/Cylinder.hpp>
#include <Geometry/Mesh/AddTriangle.hpp>
#include <Geometry/Mesh/MakeTriangleMesh.hpp>
#include <Geometry/Mesh/MeshDelete.hpp>
#include <Geometry/Mesh/MeshFlip.hpp>
#include <Geometry/Mesh/MeshFromTriangles.hpp>
#include <Geometry/Mesh/MeshGlobalTopology.hpp>
#include <Geometry/Mesh/MeshOrientation.hpp>
#include <Geometry/Mesh/MeshSplit.hpp>
#include <Geometry/Mesh/MeshTopology.hpp>
#include <Geometry/Mesh/MeshVerify.hpp>
#include <Geometry/Mesh/TriangleHalfedgeMesh.hpp>
#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <random>
#include <span>
#include <tuple>
#include <utility>
#include <vector>

using namespace Geometry;

namespace
{

using Mesh = TriangleHalfedgeMesh3d;
using VertexHandle = Mesh::VertexHandle;
using HalfedgeHandle = Mesh::HalfedgeHandle;
using EdgeHandle = Mesh::EdgeHandle;
using FaceHandle = Mesh::FaceHandle;
using Vec3 = Mesh::vec_t;
using Index = Mesh::handle_value_type;
using Triangle = std::array<Index, 3>;

// n x n grid in the xy-plane as in MeshEdgeCollapseTest.cpp; vertex (i, j) has handle value j * n + i.
// Each cell is split along its (i, j)-(i+1, j+1) diagonal.
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
  const auto vertexAt = [&](std::size_t column, std::size_t row) { return vertices[row * size + column]; };
  // Every triangle must attach along an existing edge (add_triangle refuses a second fan at a
  // corner): row 0 grows through each cell's upper triangle first, later rows through the lower.
  for (std::size_t j = 0; j + 1 < size; ++j)
  {
    for (std::size_t i = 0; i + 1 < size; ++i)
    {
      const std::array<VertexHandle, 3> lower{vertexAt(i, j), vertexAt(i + 1, j), vertexAt(i + 1, j + 1)};
      const std::array<VertexHandle, 3> upper{vertexAt(i, j), vertexAt(i + 1, j + 1), vertexAt(i, j + 1)};
      for (const auto& triangle : j == 0 ? std::array{upper, lower} : std::array{lower, upper})
      {
        EXPECT_TRUE(add_triangle(mesh, triangle).is_valid());
      }
    }
  }
  return mesh;
}

// Handle of grid vertex (column, row) in a 3 x 3 grid.
constexpr VertexHandle grid3(std::uint32_t column, std::uint32_t row)
{
  return VertexHandle{row * 3 + column};
}

// A 4 x 4 grid with its interior face (1,1), (2,1), (2,2) deleted. add_triangle stores the face side
// of a new boundary edge, so only a deletion leaves edges whose stored halfedge is on the boundary.
Mesh make_grid_with_hole()
{
  Mesh mesh = make_grid(4);
  const HalfedgeHandle halfedge = mesh.find_halfedge(VertexHandle{5}, VertexHandle{6});
  EXPECT_EQ(delete_face(mesh, mesh.get_halfedge(halfedge).face), DeleteStatus::Ok);
  return mesh;
}

// Two triangles over the same three vertices, wound oppositely: a closed surface whose two faces share
// their apex across every edge. add_triangle refuses the second triangle; make_mesh_from_triangles
// accepts it.
Mesh make_pillow()
{
  const std::array<Vec3, 3> positions{Vec3{0.0, 0.0, 0.0}, Vec3{1.0, 0.0, 0.0}, Vec3{0.0, 1.0, 0.0}};
  const std::array<Triangle, 2> triangles{Triangle{0, 1, 2}, Triangle{1, 0, 2}};
  auto creation = make_mesh_from_triangles(std::span<const Vec3>{positions}, std::span<const Triangle>{triangles});
  EXPECT_TRUE(creation.has_value());
  return std::move(creation.mesh);
}

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

Mesh make_cube()
{
  auto creation = make_triangle_mesh(Cuboid<double>{{1.0, 1.0, 1.0}});
  EXPECT_TRUE(creation.has_value());
  return std::move(creation.mesh);
}

Mesh make_cylinder()
{
  auto creation = make_triangle_mesh(Cylinder<double>{Segment3<double>{{0.0, 0.0, 0.0}, {0.0, 0.0, 2.0}}, 1.0}, 12);
  EXPECT_TRUE(creation.has_value());
  return std::move(creation.mesh);
}

EdgeHandle edge_between(const Mesh& mesh, VertexHandle first, VertexHandle second)
{
  const HalfedgeHandle halfedge = mesh.find_halfedge(first, second);
  EXPECT_TRUE(halfedge.is_valid());
  return halfedge.is_valid() ? mesh.get_halfedge(halfedge).edge : EdgeHandle{};
}

// Whether face has exactly the expected corners, in the same cyclic order.
bool has_corners(const Mesh& mesh, FaceHandle face, const std::array<VertexHandle, 3>& expected)
{
  const auto corners = mesh.vertices_around_face(face);
  for (std::size_t shift = 0; shift < 3; ++shift)
  {
    if (corners[shift] == expected[0] && corners[(shift + 1) % 3] == expected[1] && corners[(shift + 2) % 3] == expected[2])
    {
      return true;
    }
  }
  return false;
}

// Every link and flag of the connectivity, in storage order, so that a test can show a rejected
// operation left the mesh exactly as it was rather than merely valid.
struct ConnectivityRecords
{
  std::vector<std::tuple<VertexHandle, HalfedgeHandle, HalfedgeHandle, HalfedgeHandle, FaceHandle, EdgeHandle>> halfedges;
  std::vector<std::tuple<HalfedgeHandle, bool, bool>> edges;
  std::vector<std::tuple<HalfedgeHandle, bool>> vertices;
  std::vector<std::tuple<HalfedgeHandle, bool>> faces;

  bool operator==(const ConnectivityRecords&) const = default;
};

ConnectivityRecords connectivity_records(const Mesh& mesh)
{
  ConnectivityRecords records;
  for (Index i = 0; i < mesh.halfedge_storage_size(); ++i)
  {
    const auto& halfedge = mesh.get_halfedge(HalfedgeHandle{i});
    records.halfedges.emplace_back(halfedge.targetVertex, halfedge.twin, halfedge.next, halfedge.prev, halfedge.face, halfedge.edge);
  }
  for (Index i = 0; i < mesh.edge_storage_size(); ++i)
  {
    const auto& edge = mesh.get_edge(EdgeHandle{i});
    records.edges.emplace_back(edge.halfedge, edge.crease, edge.deleted);
  }
  for (Index i = 0; i < mesh.vertex_storage_size(); ++i)
  {
    const auto& vertex = mesh.get_vertex(VertexHandle{i});
    records.vertices.emplace_back(vertex.halfedge, vertex.deleted);
  }
  for (Index i = 0; i < mesh.face_storage_size(); ++i)
  {
    const auto& face = mesh.get_face(FaceHandle{i});
    records.faces.emplace_back(face.get_halfedgehandle(), face.is_deleted());
  }
  return records;
}

// Sum of origin tetrahedra.
double signed_volume(const Mesh& mesh)
{
  double volume = 0.0;
  for (const FaceHandle face : mesh.faces())
  {
    const auto corners = mesh.vertices_around_face(face);
    volume += linal::dot(mesh.get_position(corners[0]),
                         linal::cross(mesh.get_position(corners[1]), mesh.get_position(corners[2])))
              / 6.0;
  }
  return volume;
}

// Every face of a planar mesh in the xy-plane winds counter-clockwise, i.e. none is folded over.
bool all_faces_counter_clockwise(const Mesh& mesh)
{
  return std::ranges::all_of(mesh.faces(), [&](FaceHandle face) {
    const auto corners = mesh.vertices_around_face(face);
    const Vec3 first = mesh.get_position(corners[0]);
    const Vec3 normal = linal::cross(Vec3{mesh.get_position(corners[1]) - first}, Vec3{mesh.get_position(corners[2]) - first});
    return normal[2] > 0.0;
  });
}

void expect_structurally_valid(const Mesh& mesh)
{
  EXPECT_TRUE(mesh.has_valid_connectivity());
  EXPECT_TRUE(verify_manifold(mesh));
  EXPECT_TRUE(is_consistently_oriented(mesh));
}

} // namespace

TEST(MeshFlip, FlipsInteriorDiagonal)
{
  Mesh mesh = make_grid(3);
  const EdgeHandle edge = edge_between(mesh, grid3(0, 0), grid3(1, 1));
  const HalfedgeHandle forward = mesh.get_edge(edge).halfedge;
  const HalfedgeHandle backward = mesh.get_halfedge(forward).twin;
  const VertexHandle start = mesh.source_vertex(forward);
  const VertexHandle end = mesh.target_vertex(forward);
  const VertexHandle leftApex = mesh.target_vertex(mesh.get_halfedge(forward).next);
  const VertexHandle rightApex = mesh.target_vertex(mesh.get_halfedge(backward).next);
  const FaceHandle leftFace = mesh.get_halfedge(forward).face;
  const FaceHandle rightFace = mesh.get_halfedge(backward).face;
  const std::size_t faceCount = mesh.face_count();
  const std::size_t edgeCount = mesh.edge_count();

  ASSERT_EQ(flip_edge(mesh, edge), FlipStatus::Ok);

  EXPECT_FALSE(mesh.find_halfedge(grid3(0, 0), grid3(1, 1)).is_valid());
  const HalfedgeHandle flipped = mesh.find_halfedge(grid3(1, 0), grid3(0, 1));
  ASSERT_TRUE(flipped.is_valid());
  EXPECT_EQ(mesh.get_halfedge(flipped).edge, edge);
  // The edge keeps its stored halfedge, now running from the right apex to the left one; the face on
  // its side keeps the start vertex, the other face the end vertex.
  EXPECT_EQ(mesh.get_edge(edge).halfedge, forward);
  EXPECT_EQ(mesh.find_halfedge(rightApex, leftApex), forward);
  EXPECT_EQ(mesh.get_halfedge(forward).face, leftFace);
  EXPECT_EQ(mesh.get_halfedge(backward).face, rightFace);
  EXPECT_TRUE(has_corners(mesh, leftFace, {leftApex, start, rightApex}));
  EXPECT_TRUE(has_corners(mesh, rightFace, {rightApex, end, leftApex}));
  EXPECT_EQ(mesh.face_count(), faceCount);
  EXPECT_EQ(mesh.edge_count(), edgeCount);
  EXPECT_FALSE(mesh.has_garbage());
  EXPECT_TRUE(mesh.is_live(leftFace) && mesh.is_live(rightFace));
  EXPECT_EQ(valence(mesh, grid3(0, 0)), 2U);
  EXPECT_EQ(valence(mesh, grid3(1, 1)), 5U);
  EXPECT_EQ(valence(mesh, grid3(1, 0)), 5U);
  EXPECT_EQ(valence(mesh, grid3(0, 1)), 5U);
  EXPECT_EQ(euler_characteristic(mesh), 1);
  EXPECT_EQ(boundary_loop_count(mesh), 1U);
  // The cell is a convex square, so the flip folds nothing.
  EXPECT_TRUE(all_faces_counter_clockwise(mesh));
  expect_structurally_valid(mesh);
}

TEST(MeshFlip, FlippingTwiceRejoinsOriginalEndpoints)
{
  Mesh mesh = make_grid(3);
  const EdgeHandle edge = edge_between(mesh, grid3(1, 1), grid3(2, 2));
  const HalfedgeHandle forward = mesh.get_edge(edge).halfedge;
  const VertexHandle start = mesh.source_vertex(forward);
  const VertexHandle end = mesh.target_vertex(forward);

  ASSERT_EQ(flip_edge(mesh, edge), FlipStatus::Ok);
  ASSERT_EQ(flip_edge(mesh, edge), FlipStatus::Ok);

  const HalfedgeHandle restored = mesh.find_halfedge(grid3(1, 1), grid3(2, 2));
  ASSERT_TRUE(restored.is_valid());
  EXPECT_EQ(mesh.get_halfedge(restored).edge, edge);
  // Each flip runs the stored halfedge from the right apex to the left one, so after two it is reversed.
  EXPECT_EQ(mesh.get_edge(edge).halfedge, forward);
  EXPECT_EQ(mesh.find_halfedge(end, start), forward);
  EXPECT_TRUE(all_faces_counter_clockwise(mesh));
  expect_structurally_valid(mesh);
}

TEST(MeshFlip, FlippingAnyInteriorGridEdgeKeepsMeshValid)
{
  const Mesh original = make_grid(4);
  std::size_t flipped = 0;
  std::size_t representativeReplaced = 0;
  for (const EdgeHandle edge : original.edges())
  {
    Mesh mesh = original;
    const HalfedgeHandle forward = mesh.get_edge(edge).halfedge;
    const HalfedgeHandle backward = mesh.get_halfedge(forward).twin;
    // An endpoint represented by the flipped edge's halfedge needs a new representative.
    const bool replacesRepresentative = mesh.get_vertex(mesh.source_vertex(forward)).halfedge == forward
                                        || mesh.get_vertex(mesh.target_vertex(forward)).halfedge == backward;
    const FlipStatus status = flip_edge(mesh, edge);
    if (mesh.is_boundary(edge))
    {
      EXPECT_EQ(status, FlipStatus::BoundaryEdge);
      continue;
    }
    ASSERT_EQ(status, FlipStatus::Ok);
    ++flipped;
    representativeReplaced += replacesRepresentative ? 1U : 0U;
    EXPECT_EQ(euler_characteristic(mesh), euler_characteristic(original));
    EXPECT_EQ(boundary_loop_count(mesh), 1U);
    expect_structurally_valid(mesh);
  }
  // 33 edges, 12 of them on the boundary.
  EXPECT_EQ(flipped, 21U);
  EXPECT_GT(representativeReplaced, 0U);
}

// Deleting a face leaves a tombstone in storage and edges whose stored halfedge lies on the new hole;
// those edges must be rejected like any boundary edge, and the rest flip as without the hole.
TEST(MeshFlip, FlippingAnyEdgeOfGridWithHoleKeepsMeshValid)
{
  const Mesh original = make_grid_with_hole();
  const ConnectivityRecords records = connectivity_records(original);
  std::size_t flipped = 0;
  std::size_t storedOnBoundary = 0;
  for (const EdgeHandle edge : original.edges())
  {
    Mesh mesh = original;
    const FlipStatus status = flip_edge(mesh, edge);
    if (original.is_boundary(edge))
    {
      EXPECT_EQ(status, FlipStatus::BoundaryEdge);
      EXPECT_TRUE(connectivity_records(mesh) == records);
      storedOnBoundary += original.is_boundary(original.get_edge(edge).halfedge) ? 1U : 0U;
      continue;
    }
    ASSERT_EQ(status, FlipStatus::Ok);
    ++flipped;
    EXPECT_EQ(euler_characteristic(mesh), euler_characteristic(original));
    EXPECT_EQ(boundary_loop_count(mesh), 2U);
    expect_structurally_valid(mesh);
  }
  EXPECT_GT(storedOnBoundary, 0U);
  // The hole turns three of the 21 interior edges into boundary edges.
  EXPECT_EQ(flipped, 18U);
}

TEST(MeshFlip, FlippingCubeFaceDiagonalsKeepsSolid)
{
  Mesh mesh = make_cube();
  const double volume = signed_volume(mesh);
  std::size_t flipped = 0;
  for (const EdgeHandle edge : mesh.edges())
  {
    // The non-crease edges are the face diagonals; flipping one re-triangulates a planar square.
    if (mesh.is_crease(edge))
    {
      continue;
    }
    ASSERT_EQ(flip_edge(mesh, edge), FlipStatus::Ok);
    ++flipped;
  }

  EXPECT_EQ(flipped, 6U);
  EXPECT_EQ(euler_characteristic(mesh), 2);
  EXPECT_TRUE(verify_closed(mesh));
  EXPECT_EQ(mesh_orientation(mesh), MeshOrientation::Outward);
  EXPECT_NEAR(signed_volume(mesh), volume, 1e-12);
  expect_structurally_valid(mesh);
}

TEST(MeshFlip, ClearsCrease)
{
  Mesh mesh = make_grid(3);
  const EdgeHandle edge = edge_between(mesh, grid3(0, 0), grid3(1, 1));
  mesh.set_crease(edge, true);

  ASSERT_EQ(flip_edge(mesh, edge), FlipStatus::Ok);

  EXPECT_FALSE(mesh.is_crease(edge));
}

TEST(MeshFlip, RejectsBoundaryEdge)
{
  Mesh mesh = make_grid(3);
  const EdgeHandle edge = edge_between(mesh, grid3(0, 0), grid3(1, 0));
  const ConnectivityRecords records = connectivity_records(mesh);

  EXPECT_EQ(is_flip_ok(mesh, edge), FlipStatus::BoundaryEdge);
  EXPECT_EQ(flip_edge(mesh, edge), FlipStatus::BoundaryEdge);
  EXPECT_TRUE(connectivity_records(mesh) == records);
}

TEST(MeshFlip, RejectsEveryEdgeOfTetrahedron)
{
  // Every pair of vertices is already joined, so every flip would duplicate an edge.
  Mesh mesh = make_tetrahedron();
  const ConnectivityRecords records = connectivity_records(mesh);
  for (const EdgeHandle edge : mesh.edges())
  {
    EXPECT_EQ(flip_edge(mesh, edge), FlipStatus::DiagonalExists);
    EXPECT_TRUE(connectivity_records(mesh) == records);
  }
}

// Both faces of a pillow edge have the same apex, so the flipped edge would join that apex to itself.
TEST(MeshFlip, RejectsEdgeWithSharedApex)
{
  Mesh mesh = make_pillow();
  ASSERT_EQ(mesh.face_count(), 2U);
  const ConnectivityRecords records = connectivity_records(mesh);
  for (const EdgeHandle edge : mesh.edges())
  {
    EXPECT_EQ(flip_edge(mesh, edge), FlipStatus::DiagonalExists);
    EXPECT_TRUE(connectivity_records(mesh) == records);
  }
}

TEST(MeshFlip, RejectsEdgeAtInteriorValenceThreeVertex)
{
  // Flipping a spoke of a split face would leave the center with two edges and two triangles
  // covering each other; that shows up as the flipped edge already existing.
  Mesh mesh = make_cube();
  const FaceHandle face = *mesh.faces().begin();
  const auto corners = mesh.vertices_around_face(face);
  const Vec3 centroid = (mesh.get_position(corners[0]) + mesh.get_position(corners[1]) + mesh.get_position(corners[2])) / 3.0;
  const VertexHandle center = split_face(mesh, face, centroid);
  ASSERT_TRUE(center.is_valid());
  const ConnectivityRecords records = connectivity_records(mesh);

  for (const VertexHandle corner : corners)
  {
    EXPECT_EQ(flip_edge(mesh, edge_between(mesh, center, corner)), FlipStatus::DiagonalExists);
    EXPECT_TRUE(connectivity_records(mesh) == records);
  }
}

TEST(MeshFlip, RejectsInvalidAndDeletedEdge)
{
  Mesh mesh = make_grid(3);
  const ConnectivityRecords intactRecords = connectivity_records(mesh);
  EXPECT_EQ(flip_edge(mesh, EdgeHandle{}), FlipStatus::InvalidHandle);
  EXPECT_TRUE(connectivity_records(mesh) == intactRecords);

  // Deleting the corner face (1,0), (2,0), (2,1) drops its two boundary edges.
  const HalfedgeHandle cornerEdge = mesh.find_halfedge(grid3(1, 0), grid3(2, 0));
  ASSERT_TRUE(cornerEdge.is_valid());
  const EdgeHandle deleted = mesh.get_halfedge(cornerEdge).edge;
  ASSERT_EQ(delete_face(mesh, mesh.get_halfedge(cornerEdge).face), DeleteStatus::Ok);
  ASSERT_TRUE(mesh.is_deleted(deleted));
  const ConnectivityRecords records = connectivity_records(mesh);

  EXPECT_EQ(flip_edge(mesh, deleted), FlipStatus::InvalidHandle);
  EXPECT_TRUE(connectivity_records(mesh) == records);
}

// The quad around the edge has a reflex corner at the edge's start, so the other diagonal runs outside
// it. The flip is still legal topologically and folds the surface: only the caller can tell.
TEST(MeshFlip, AcceptsNonConvexQuadTopologically)
{
  Mesh mesh;
  const VertexHandle start = mesh.add_vertex({0.0, 0.0, 0.0});
  const VertexHandle end = mesh.add_vertex({2.0, 0.0, 0.0});
  const VertexHandle leftApex = mesh.add_vertex({1.0, 1.0, 0.0});
  const VertexHandle rightApex = mesh.add_vertex({-1.0, -0.5, 0.0});
  ASSERT_TRUE(add_triangle(mesh, start, end, leftApex).is_valid());
  ASSERT_TRUE(add_triangle(mesh, end, start, rightApex).is_valid());
  ASSERT_TRUE(all_faces_counter_clockwise(mesh));
  const EdgeHandle edge = edge_between(mesh, start, end);

  EXPECT_EQ(is_flip_ok(mesh, edge), FlipStatus::Ok);
  ASSERT_EQ(flip_edge(mesh, edge), FlipStatus::Ok);

  EXPECT_TRUE(mesh.find_halfedge(rightApex, leftApex).is_valid());
  expect_structurally_valid(mesh);
  EXPECT_FALSE(all_faces_counter_clockwise(mesh));
}

// Constraint recovery in corefinement flips repeatedly in a region that splits have refined, so
// flips and splits are interleaved here.
TEST(MeshFlipFuzz, RandomFlipsAndSplitsStayValid)
{
  for (std::uint32_t seed = 1; seed <= 5; ++seed)
  {
    Mesh mesh = make_cylinder();
    std::mt19937 generator(seed);
    std::size_t flipCount = 0;

    for (int step = 0; step < 300; ++step)
    {
      std::uniform_int_distribution<std::uint32_t> pick(0, static_cast<std::uint32_t>(mesh.edge_storage_size() - 1));
      const EdgeHandle edge{pick(generator)};
      if (step % 5 == 4)
      {
        const HalfedgeHandle halfedge = mesh.get_edge(edge).halfedge;
        const Vec3 midpoint = (mesh.get_position(mesh.source_vertex(halfedge)) + mesh.get_position(mesh.target_vertex(halfedge))) / 2.0;
        ASSERT_TRUE(split_edge(mesh, edge, midpoint).is_valid());
      }
      else
      {
        const std::size_t edgeCount = mesh.edge_count();
        const FlipStatus status = flip_edge(mesh, edge);
        // The cylinder is closed, so every edge has two faces.
        ASSERT_TRUE(status == FlipStatus::Ok || status == FlipStatus::DiagonalExists);
        ASSERT_EQ(mesh.edge_count(), edgeCount);
        flipCount += status == FlipStatus::Ok ? 1U : 0U;
      }
      ASSERT_TRUE(mesh.has_valid_connectivity()) << "seed " << seed << ", step " << step;
      ASSERT_TRUE(verify_manifold(mesh)) << "seed " << seed << ", step " << step;
    }

    EXPECT_GT(flipCount, 0U);
    EXPECT_FALSE(mesh.has_garbage());
    EXPECT_EQ(euler_characteristic(mesh), 2);
    EXPECT_TRUE(verify_closed(mesh));
    EXPECT_TRUE(is_consistently_oriented(mesh));
  }
}
