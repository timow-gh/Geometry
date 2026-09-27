#include <Geometry/Mesh/AddTriangle.hpp>
#include <Geometry/Mesh/MeshBuffers.hpp>
#include <Geometry/Mesh/MeshGlobalTopology.hpp>
#include <Geometry/Mesh/MeshOrientation.hpp>
#include <Geometry/Mesh/MeshVerify.hpp>
#include <Geometry/Mesh/TriangleHalfedgeMesh.hpp>
#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <iterator>
#include <set>
#include <vector>

using namespace Geometry;

namespace
{

using Mesh = TriangleHalfedgeMesh3d;
using VertexHandle = Mesh::VertexHandle;
using HalfedgeHandle = Mesh::HalfedgeHandle;
using EdgeHandle = Mesh::EdgeHandle;
using FaceHandle = Mesh::FaceHandle;
using Position = Mesh::vec_t;

// linal vectors have no ordering; positions are compared lexicographically for sorting and sets.
struct PositionLess
{
  bool operator()(const Position& lhs, const Position& rhs) const
  {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
  }
};
using PositionSet = std::set<Position, PositionLess>;

// Three disjoint components in handle order: triangle A (vertices 0-2), closed tetrahedron B
// (vertices 3-6), triangle C (vertices 7-9). Deleting B leaves garbage in the middle of every
// storage array, so compaction has to move C's elements down.
struct ThreeComponents
{
  Mesh mesh;
  std::array<VertexHandle, 3> triangleA{};
  std::array<VertexHandle, 4> tetrahedronB{};
  std::array<VertexHandle, 3> triangleC{};
};

ThreeComponents make_three_components()
{
  ThreeComponents result;
  Mesh& mesh = result.mesh;
  result.triangleA = {mesh.add_vertex({0.0, 0.0, 0.0}), mesh.add_vertex({1.0, 0.0, 0.0}), mesh.add_vertex({0.0, 1.0, 0.0})};
  EXPECT_TRUE(add_triangle(mesh, result.triangleA).is_valid());

  result.tetrahedronB = {mesh.add_vertex({10.0, 0.0, 0.0}), mesh.add_vertex({11.0, 0.0, 0.0}),
                         mesh.add_vertex({10.0, 1.0, 0.0}), mesh.add_vertex({10.0, 0.0, 1.0})};
  const auto& tet = result.tetrahedronB;
  EXPECT_TRUE(add_triangle(mesh, tet[0], tet[2], tet[1]).is_valid());
  EXPECT_TRUE(add_triangle(mesh, tet[0], tet[1], tet[3]).is_valid());
  EXPECT_TRUE(add_triangle(mesh, tet[1], tet[2], tet[3]).is_valid());
  EXPECT_TRUE(add_triangle(mesh, tet[2], tet[0], tet[3]).is_valid());

  result.triangleC = {mesh.add_vertex({20.0, 0.0, 0.0}), mesh.add_vertex({21.0, 0.0, 0.0}), mesh.add_vertex({20.0, 1.0, 0.0})};
  EXPECT_TRUE(add_triangle(mesh, result.triangleC).is_valid());
  return result;
}

// Tombstones every face, edge and vertex reachable from the given vertices. Valid only for whole
// connected components: nothing outside them references their elements, so no relinking is needed.
template <std::size_t N>
void delete_component(Mesh& mesh, const std::array<VertexHandle, N>& component)
{
  std::set<FaceHandle> faces;
  std::set<EdgeHandle> edges;
  for (const VertexHandle vertex : component)
  {
    for (const FaceHandle face : mesh.faces_around_vertex(vertex))
    {
      faces.insert(face);
    }
    for (auto outgoing = mesh.outgoing_halfedges(vertex).circulator(); outgoing.is_valid(); ++outgoing)
    {
      edges.insert(outgoing->edge);
    }
  }

  const auto connectivity = mesh.connectivity();
  for (const FaceHandle face : faces)
  {
    connectivity.mark_deleted(face);
  }
  for (const EdgeHandle edge : edges)
  {
    connectivity.mark_deleted(edge);
  }
  for (const VertexHandle vertex : component)
  {
    connectivity.mark_deleted(vertex);
  }
}

// Sorted corner positions of every live face, so a mesh can be compared across renumbering.
std::vector<std::vector<Position>> face_positions(const Mesh& mesh)
{
  std::vector<std::vector<Position>> result;
  for (const FaceHandle face : mesh.faces())
  {
    std::vector<Position> corners;
    for (const VertexHandle vertex : mesh.vertices_around_face(face))
    {
      corners.push_back(mesh.get_position(vertex));
    }
    std::sort(corners.begin(), corners.end(), PositionLess{});
    result.push_back(corners);
  }
  return result;
}

void expect_structurally_valid(const Mesh& mesh)
{
  EXPECT_TRUE(mesh.has_valid_connectivity());
  EXPECT_TRUE(verify_manifold(mesh));
  EXPECT_TRUE(is_consistently_oriented(mesh));
}

} // namespace

TEST(MeshGarbageCollection, FreshMeshHasNoGarbage)
{
  const ThreeComponents components = make_three_components();
  const Mesh& mesh = components.mesh;

  EXPECT_FALSE(mesh.has_garbage());
  EXPECT_EQ(mesh.vertex_count(), mesh.vertex_storage_size());
  EXPECT_EQ(mesh.halfedge_count(), mesh.halfedge_storage_size());
  EXPECT_EQ(mesh.edge_count(), mesh.edge_storage_size());
  EXPECT_EQ(mesh.face_count(), mesh.face_storage_size());
}

TEST(MeshGarbageCollection, MarkDeletedExcludesElementFromCountsAndRanges)
{
  Mesh mesh;
  const VertexHandle first = mesh.add_vertex({0.0, 0.0, 0.0});
  const VertexHandle middle = mesh.add_vertex({1.0, 0.0, 0.0});
  const VertexHandle last = mesh.add_vertex({2.0, 0.0, 0.0});

  mesh.connectivity().mark_deleted(middle);

  EXPECT_TRUE(mesh.has_garbage());
  EXPECT_TRUE(mesh.is_deleted(middle));
  EXPECT_FALSE(mesh.is_deleted(first));
  EXPECT_EQ(mesh.vertex_count(), 2U);
  EXPECT_EQ(mesh.vertex_storage_size(), 3U);
  EXPECT_EQ(mesh.vertices().size(), 2U);

  std::vector<VertexHandle> visited(mesh.vertices().begin(), mesh.vertices().end());
  EXPECT_EQ(visited, (std::vector<VertexHandle>{first, last}));
  // An isolated vertex references nothing, so deleting it leaves the connectivity valid.
  EXPECT_TRUE(mesh.has_valid_connectivity());
}

TEST(MeshGarbageCollection, IteratorsSkipLeadingAndTrailingDeletedElements)
{
  Mesh mesh;
  std::vector<VertexHandle> vertices;
  for (int i = 0; i < 5; ++i)
  {
    vertices.push_back(mesh.add_vertex({static_cast<double>(i), 0.0, 0.0}));
  }
  mesh.connectivity().mark_deleted(vertices.front());
  mesh.connectivity().mark_deleted(vertices.back());

  const auto range = mesh.vertices();
  EXPECT_EQ(*range.begin(), vertices[1]);
  EXPECT_EQ(*std::prev(range.end()), vertices[3]);
  EXPECT_EQ(std::distance(range.begin(), range.end()), 3);

  std::vector<VertexHandle> backwards;
  for (auto iterator = range.end(); iterator != range.begin();)
  {
    backwards.push_back(*--iterator);
  }
  EXPECT_EQ(backwards, (std::vector<VertexHandle>{vertices[3], vertices[2], vertices[1]}));
}

TEST(MeshGarbageCollection, RangeIsEmptyWhenEveryElementIsDeleted)
{
  Mesh mesh;
  const VertexHandle only = mesh.add_vertex({0.0, 0.0, 0.0});
  mesh.connectivity().mark_deleted(only);

  EXPECT_TRUE(mesh.vertices().empty());
  EXPECT_EQ(mesh.vertices().begin(), mesh.vertices().end());
  EXPECT_TRUE(mesh.empty());
}

TEST(MeshGarbageCollection, DeletedHalfedgesFollowTheirEdge)
{
  ThreeComponents components = make_three_components();
  Mesh& mesh = components.mesh;
  const HalfedgeHandle halfedge = mesh.get_vertex(components.triangleC[0]).halfedge;
  const HalfedgeHandle twin = mesh.get_halfedge(halfedge).twin;

  delete_component(mesh, components.triangleC);

  EXPECT_TRUE(mesh.is_deleted(halfedge));
  EXPECT_TRUE(mesh.is_deleted(twin));
  EXPECT_EQ(mesh.halfedge_count(), mesh.halfedge_storage_size() - 6U);
}

TEST(MeshGarbageCollection, DeletingWholeComponentKeepsMeshValid)
{
  ThreeComponents components = make_three_components();
  Mesh& mesh = components.mesh;

  delete_component(mesh, components.tetrahedronB);

  EXPECT_TRUE(mesh.has_garbage());
  EXPECT_EQ(mesh.vertex_count(), 6U);
  EXPECT_EQ(mesh.edge_count(), 6U);
  EXPECT_EQ(mesh.halfedge_count(), 12U);
  EXPECT_EQ(mesh.face_count(), 2U);
  EXPECT_EQ(mesh.faces().size(), 2U);
  // Two open triangles: chi = 1 each, so the deleted closed tetrahedron (chi = 2) no longer counts.
  EXPECT_EQ(euler_characteristic(mesh), 2);
  EXPECT_EQ(connected_component_count(mesh), 2U);
  EXPECT_EQ(boundary_loop_count(mesh), 2U);
  expect_structurally_valid(mesh);
}

TEST(MeshGarbageCollection, LiveElementReferencingDeletedOneFailsValidation)
{
  ThreeComponents components = make_three_components();
  Mesh& mesh = components.mesh;

  // Tombstoning a corner of a live triangle without detaching it leaves dangling references.
  mesh.connectivity().mark_deleted(components.triangleA[0]);
  EXPECT_FALSE(mesh.has_valid_connectivity());
}

TEST(MeshGarbageCollection, GarbageCollectionCompactsAndRemaps)
{
  ThreeComponents components = make_three_components();
  Mesh& mesh = components.mesh;
  delete_component(mesh, components.tetrahedronB);
  const auto facesBefore = face_positions(mesh);

  mesh.garbage_collection();

  EXPECT_FALSE(mesh.has_garbage());
  EXPECT_EQ(mesh.vertex_storage_size(), 6U);
  EXPECT_EQ(mesh.halfedge_storage_size(), 12U);
  EXPECT_EQ(mesh.edge_storage_size(), 6U);
  EXPECT_EQ(mesh.face_storage_size(), 2U);
  expect_structurally_valid(mesh);
  EXPECT_EQ(face_positions(mesh), facesBefore);

  // Survivors keep their relative order: A's vertices first, then C's moved down past the gap.
  const std::array<Position, 6> expectedPositions{Position{0.0, 0.0, 0.0},  Position{1.0, 0.0, 0.0},
                                                  Position{0.0, 1.0, 0.0},  Position{20.0, 0.0, 0.0},
                                                  Position{21.0, 0.0, 0.0}, Position{20.0, 1.0, 0.0}};
  for (std::size_t i = 0; i < expectedPositions.size(); ++i)
  {
    EXPECT_EQ(mesh.get_position(VertexHandle{static_cast<Mesh::handle_value_type>(i)}), expectedPositions[i]);
  }
}

TEST(MeshGarbageCollection, GarbageCollectionWithoutGarbageIsNoOp)
{
  ThreeComponents components = make_three_components();
  Mesh& mesh = components.mesh;
  const auto facesBefore = face_positions(mesh);
  const std::size_t halfedgeStorage = mesh.halfedge_storage_size();

  mesh.garbage_collection();

  EXPECT_EQ(mesh.halfedge_storage_size(), halfedgeStorage);
  EXPECT_EQ(face_positions(mesh), facesBefore);
  expect_structurally_valid(mesh);
}

TEST(MeshGarbageCollection, GarbageCollectionPreservesCreaseFlags)
{
  ThreeComponents components = make_three_components();
  Mesh& mesh = components.mesh;
  const HalfedgeHandle creaseHalfedge = mesh.find_halfedge(components.triangleC[0], components.triangleC[1]);
  mesh.set_crease(mesh.get_halfedge(creaseHalfedge).edge, true);
  delete_component(mesh, components.tetrahedronB);

  mesh.garbage_collection();

  std::size_t creaseCount = 0;
  for (const EdgeHandle edge : mesh.edges())
  {
    if (mesh.is_crease(edge))
    {
      ++creaseCount;
      const HalfedgeHandle halfedge = mesh.get_edge(edge).halfedge;
      const PositionSet endpoints{mesh.get_position(mesh.source_vertex(halfedge)),
                                         mesh.get_position(mesh.target_vertex(halfedge))};
      EXPECT_EQ(endpoints, (PositionSet{Position{20.0, 0.0, 0.0}, Position{21.0, 0.0, 0.0}}));
    }
  }
  EXPECT_EQ(creaseCount, 1U);
}

TEST(MeshGarbageCollection, AddTriangleWorksWhileGarbageIsPresent)
{
  ThreeComponents components = make_three_components();
  Mesh& mesh = components.mesh;
  delete_component(mesh, components.tetrahedronB);

  // A duplicate is rejected before anything is appended, so storage stays untouched.
  const std::size_t faceStorage = mesh.face_storage_size();
  EXPECT_FALSE(add_triangle(mesh, components.triangleA).is_valid());
  EXPECT_EQ(mesh.face_storage_size(), faceStorage);

  // Growing triangle A across its boundary edge appends after the tombstones.
  const VertexHandle apex = mesh.add_vertex({1.0, 1.0, 0.0});
  const FaceHandle face = add_triangle(mesh, components.triangleA[2], components.triangleA[1], apex);
  ASSERT_TRUE(face.is_valid());
  EXPECT_EQ(face.get_value(), faceStorage);
  EXPECT_EQ(mesh.face_count(), 3U);
  expect_structurally_valid(mesh);

  mesh.garbage_collection();
  EXPECT_EQ(mesh.face_count(), 3U);
  EXPECT_EQ(connected_component_count(mesh), 2U);
  expect_structurally_valid(mesh);
}

TEST(MeshGarbageCollection, RenderBuffersIgnoreDeletedElements)
{
  ThreeComponents components = make_three_components();
  Mesh& mesh = components.mesh;
  delete_component(mesh, components.tetrahedronB);

  // make_render_buffers remaps vertices itself, so it must give the same result before and after
  // compaction; the raw index buffers instead require a collected mesh.
  const MeshRenderBuffers withGarbage = make_render_buffers(mesh);
  mesh.garbage_collection();
  const MeshRenderBuffers collected = make_render_buffers(mesh);

  ASSERT_TRUE(withGarbage.has_value());
  ASSERT_TRUE(collected.has_value());
  EXPECT_EQ(withGarbage.positions, collected.positions);
  EXPECT_EQ(withGarbage.normals, collected.normals);
  EXPECT_EQ(withGarbage.triangles, collected.triangles);
  EXPECT_EQ(withGarbage.segments, collected.segments);
  EXPECT_EQ(withGarbage.triangles.size(), 6U);
}
